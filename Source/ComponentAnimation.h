#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include "ComponentWithResource.h"
#include <map>

class ComponentBone;

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

	uint CountBones() const;
	void AttachBones();
	uint CountAttachedBones() const;
	float GetTime() const;

private:
	void RecursiveCountBones(const GameObject * go, uint& count) const;
	void RecursiveAttachBones(const GameObject * go);

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

	bool loop = false;
	float speed = 1.0f;
	bool interpolate = true;

private:
	state current_state = state::unloaded;
	float time = 0.f;
	std::map<uint, ComponentBone*> bones;
};

#endif // __COMPONENT_AUDIOSOURCE_H__