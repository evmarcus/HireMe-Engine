#include "AudioDB.h"

Mix_Chunk* AudioDB::GetAudioClip(std::string clipName) {
	// If the clip already exists, retrieve it and return
	if (audioClips.find(clipName) != audioClips.end()) {
		return audioClips[clipName];
	}

	// Get the path of the current executable
	fs::path exePath = fs::current_path();

	fs::path clipDir = exePath / "resources" / "audio" / (clipName + ".wav");

	if (!fs::exists(clipDir)) {
		clipDir = exePath / "resources" / "audio" / (clipName + ".ogg");

		if (!fs::exists(clipDir)) {
			std::cout << "error: failed to play audio clip " << clipName;
			exit(0);
		}
	}

	Mix_Chunk* clip = AudioHelper::Mix_LoadWAV(clipDir.string().c_str());

	audioClips[clipName] = clip;

	return clip;
}

void AudioDB::PlayClip(std::string clipName, int channel, int loops) {
	Mix_Chunk* clip = GetAudioClip(clipName);

	AudioHelper::Mix_PlayChannel(channel, clip, loops);
}

void AudioDB::Play(int channel, std::string clipName, bool doesLoop) {
	int loops = (doesLoop == true ? -1 : 0);
	PlayClip(clipName, channel, loops);
}

int AudioDB::HaltChannel(int channel) {
	return AudioHelper::Mix_HaltChannel(channel);
}

void AudioDB::SetVolume(int channel, int volume) {
	if (volume >= 0 && volume <= 128)
		AudioHelper::Mix_Volume(channel, volume);
}
