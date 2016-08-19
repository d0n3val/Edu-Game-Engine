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
}

// ---------------------------------------------------------
ComponentAnimation::~ComponentAnimation()
{}

// ---------------------------------------------------------
void ComponentAnimation::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
	config.AddBool("Loop", loop);
	config.AddFloat("Speed", speed);
}

// ---------------------------------------------------------
void ComponentAnimation::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
	loop = config->GetBool("Loop", false);
	speed = config->GetFloat("Speed", 1.0f);
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
uint ComponentAnimation::CountBones() const
{
	uint ret = 0;
	RecursiveCountBones(game_object, ret);
	return ret;
}

// ---------------------------------------------------------
void ComponentAnimation::AttachBones()
{
	bones.clear();
	RecursiveAttachBones(game_object);
}

// ---------------------------------------------------------
uint ComponentAnimation::CountAttachedBones() const
{
	return bones.size();
}

// ---------------------------------------------------------
float ComponentAnimation::GetTime() const
{
	return time;
}

// ---------------------------------------------------------
void ComponentAnimation::RecursiveCountBones(const GameObject * go, uint& count) const
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
		RecursiveCountBones(*it, count);
}

// ---------------------------------------------------------
void ComponentAnimation::RecursiveAttachBones(const GameObject * go)
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

