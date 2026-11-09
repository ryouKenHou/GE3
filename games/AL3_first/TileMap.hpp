#pragma once
#include "EngineCommon.h"
#include "block.hpp"

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstdint>
#include <filesystem>

// Note: ImGui is already made available transitively via "EngineCommon.h"
// (it includes imgui.h with a path relative to project/src/engine), so no
// separate ImGui include is needed here.

// Describes what a given tile "type" id looks like.
// Register one of these per distinct block visual (wall, floor, pillar, etc.).
struct BlockTypeDef {
	std::string name;            // Display name, e.g. "Wall"
	std::string modelDirectory;  // e.g. "../games/AL3_first/resources/model/block"
	std::string modelFilename;   // e.g. "block.obj"
};

// A simple 2D grid-based tile map that places instances of Block to form rooms.
// - Tiles are identified by an integer "type" (0 = empty/no block).
// - Each distinct type's Model is loaded once and shared across all placed instances.
// - Can be saved/loaded to a small, human-readable JSON file.
// - Includes an optional ImGui-based in-game editor (Debug builds only).
class TileMap {
public:
	static constexpr int kEmpty = 0;

	void Initialize(EngineCommon* engineCommon, int width, int height, float tileSize = 1.0f) {
		engineCommon_ = engineCommon;
		tileSize_ = tileSize;
		width_ = max(1, width);
		height_ = max(1, height);
		grid_.assign(height_, std::vector<int>(width_, kEmpty));

		// Default type 1 reuses the existing block.obj model that Block already ships with.
		RegisterBlockType(1, { "Block", "resources/model/block", "block.obj" });
	}

	// Call before placing tiles of a new type. Safe to call again to redefine a type
	// (existing placed instances of that type keep using the model loaded at the time
	// they were placed/rebuilt).
	void RegisterBlockType(int typeId, const BlockTypeDef& def) {
		blockTypes_[typeId] = def;
	}

	// Resizes the grid, preserving existing tiles that still fit, and rebuilds instances.
	void Resize(int width, int height) {
		width = max(1, width);
		height = max(1, height);
		std::vector<std::vector<int>> newGrid(height, std::vector<int>(width, kEmpty));
		for (int y = 0; y <min((int)grid_.size(), height); ++y) {
			for (int x = 0; x < min((int)grid_[y].size(), width); ++x) {
				newGrid[y][x] = grid_[y][x];
			}
		}
		grid_ = std::move(newGrid);
		width_ = width;
		height_ = height;
		RebuildAllInstances();
	}

	void SetTile(int x, int y, int typeId) {
		if (!InBounds(x, y)) return;
		grid_[y][x] = typeId;
		RebuildTileInstance(x, y);
	}

	int GetTile(int x, int y) const {
		if (!InBounds(x, y)) return kEmpty;
		return grid_[y][x];
	}

	// True if the given cell is occupied by any block (i.e. solid for collision).
	// Out-of-bounds cells are treated as empty/non-solid.
	bool IsSolid(int x, int y) const {
		return GetTile(x, y) != kEmpty;
	}

	// World-space center position of a grid cell (public so collision code can
	// build an AABB for a tile without duplicating the grid-to-world formula).
	Vector3 GridToWorld(int x, int y) const {
		float worldX = static_cast<float>(x) * tileSize_;
		float worldY = static_cast<float>(height_ - 1 - y) * tileSize_;
		return { worldX, worldY, 0.0f };
	}

	// Inverse of GridToWorld: given a world-space coordinate, returns the
	// column/row of the cell whose center is closest to it. Used to find the
	// range of cells an AABB overlaps for collision checks.
	int WorldToCol(float worldX) const {
		return static_cast<int>(std::floor(worldX / tileSize_ + 0.5f));
	}
	int WorldToRow(float worldY) const {
		return (height_ - 1) - static_cast<int>(std::floor(worldY / tileSize_ + 0.5f));
	}

	void Update(float deltaTime, DebugCamera* debugCamera) {
		// Safe to actually destroy anything queued from edits made during the
		// previous frame: the engine's PostDraw() already closed/executed that
		// frame's command list and fence-waited for the GPU to finish, so any
		// GPU resources those Blocks owned are guaranteed to be done being used.
		pendingRemoval_.clear();

		for (auto& [key, block] : instances_) {
			block->Update(deltaTime, debugCamera);
		}
	}

