#pragma once
#include <ostream>
#include <string>
#include "ConvertString.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <Windows.h>

class LogSystem {
private:
	std::ofstream logStream;	

	LogSystem() = default;
	~LogSystem() {
		if (logStream.is_open()) logStream.close();
	}
public:
	static LogSystem& GetInstance() {
		static LogSystem instance;
		return instance;
	}

	void Log(const std::string& message) {
		if (logStream.is_open()) {
			logStream << message << std::endl;
		}
		OutputDebugStringA((message + "\n").c_str());
	}

	void Log(const std::wstring& message) {
		Log(ConvertString(message)); // Just call the string version
	}

	std::ofstream& GetLogStream() {
		return logStream;
	}

	void Initialize() {
		std::filesystem::create_directories("logs");

		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
		std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
		std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
		std::string logFilePath = std::string("logs/") + dateString + ".log";
		logStream.open(logFilePath);
		Log( "LogSystem initialized.");
	}

};

class Log {
public:
	static void Initialize() {
		LogSystem::GetInstance().Initialize();
	}

	static void LogMessage(const std::string& message) {
		LogSystem::GetInstance().Log(message);
	}

	static void LogMessage(const std::wstring& message) {
		LogSystem::GetInstance().Log(message);
	}
};