#include "Globals.h"
#include "ComponentAnimation.h"
#include "Application.h"
#include "ModuleAnimation.h"
#include "ModuleResources.h"
#include "ResourceAnimation.h"
#include "gameObject.h"
#include "Component.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
ComponentAnimation::ComponentAnimation(GameObject* container) : Component(container)
{
	type = ComponentTypes::Animation;
	first.component = second.component = this;
}

// ---------------------------------------------------------
ComponentAnimation::~ComponentAnimation()
{}

// ---------------------------------------------------------
void ComponentAnimation::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
	config.AddBool("Loop", current->loop);
	config.AddFloat("Speed", current->speed);
}

// ---------------------------------------------------------
void ComponentAnimation::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
	current->loop = config->GetBool("Loop", false);
	current->speed = config->GetFloat("Speed", 1.0f);
}

// ---------------------------------------------------------
bool ComponentAnimation::SetResource(UID resource)
{
	bool ret = false;

	if(current_state != state::unloaded)
		Unload();

	if (resource != 0)
	{
		Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::animation)
		{
			if(res->LoadToMemory() == true)
			{
				this->resource = resource;
				current->resource = GetResourceUID();
				current_state = state::stopped;
				ret = true;
			}
		}
	}

	return ret;
}

// ---------------------------------------------------------
void ComponentAnimation::Unload()
{
	// TODO: still not a formal way to unload resources
}

// ---------------------------------------------------------
void ComponentAnimation::OnDeActivate()
{
	if (IsPlaying() == true)
		Stop();
}

// ---------------------------------------------------------
bool ComponentAnimation::Play()
{
	bool ret = false;

	if (current_state == state::stopped)
	{
		current_state = state::waiting_to_play;
		ret = true;
	}

	return ret;
}

// ---------------------------------------------------------
bool ComponentAnimation::Pause()
{
	bool ret = false;

	if (current_state == state::playing)
	{
		current_state = state::waiting_to_pause;
		ret = true;
	}

	return ret;
}

// ---------------------------------------------------------
bool ComponentAnimation::UnPause()
{
	bool ret = false;

	if (current_state == state::paused)
	{
		current_state = state::waiting_to_unpause;
		ret = true;
	}

	return ret;
}

// ---------------------------------------------------------
void ComponentAnimation::Stop()
{
	if (current_state == state::playing)
		current_state = state::waiting_to_stop;
}

// ---------------------------------------------------------
bool ComponentAnimation::IsPlaying() const
{
	return current_state == state::playing;
}

// ---------------------------------------------------------
bool ComponentAnimation::IsPaused() const
{
	return current_state == state::paused;
}

// ---------------------------------------------------------
int ComponentAnimation::GetCurrentState() const
{
	return current_state;
}

// ---------------------------------------------------------
void ComponentAnimation::SwitchChannels()
{
	if (current == &first)
	{
		current = &second;
		next = &first;
	}
	else
	{
		current = &first;
		next = &second;
	}
}

// ---------------------------------------------------------
bool ComponentAnimation::BlendTo(UID next_animation, float blend_time)
{
	bool ret = false;

	if (current_state == blending)
		return ret;

	// TODO unload previous animation
	Resource* res = App->resources->Get(next_animation);

	if (res != nullptr && res->GetType() == Resource::animation && res->LoadToMemory() == true)
	{
		next->resource = next_animation;
		next->loop = current->loop;
		next->speed = current->speed; // TODO maybe change that for better visual effect
		next->time = current->time; // TODO worth jumping to the same % time of the current animation ?

		current_state = waiting_to_blend;
		total_blend_time = blend_time;
		this->blend_time = 0.0f;
	}

	return ret;
}

// ---------------------------------------------------------
const ResourceAnimation * ComponentAnimation::Channel::GetResource() const
{
	return (const ResourceAnimation*) App->resources->Get(resource);
}

// ---------------------------------------------------------
uint ComponentAnimation::Channel::CountBones() const
{
	uint ret = 0;
	RecursiveCountBones(component->game_object, ret);
	return ret;
}

// ---------------------------------------------------------
void ComponentAnimation::Channel::AttachBones()
{
	bones.clear();
	RecursiveAttachBones(component->game_object);
}

// ---------------------------------------------------------
uint ComponentAnimation::Channel::CountAttachedBones() const
{
	return bones.size();
}

// ---------------------------------------------------------
float ComponentAnimation::Channel::GetTime() const
{
	return time;
}

// ---------------------------------------------------------
void ComponentAnimation::Channel::RecursiveCountBones(const GameObject * go, uint & count) const
{
	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::Bone)
		{
			const ResourceAnimation* anim = (const ResourceAnimation*) GetResource();
			for (uint i = 0; i < anim->num_keys; ++i)
			{
				if (anim->bone_keys[i].bone_name == (*it)->GetGameObject()->name)
					++count;
			}
		}
	}

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveCountBones( *it, count);
}

// ---------------------------------------------------------
void ComponentAnimation::Channel::RecursiveAttachBones(const GameObject * go)
{
	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::Bone)
		{
			const ResourceAnimation* anim = (const ResourceAnimation*) GetResource();

			for (uint i = 0; i < anim->num_keys; ++i)
			{
				if (anim->bone_keys[i].bone_name == (*it)->GetGameObject()->name)
					bones[i] = (ComponentBone*) (*it);
			}
		}
	}

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveAttachBones(*it);
}
