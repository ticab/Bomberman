#pragma once

#include "BaseModule/BaseModule.hpp"
#include <SFML/Audio.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

class SoundSystem : public BaseModule
{
public:
    // SOUNDS: used for small sounds (gun shots, foot steps, etc.)

    // Adding a single sound
    bool addSound(int32_t soundID, const std::string& filePath);

    // Loading sounds from the configuration file
    bool loadSoundsFromConfig(const std::string& configFilePath);

    // Adding a sound that can have multiple sounds
    bool addSounds(int32_t soundID, const std::list<std::string>& filePaths);

    void playSound(int32_t soundID);

    void stopSound(int32_t soundID);

    void pauseSound(int32_t soundID);

    bool isSoundPlaying(int32_t soundID) const;

    void setSoundsVolume(float volume);


    // MUSIC: used to play compressed music that lasts several minutes

    bool addMusic(int32_t musicID, const std::string& filePath);

    void playMusic(int32_t musicID);

    void stopMusic();

    void pauseMusic();

    void continueMusic();

    bool isMusicPlaying() const;

    void terminate() override;

    void setMusicVolume(float volume);

    void update(float, sf::Window*) override;

private:
    //SOUNDS
    // Map that associates a soundID with a list of SoundBuffer objects
    std::map<int32_t, std::list<std::shared_ptr<sf::SoundBuffer>>> soundEffectBuffers;

    // Map of active sounds, paired with their soundID
    std::map<int32_t, std::unique_ptr<sf::Sound>> activeSounds;

    void playSoundFromBuffer(const std::shared_ptr<sf::SoundBuffer>& buffer, int32_t soundID);


    //MUSIC
    // Map that stores music for background music
    std::map<int32_t, std::shared_ptr<sf::Music>> musicTracks;

    // Shared pointer for the currently playing music
    std::shared_ptr<sf::Music> currentMusic;

    float soundsVolume = 50.f;
    float musicVolume  = 50.f;
};