	void Draw() {
		for (auto& [key, block] : instances_) {
			block->Draw();
		}
	}

	int GetWidth() const { return width_; }
	int GetHeight() const { return height_; }
	float GetTileSize() const { return tileSize_; }

	// ---------------- JSON persistence ----------------
	// Produces/reads a small, hand-editable JSON file, e.g.:
	// {
	//   "width": 10, "height": 8, "tileSize": 1.0,
	//   "tiles": [ { "x": 0, "y": 0, "type": 1 }, ... ]
	// }
	bool SaveToJson(const std::string& filePath) const {
		// std::ofstream will not create missing directories on its own,
		// so make sure the destination folder exists first.
		std::filesystem::path path(filePath);
		if (path.has_parent_path()) {
			std::error_code ec;
			std::filesystem::create_directories(path.parent_path(), ec);
		}

		std::ofstream file(filePath);
		if (!file.is_open()) return false;

		file << "{\n";
		file << "  \"width\": " << width_ << ",\n";
		file << "  \"height\": " << height_ << ",\n";
		file << "  \"tileSize\": " << tileSize_ << ",\n";
		file << "  \"tiles\": [\n";

		bool first = true;
		for (int y = 0; y < height_; ++y) {
			for (int x = 0; x < width_; ++x) {
				int type = grid_[y][x];
				if (type == kEmpty) continue;
				if (!first) file << ",\n";
				first = false;
				file << "    { \"x\": " << x << ", \"y\": " << y << ", \"type\": " << type << " }";
			}
		}
		file << "\n  ]\n";
		file << "}\n";
		return true;
	}

	bool LoadFromJson(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file.is_open()) return false;

		std::stringstream ss;
		ss << file.rdbuf();
		std::string content = ss.str();

		int newWidth = width_;
		int newHeight = height_;
		float newTileSize = tileSize_;

		std::smatch m;
		if (std::regex_search(content, m, std::regex(R"("width"\s*:\s*(\d+))"))) {
			newWidth = std::stoi(m[1]);
		}
		if (std::regex_search(content, m, std::regex(R"("height"\s*:\s*(\d+))"))) {
			newHeight = std::stoi(m[1]);
		}
		if (std::regex_search(content, m, std::regex(R"("tileSize"\s*:\s*([\d.]+))"))) {
			newTileSize = std::stof(m[1]);
		}

		width_ = max(1, newWidth);
		height_ = max(1, newHeight);
		tileSize_ = newTileSize;
		grid_.assign(height_, std::vector<int>(width_, kEmpty));

		std::regex tileRegex(R"(\{\s*"x"\s*:\s*(-?\d+)\s*,\s*"y"\s*:\s*(-?\d+)\s*,\s*"type"\s*:\s*(-?\d+)\s*\})");
		auto begin = std::sregex_iterator(content.begin(), content.end(), tileRegex);
		auto end = std::sregex_iterator();
		for (auto it = begin; it != end; ++it) {
			int x = std::stoi((*it)[1]);
			int y = std::stoi((*it)[2]);
			int type = std::stoi((*it)[3]);
			if (InBounds(x, y)) grid_[y][x] = type;
		}

		RebuildAllInstances();
		return true;
	}

#ifdef _DEBUG
	// Simple in-game editor: click a cell to place the current brush type, or
	// erase it if it already has a block. Includes resize + save/load controls.
	void DrawImGuiEditor() {
		ImGui::Begin("Tile Map Editor");

		ImGui::Text("Grid: %d x %d  (tileSize %.2f)", width_, height_, tileSize_);

		static int newWidth = width_;
		static int newHeight = height_;
		ImGui::InputInt("Width", &newWidth);
		ImGui::InputInt("Height", &newHeight);
		if (ImGui::Button("Resize")) {
			Resize(newWidth, newHeight);
		}

		static int brushType = 1;
		ImGui::InputInt("Brush Type (0 = erase)", &brushType);

		for (int y = 0; y < height_; ++y) {
			ImGui::PushID(y);
			for (int x = 0; x < width_; ++x) {
				ImGui::PushID(x);
				int type = grid_[y][x];
				char label[8];
				snprintf(label, sizeof(label), "%d", type);

				bool occupied = (type != kEmpty);
				ImGui::PushStyleColor(ImGuiCol_Button, occupied
					? ImVec4(0.2f, 0.6f, 0.2f, 1.0f)
					: ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

				if (ImGui::Button(label, ImVec2(24, 24))) {
					SetTile(x, y, occupied ? kEmpty : brushType);
				}

				ImGui::PopStyleColor();
				if (x + 1 < width_) ImGui::SameLine();
				ImGui::PopID();
			}
			ImGui::PopID();
		}

		static char path[256] = "resources/maps/room1.json";
		ImGui::InputText("File", path, sizeof(path));
		if (ImGui::Button("Save")) {
			SaveToJson(path);
		}
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			LoadFromJson(path);
		}

		ImGui::End();
	}
