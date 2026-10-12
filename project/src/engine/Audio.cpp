#include "Audio.h"

#pragma comment(lib, "xaudio2.lib")

AudioSystem::AudioSystem() {}

AudioSystem::~AudioSystem() {
    Shutdown();
}

void AudioSystem::Initialize() {
    HRESULT hr;

    hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));

    hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
    assert(SUCCEEDED(hr));

    voiceCallback_ = std::make_unique<VoiceCallback>(this);

    LoadAudio("Test", "resources/Alarm01.wav");
}

SoundData AudioSystem::SoundLoadWave(const char* filename) {
	// Implementation for loading a WAV file into SoundData
    std::ifstream file;
    file.open(filename, std::ios_base::binary);
    assert(file.is_open());

    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0) { assert(0); }
    if (strncmp(riff.type, "WAVE", 4) != 0) { assert(0); }

    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    while (strncmp(format.chunk.id, "fmt ", 4) != 0) {
        file.seekg(format.chunk.size, std::ios_base::cur);
        file.read((char*)&format, sizeof(ChunkHeader));
        assert(!file.eof());
    }

    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);

    ChunkHeader data;
    file.read((char*)&data, sizeof(data));
    if (strncmp(data.id, "JUNK", 4) == 0) {
        file.seekg(data.size, std::ios_base::cur);
        file.read((char*)&data, sizeof(data));
    }

    if (strncmp(data.id, "data", 4) != 0) { assert(0); }

    char* pBuffer = new char[data.size];
    file.read(pBuffer, data.size);
    file.close();

    SoundData soundData = {};
    soundData.wfex = format.fmt;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
    soundData.bufferSize = data.size;

    return soundData;
}

void AudioSystem::UnloadSound(const std::string& key) {
    auto it = soundRegistry_.find(key);
    if (it != soundRegistry_.end()) {
        SoundUnload(&it->second); // Free PCM buffer memory
        soundRegistry_.erase(it);
    }
}

void AudioSystem::UnloadAll() {
    for (auto& [key, data] : soundRegistry_) {
        SoundUnload(&data);
    }
    soundRegistry_.clear();
}

void AudioSystem::SoundUnload(SoundData* soundData) {
    delete[] soundData->pBuffer;
    soundData->pBuffer = 0;
    soundData->bufferSize = 0;
    soundData->wfex = {};
}

bool AudioSystem::LoadAudio(const std::string& soundName, const std::string& filePath) {
    // Avoid re-loading if already cached
    if (soundRegistry_.find(soundName) != soundRegistry_.end()) {
        return true;
    }

    // Load sound data from file (your wave loading logic)
    SoundData data = SoundLoadWave(filePath.c_str());
    if (data.pBuffer == nullptr) {
        return false; 
    }
	soundRegistry_[soundName] = data;
    return true;
}

void AudioSystem::PlayAudio(const std::string& soundName) {
	auto it = soundRegistry_.find(soundName);
	if (it != soundRegistry_.end()) {
		SoundPlayWave(it->second);
	}
}

void AudioSystem::SoundPlayWave(const SoundData& soundData) {
    HRESULT result;
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voiceCallback_.get());
    assert(SUCCEEDED(result));

    {
        std::lock_guard<std::mutex> lock(voiceMutex_);
        activeVoices_.push_back(pSourceVoice);
    }

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.pBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();
}

void AudioSystem::DestroyFinishedVoice(IXAudio2SourceVoice* voice) {
    {
        std::lock_guard<std::mutex> lock(voiceMutex_);
        auto it = std::find(activeVoices_.begin(), activeVoices_.end(), voice);
        if (it != activeVoices_.end()) {
            activeVoices_.erase(it);
        }
    }
    voice->DestroyVoice();
}

void AudioSystem::VoiceCallback::OnStreamEnd() {
    // NOTE: XAudio2 callbacks fire on the audio thread; DestroyVoice must not be
    // called from inside the callback that belongs to that same voice.
    // In production, post this to a worker/queue. For simplicity here we defer
    // destruction to Shutdown()/next PlayAudio cleanup pass, OR use
    // IXAudio2SourceVoice::Stop + a periodic cleanup tick instead of calling
    // DestroyVoice directly in OnStreamEnd.
}

void AudioSystem::Shutdown() {
    UnloadAll();

    {
        std::lock_guard<std::mutex> lock(voiceMutex_);
        for (auto* voice : activeVoices_) {
            voice->DestroyVoice();
        }
        activeVoices_.clear();
    }

    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    xAudio2_.Reset();
}