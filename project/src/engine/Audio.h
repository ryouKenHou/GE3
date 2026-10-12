#pragma once

#include <xaudio2.h>
#include <stdint.h>
#include <wrl.h>
#include <assert.h>
#include <fstream>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>


struct ChunkHeader {
	char id[4];
	int32_t size;
};

struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};

struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int bufferSize;
};

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    void Initialize();

	SoundData SoundLoadWave(const char* filename);
	
	bool LoadAudio(const std::string& soundName, const std::string& filePath);
	void PlayAudio(const std::string& soundName);

	void UnloadSound(const std::string& key);
	void SoundUnload(SoundData* soundData);
	void UnloadAll();

	void Shutdown();
private:

	class VoiceCallback : public IXAudio2VoiceCallback {
	public:
		explicit VoiceCallback(AudioSystem* owner) : owner_(owner) {}
		void OnStreamEnd() override;
		void OnVoiceProcessingPassStart(UINT32) override {}
		void OnVoiceProcessingPassEnd() override {}
		void OnBufferStart(void*) override {}
		void OnBufferEnd(void*) override {}
		void OnLoopEnd(void*) override {}
		void OnVoiceError(void*, HRESULT) override {}
	private:
		AudioSystem* owner_;
	};

	void SoundPlayWave(const SoundData& soundData);
	void DestroyFinishedVoice(IXAudio2SourceVoice* voice);

	std::unordered_map<std::string, SoundData> soundRegistry_;

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;	
    IXAudio2MasteringVoice* masterVoice_;	

	std::unique_ptr<VoiceCallback> voiceCallback_;
	std::vector<IXAudio2SourceVoice*> activeVoices_;
	std::mutex voiceMutex_;
};