#endif

private:
	bool InBounds(int x, int y) const {
		return x >= 0 && y >= 0 && x < width_ && y < height_;
	}

	// For a 2D action game, tiles are laid out on the X/Y plane (screen-facing),
	// with Z fixed (depth into the screen) rather than X/Z (top-down ground plane).
	// Row 0 (the top row in the ImGui grid editor) is mapped to the highest world Y,
	// so what you see in the editor matches "up" on screen (assuming a Y-up world/camera).

	//Vector3 GridToWorld(int x, int y) const {
	//	float worldX = static_cast<float>(x) * tileSize_;
	//	float worldY = static_cast<float>(height_ - 1 - y) * tileSize_;
	//	return { worldX, worldY, 0.0f };
	//}

	static int64_t MakeKey(int x, int y) {
		return (static_cast<int64_t>(y) << 32) | static_cast<uint32_t>(x);
	}

	Model* GetOrLoadModel(int typeId) {
		auto it = models_.find(typeId);
		if (it != models_.end()) return it->second.get();

		auto defIt = blockTypes_.find(typeId);
		if (defIt == blockTypes_.end()) return nullptr; // unknown type id, ignore

		auto model = std::make_unique<Model>();
		model->Initialize(engineCommon_);
		model->LoadModel(defIt->second.modelDirectory, defIt->second.modelFilename);
		Model* raw = model.get();
		models_.emplace(typeId, std::move(model));
		return raw;
	}

	void RebuildTileInstance(int x, int y) {
		int64_t key = MakeKey(x, y);

		// IMPORTANT: Do not destroy an existing instance here. It may still be
		// referenced by a command list recorded earlier this frame (e.g. this
		// TileMap's own Draw() call) that hasn't been submitted/executed yet.
		// Destroying its ID3D12Resource objects now would cause
		// "OBJECT_DELETED_WHILE_STILL_IN_USE". Instead, move it to a pending
		// list and let Update() free it next frame, once the GPU is done.
		auto existing = instances_.find(key);
		if (existing != instances_.end()) {
			pendingRemoval_.push_back(std::move(existing->second));
			instances_.erase(existing);
		}

		int typeId = grid_[y][x];
		if (typeId == kEmpty) {
			return; // tile cleared, nothing to (re)create
		}

		Model* model = GetOrLoadModel(typeId);
		if (!model) {
			return; // unknown type id, treat as empty
		}

		auto block = std::make_unique<Block>();
		block->Initialize(engineCommon_, model);
		block->SetPosition(GridToWorld(x, y));
		instances_[key] = std::move(block);
	}

	void RebuildAllInstances() {
		// Same deferred-destruction concern as RebuildTileInstance: this can be
		// triggered mid-frame (Resize/Load buttons in the editor), so don't
		// destroy the old instances synchronously.
		for (auto& [key, block] : instances_) {
			pendingRemoval_.push_back(std::move(block));
		}
		instances_.clear();

		for (int y = 0; y < height_; ++y) {
			for (int x = 0; x < width_; ++x) {
				if (grid_[y][x] != kEmpty) {
					RebuildTileInstance(x, y);
				}
			}
		}
	}

	EngineCommon* engineCommon_ = nullptr;
	int width_ = 0;
	int height_ = 0;
	float tileSize_ = 1.0f;

	std::vector<std::vector<int>> grid_;
	std::unordered_map<int, BlockTypeDef> blockTypes_;
	std::unordered_map<int, std::unique_ptr<Model>> models_;
	std::unordered_map<int64_t, std::unique_ptr<Block>> instances_;

	// Instances removed/replaced this frame, kept alive until it's safe to
	// actually destroy them (see Update()). See RebuildTileInstance/RebuildAllInstances.
	std::vector<std::unique_ptr<Block>> pendingRemoval_;
};
