#include "Globals.h"
#include "ComponentAnimation.h"
#include "Application.h"
#include "ModuleAnimation.h"
#include "ModuleResources.h"
#include "ResourceAnimation.h"

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
}

// ---------------------------------------------------------
void ComponentAnimation::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
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
