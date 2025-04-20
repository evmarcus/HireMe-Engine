#pragma once
#ifndef AUDIODB_H
#define AUDIODB_H

#include <filesystem>
#include <unordered_map>
#include "utility.h"

namespace fs = std::filesystem;

class AudioDB
{
public:
    static inline std::unordered_map<std::string, Mix_Chunk*> audioClips;

    // Public method to access the single instance of the class
    static AudioDB& getInstance() {
        static AudioDB instance; // Guaranteed to be created only once
        return instance;
    }

    static void Init() {
        // Initialize SDL_mixer
        AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
        AudioHelper::Mix_AllocateChannels(50);
    }

    static Mix_Chunk* GetAudioClip(std::string clipName);

    // make loops -1 to play infinitely
    static void PlayClip(std::string clipName, int channel, int loops);

    static void Play(int channel, std::string clipName, bool doesLoop);

    static int HaltChannel(int channel);

    static void SetVolume(int channel, int volume);

private:

    // Private constructor and destructor to prevent multiple instances
    AudioDB() {}
    ~AudioDB() = default;

    // Delete copy constructor and assignment operator to prevent copying
    AudioDB(const AudioDB&) = delete;
    AudioDB& operator=(const AudioDB&) = delete;

};

#endif