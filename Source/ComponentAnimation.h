#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include "ComponentWithResource.h"

class ComponentAnimation : public Component, public ComponentWithResource
{
	friend class ModuleAnimation;
public:
	ComponentAnimation (GameObject* container);
	~ComponentAnimation ();

	void OnSave(Config& config) const override;
	void OnLoad(Config* config) override;

	bool SetResource(UID resource) override;

	void Unload();

	void OnDeActivate() override;

	bool Play();
	bool Pause();
	bool UnPause();
	void Stop();

	bool IsPlaying() const;
	bool IsPaused() const;

	int GetCurrentState() const;

public:
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