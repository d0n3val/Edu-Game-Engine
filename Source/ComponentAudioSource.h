#ifndef __COMPONENT_AUDIOSOURCE_H__
#define __COMPONENT_AUDIOSOURCE_H__

// Component to hold an audio source in 3D space (both music and fx)

#include "Globals.h"
#include "Component.h"

class ComponentAudioSource : public Component
{
	friend class ModuleAudio;
public:
	ComponentAudioSource (GameObject* container);
	~ComponentAudioSource ();

	bool LoadFile(const char* file);
	void Unload();

	bool Play();
	bool Pause();
	bool UnPause();
	void Stop();

	bool IsPlaying() const;
	bool IsPaused() const;

public:
	ulong id = 0;
	bool is_2d = false;
	float min_distance = 0.f;
	float max_distance = 0.f;
	int cone_angle_in = 360;
	int cone_angle_out = 360;
	float out_cone_vol = 0.f;
	float fade_in = 1.0f;
	float fade_out = 1.0f;

	enum state
	{
		unloaded,
		stopped,
		waiting_to_play,
		playing,
		waiting_to_pause,
		paused,
		waiting_to_unpause,
		waiting_to_stop
	};

private:
	state current_state = state::unloaded;
};

#endif // __COMPONENT_AUDIOSOURCE_H__