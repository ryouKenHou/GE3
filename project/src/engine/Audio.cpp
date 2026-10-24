#include "Audio.h"

#pragma comment(lib, "xaudio2.lib")

AudioSystem::AudioSystem() {
    // Initialize Media Foundation
    HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    assert(SUCCEEDED(hr));
}

AudioSystem::~AudioSystem() {
    Shutdown();
    MFShutdown();
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

SoundData AudioSystem::SoundLoadMedia(const std::string& filename) {
    HRESULT hr;

    // Convert string path to wide string for Media Foundation
    int wideLen = MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, nullptr, 0);
    std::wstring wfilename(wideLen, 0);
    MultiByteToWideChar(CP_ACP, 0, filename.c_str(), -1, &wfilename[0], wideLen);

    // 1. Create Source Reader
    Microsoft::WRL::ComPtr<IMFSourceReader> pSourceReader;
    hr = MFCreateSourceReaderFromURL(wfilename.c_str(), nullptr, &pSourceReader);
    assert(SUCCEEDED(hr));

    // 2. Configure output format to uncompressed PCM
    Microsoft::WRL::ComPtr<IMFMediaType> pMediaType;
    hr = MFCreateMediaType(&pMediaType);
    assert(SUCCEEDED(hr));

    hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    assert(SUCCEEDED(hr));

    hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    assert(SUCCEEDED(hr));

    hr = pSourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMediaType.Get());
    assert(SUCCEEDED(hr));

    // 3. Get complete audio format
    Microsoft::WRL::ComPtr<IMFMediaType> pOutputMediaType;
    hr = pSourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pOutputMediaType);
    assert(SUCCEEDED(hr));

    WAVEFORMATEX* pWaveFormat = nullptr;
    UINT32 waveFormatSize = 0;
    hr = MFCreateWaveFormatExFromMFMediaType(pOutputMediaType.Get(), &pWaveFormat, &waveFormatSize);
    assert(SUCCEEDED(hr));

    // 4. Read audio sample data into a dynamic buffer
    std::vector<BYTE> audioData;
    while (true) {
        Microsoft::WRL::ComPtr<IMFSample> pSample;
        DWORD dwFlags = 0;
        hr = pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwFlags, nullptr, &pSample);
        assert(SUCCEEDED(hr));

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            break;
        }

        if (pSample) {
            Microsoft::WRL::ComPtr<IMFMediaBuffer> pBuffer;
            hr = pSample->ConvertToContiguousBuffer(&pBuffer);
            assert(SUCCEEDED(hr));

            BYTE* pAudioBytes = nullptr;
            DWORD currentLength = 0;
            hr = pBuffer->Lock(&pAudioBytes, nullptr, &currentLength);
            assert(SUCCEEDED(hr));

            size_t oldSize = audioData.size();
            audioData.resize(oldSize + currentLength);
            memcpy(audioData.data() + oldSize, pAudioBytes, currentLength);

            hr = pBuffer->Unlock();
            assert(SUCCEEDED(hr));
        }
    }

    // 5. Populate SoundData structure
    SoundData soundData = {};
    soundData.wfex = *pWaveFormat;
    CoTaskMemFree(pWaveFormat);

    soundData.bufferSize = static_cast<unsigned int>(audioData.size());
    soundData.pBuffer = new BYTE[soundData.bufferSize];
    memcpy(soundData.pBuffer, audioData.data(), soundData.bufferSize);

    return soundData;
}

void AudioSystem::UnloadSound(const std::string& key) {
    auto it = soundRegistry_.find(key);
    if (it != soundRegistry_.end()) {
        SoundUnload(&it->second);
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
    if (soundRegistry_.find(soundName) != soundRegistry_.end()) {
        return true;
    }

    // Uses Media Foundation loader supporting both WAV and compressed files like MP3
    SoundData data = SoundLoadMedia(filePath);
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

void AudioSystem::VoiceCallback::OnStreamEnd() {}

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