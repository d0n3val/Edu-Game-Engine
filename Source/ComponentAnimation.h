#ifndef __COMPONENT_ANIMATION_H__
#define __COMPONENT_ANIMATION_H__

#include "Component.h"
#include "ComponentWithResource.h"
#include <map>

class ComponentBone;
class ResourceAnimation;

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
	bool BlendTo(UID next_animation, float blend_time);

	bool IsPlaying() const;
	bool IsPaused() const;

	int GetCurrentState() const;
	void SwitchChannels();

private:

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
		waiting_to_stop,
		blending,
		waiting_to_blend
	};

	struct Channel
	{
		ComponentAnimation* component = nullptr;

		UID resource = 0;
		float time = 0.0f;
		std::map<uint, ComponentBone*> bones;
		float speed = 1.0f;
		bool loop = false;
		
		const ResourceAnimation* GetResource() const;
		uint CountBones() const;
		void AttachBones();
		uint CountAttachedBones() const;
		float GetTime() const;


	private:
		void RecursiveCountBones(const GameObject * go, uint& count) const;
		void RecursiveAttachBones(const GameObject * go);
	};

	Channel first;
	Channel second;

	Channel* current = &first;
	Channel* next = &second;

	bool interpolate = true;

	float blend_time = 0.f;
	float total_blend_time = 0.f;

private:
	state current_state = state::unloaded;
};

#endif // __COMPONENT_AUDIOSOURCE_H__