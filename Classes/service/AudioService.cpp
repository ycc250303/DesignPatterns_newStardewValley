#include "service/AudioService.h"
#include "AudioManager.h"

void AudioService::init() {
    AudioManager::getInstance()->playBGM("normal.mp3");
    cachedVolume = 1.0f;
    isMuted = false;
}

void AudioService::update(float dt) {
    // 如需做音量控制，可在此实现
}

void AudioService::setMute(bool mute) {
    isMuted = mute;
    AudioManager::getInstance()->setVolume(isMuted ? 0.0f : cachedVolume);
}

void AudioService::toggleMute() {
    setMute(!isMuted);
}

void AudioService::playBGM(const std::string& file) {
    AudioManager::getInstance()->playBGM(file);
}

void AudioService::pauseBGM() {
    AudioManager::getInstance()->pauseBGM();
}

void AudioService::playBGMForMap(const std::string& mapName, int month, int day) {
    pauseBGM();
    if (mapName == "Town") {
        // 如果是3月3日，播放圣诞BGM
        if (month == 3 && day == 3) {
            playBGM("Christmas.mp3");
        } else {
            playBGM("normal.mp3");
        }
    } else {
        playBGM("normal.mp3");
    }
}

void AudioService::setVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    cachedVolume = volume;
    if (!isMuted) {
        AudioManager::getInstance()->setVolume(cachedVolume);
    }
}