#ifndef __MODULEAUDIO_H__
#define __MODULEAUDIO_H__

#include <vector>
#include "Module.h"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;
struct Mix_Chunk;
typedef struct _Mix_Music Mix_Music;

class GameObject;

class ComponentAudioListener;
class ComponentAudioSource;

class ModuleAudio : public Module
{
public:

	ModuleAudio(bool start_enabled = true);
	~ModuleAudio();

	bool Init(Config* config = nullptr) override;
	
	bool Start(Config* config = nullptr) override;
	update_status PostUpdate(float dt) override;
	bool CleanUp() override;

	// Load audio assets
	ulong Load(const char* file);
	const char* GetFile(uint id) const;
	void Unload(ulong id);

private:

	void UpdateAudio() const;
	void RecursiveUpdateAudio(GameObject* go) const;

	void UpdateListener(ComponentAudioListener* listener) const;
	void UpdateSource(ComponentAudioSource* source) const;
};

#endif // __MODULEAUDIO_H__