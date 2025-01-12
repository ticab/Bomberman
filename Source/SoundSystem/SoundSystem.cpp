#include "SoundSystem/SoundSystem.hpp"
#include "AssetManager/AssetManager.hpp"
#include "Common/Logs.hpp"
#include "Common/Modules.hpp"
#include "ConfigSystem/ConfigSystem.hpp"
#include <iostream>
#include <random>

// SOUND
// Load audio and save to folder

bool SoundSystem::addSound(int32_t soundID, const std::string& filePath)
{
    // Use AssetManager to get the sound buffer
    auto buffer = Modules::Assets->getSound(filePath);

    if (!buffer)
    {
        LOG("Failed to load sound from path : $", filePath);
        return false;
    }

    soundEffectBuffers[soundID].emplace_back(std::move(buffer));
    return true;
}

// Loading sounds from the configuration file
bool SoundSystem::loadSoundsFromConfig(const std::string& configFilePath)
{
    Modules::Config->addFile(configFilePath);
    ConfigFile configFile = Modules::Config->getFile(configFilePath);

    // Initial ID for the effects
    int32_t soundID       = 1;
    bool    anySoundAdded = false;

    // Define a constant for the sound prefix
    const std::string soundPrefix = "sound";

    // Step through all sections in the config file
    auto sectionNames = configFile.getAllSections();

    // Passage through all sections
    for (const auto& effectName : sectionNames)
    {
        // Get the section
        const auto& section = configFile.getSection(effectName);

        // Check if the section is empty
        if (section.isEmpty())
        {
            LOG("Section $ is empty", effectName);
            // Go to the next section
            continue;
        }

        // Iterate through the keys in the section to gather the sound file paths
        int32_t i = 1;
        while (section.isValuePresent(soundPrefix + std::to_string(i)))
        {
            std::string filePath = section.getValue(soundPrefix + std::to_string(i)).getString();

            // Use AssetManager to get the sound buffer
            auto buffer = Modules::Assets->getSound(filePath);

            if (!buffer)
            {
                LOG("Failed to load sound from $", filePath);
                return false;
            }
            else
            {
                // Load the buffer into the soundEffectBuffers
                soundEffectBuffers[soundID].emplace_back(std::move(buffer));
                LOG("Successfully loaded sound: $", filePath);
            }

            i++;
        }
        // If there are at least one buffer, set the flag
        if (!soundEffectBuffers[soundID].empty())
        {
            anySoundAdded = true;
            // Move to the next soundID
            soundID++;
        }
    }

    return anySoundAdded;
}

// Adding a sound effect that can have multiple sounds
bool SoundSystem::addSounds(int32_t soundID, const std::list<std::string>& filePaths)
{
    // Check if a sound with the same ID already exists
    if (soundEffectBuffers.find(soundID) != soundEffectBuffers.end())
    {
        LOG("Sound effect with ID $ already exists!", soundID);
        return false;
    }

    // Loading sound files
    for (const auto& filePath : filePaths)
    {
        if (!addSound(soundID, filePath))
        {
            LOG("Failed to load sound from $", filePath.c_str());
            return false;
        }
    }

    LOG("Successfully loaded sound effect $", soundID);
    return true;
}

void SoundSystem::playSoundFromBuffer(const std::shared_ptr<sf::SoundBuffer>& buffer, int32_t soundID)
{
    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(*buffer);
    sound->setVolume(soundsVolume);
    sound->play();

    // Saving active sounds
    activeSounds.emplace(soundID, std::move(sound));
}

// Playing sound from the buffer
void SoundSystem::playSound(int32_t soundID)
{
    auto it = soundEffectBuffers.find(soundID);
    if (it != soundEffectBuffers.end() && !it->second.empty())
    {
        const auto& buffers = it->second;

        // Check if there's only one buffer
        if (buffers.size() == 1)
        {
            playSoundFromBuffer(buffers.front(), soundID);
        }
        // More than one sound, pick a random one
        else
        {
            // Create generator and distribution locally
            std::random_device                    randomDevice;
            std::mt19937                          randomEngine{randomDevice()};
            std::uniform_int_distribution<size_t> dist(0, std::distance(buffers.begin(), buffers.end()) - 1);
            auto                                  randomIt = std::next(buffers.begin(), dist(randomEngine));

            playSoundFromBuffer(*randomIt, soundID);
            LOG("Playing random sound:$", soundID);
        }
    }
    else
    {
        LOG("Sound $ not found!", soundID);
    }
}

// Stop sound
void SoundSystem::stopSound(int32_t soundID)
{
    auto it = activeSounds.find(soundID);
    if (it != activeSounds.end())
    {
        it->second->stop();
    }
    else
    {
        LOG("Sound $ not found!", soundID);
    }
}

// Pause sound
void SoundSystem::pauseSound(int32_t soundID)
{
    auto it = activeSounds.find(soundID);
    if (it != activeSounds.end())
    {
        it->second->pause();
    }
    else
    {
        LOG("Sound $ not found!", soundID);
    }
}

// Check if sound is playing
bool SoundSystem::isSoundPlaying(int32_t soundID) const
{
    auto it = activeSounds.find(soundID);
    if (it != activeSounds.end())
    {
        return it->second->getStatus() == sf::Sound::Playing;
    }
    return false;
}

void SoundSystem::setSoundsVolume(float volume)
{
    soundsVolume = volume;
    for (auto& sound : activeSounds)
    {
        sound.second->setVolume(volume);
    }
}

// MUSIC
// Load music from file and store it
bool SoundSystem::addMusic(int32_t musicID, const std::string& filePath)
{
    // Check if music is already in map
    if (musicTracks.find(musicID) != musicTracks.end())
    {
        LOG("Music with id [$] is already in map", musicID);
        return false;
    }

    // Use Asset Manager to get music
    auto music = Modules::Assets->getMusic(filePath);

    if (!music)
    {
        LOG("Failed to load music from $", filePath.c_str());
        return false;
    }

    musicTracks.emplace(musicID, std::move(music));
    return true;
}

// Play music
void SoundSystem::playMusic(int32_t musicID)
{
    auto it = musicTracks.find(musicID);
    if (it != musicTracks.end())
    {
        // check if it is currently playing
        if (it->second == currentMusic)
        {
            if (!isMusicPlaying())
                currentMusic->play();
            return;
        }
        stopMusic();
        currentMusic = it->second;
        currentMusic->setVolume(musicVolume);
        currentMusic->setLoop(true);
        currentMusic->play();
    }
    else
    {
        LOG("Music $ not found!", musicID);
    }
}

// Stop music
void SoundSystem::stopMusic()
{
    if (currentMusic)
    {
        currentMusic->stop();

        // Clear current music after stopping
        currentMusic.reset();
    }
}

// Pause music
void SoundSystem::pauseMusic()
{
    if (currentMusic)
    {
        currentMusic->pause();
    }
}

void SoundSystem::continueMusic()
{
    if (currentMusic && !isMusicPlaying())
    {
        currentMusic->play();
    }
}

// Check if music is playing
bool SoundSystem::isMusicPlaying() const
{
    return currentMusic && currentMusic->getStatus() == sf::Music::Playing;
}

void SoundSystem::terminate()
{
}

void SoundSystem::setMusicVolume(float volume)
{
    musicVolume = volume;
    for (auto& music : musicTracks)
    {
        music.second->setVolume(volume);
    }
}

void SoundSystem::update(float, sf::Window*)
{
    for (auto it = activeSounds.begin(); it != activeSounds.end();)
    {
        if (it->second->getStatus() == sf::SoundSource::Status::Stopped)
        {
            it = activeSounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
