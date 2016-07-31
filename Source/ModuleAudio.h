#ifndef __MODULEAUDIO_H__
#define __MODULEAUDIO_H__

#include <vector>
#include "glmath.h"
#include "Module.h"

#define DEFAULT_MUSIC_FADE_TIME 2.0f

struct _Mix_Music;
struct Mix_Chunk;
typedef struct _Mix_Music Mix_Music;

class GameObject;

class ModuleAudio : public Module
{
public:

	ModuleAudio(bool start_enabled = true);
	~ModuleAudio();

	bool Init(Config* config = nullptr) final;
	
	update_status PostUpdate(float dt) override;
	bool CleanUp() override;

	// Play a music file
	bool PlayMusic(const char* path, float fade_time = DEFAULT_MUSIC_FADE_TIME);

	// Load a WAV in memory
	unsigned int LoadFx(const char* path);

private:

	void UpdateAudio() const;
	void RecursiveUpdateAudio(const GameObject* go) const;

private:

	unsigned long music = 0;
	std::vector<unsigned long>	fx;
};

#endif // __MODULEAUDIO_H__