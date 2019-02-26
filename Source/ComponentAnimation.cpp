#include "Globals.h"
#include "ComponentAnimation.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceAnimation.h"
#include "AnimController.h"
#include "gameObject.h"
#include "Component.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
ComponentAnimation::ComponentAnimation(GameObject* container) : Component(container, Types::Animation)
{
    controller = new AnimController;
}

// ---------------------------------------------------------
ComponentAnimation::~ComponentAnimation()
{
    delete controller;

	for (std::vector<Clip>::iterator it = clips.begin(), end = clips.end(); it != end; ++it)
	{
		Resource* res = App->resources->Get(it->resource);

		if (res != nullptr)
		{
			res->Release();
		}
	}
}

// ---------------------------------------------------------
void ComponentAnimation::OnPlay()
{
    if(!clips.empty())
    {
        if(App->resources->Get(clips[0].resource) != nullptr)
        {
            controller->Play(clips[0].resource, 0);
        }
    }
}

// ---------------------------------------------------------
void ComponentAnimation::OnStop()
{
    controller->Stop();
}

// ---------------------------------------------------------
void ComponentAnimation::OnUpdate(float dt)
{
    controller->Update(unsigned(dt*1000));

    if(game_object != nullptr)
    {
        UpdateGO(game_object);
    }
}

// ---------------------------------------------------------
void ComponentAnimation::OnSave(Config& config) const 
{
    config.AddArray("clips");
    for(uint i=0; i< clips.size(); ++i)
    {
        const Clip& clip = clips[i];

        Config clip_entry;
        clip_entry.AddString("Name", clip.name.C_str());
        clip_entry.AddUID("Resource", clip.resource);
        clip_entry.AddBool("Loop", clip.loop);

        config.AddArrayEntry(clip_entry);
    }

    config.AddBool("DebugDraw", debug_draw);
}

// ---------------------------------------------------------
void ComponentAnimation::OnLoad(Config* config) 
{
    uint count = config->GetArrayCount("clips");

    clips.clear();
    clips.resize(count);
    
    for(uint i=0; i< count; ++i)
    {
        Config clip       = config->GetArray("clips", i);
        clips[i].name     = HashString(clip.GetString("Name", ""));
        clips[i].resource = clip.GetUID("Resource", 0);
        clips[i].loop     = clip.GetBool("Loop", false);

        Resource* res = App->resources->Get(clips[i].resource);

        if (res != nullptr)
        {
            res->LoadToMemory();
        }
    }

    debug_draw = config->GetBool("DebugDraw", false);

}

// ---------------------------------------------------------
void ComponentAnimation::UpdateGO(GameObject* go)
{
    float3 position;
    Quat rotation;

    if(controller->GetTransform(HashString(go->name.c_str()), position, rotation))
    {
        go->SetLocalPosition(position);
        go->SetLocalRotation(rotation);
    }

    for(std::list<GameObject*>::iterator it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
    {
        UpdateGO(*it);
    }
}

// ---------------------------------------------------------
void ComponentAnimation::AddClip(const HashString& name, UID resource, bool loop)
{
    clips.push_back(Clip(name, resource, loop));
}

// ---------------------------------------------------------
void ComponentAnimation::RemoveClip(uint index)
{
    clips.erase(clips.begin()+index);
}

// ---------------------------------------------------------
uint ComponentAnimation::FindClip(const HashString& name) const
{
    uint i=0;

    for(uint count = clips.size(); i < count; ++i)
    {
        if(clips[i].name == name)
            break;
    }

    return i;
}

// ---------------------------------------------------------
void ComponentAnimation::AddNode(const HashString& name, const HashString& clip, float speed)
{
    nodes.push_back(Node(name, clip, speed));
}

// ---------------------------------------------------------
void ComponentAnimation::RemoveNode(uint index)
{
    nodes.erase(nodes.begin()+index);
}

// ---------------------------------------------------------
uint ComponentAnimation::FindNode(const HashString& name) const
{
    uint i=0;

    for(uint count = nodes.size(); i < count; ++i)
    {
        if(nodes[i].name == name)
            break;
    }

    return i;
}

// ---------------------------------------------------------
void ComponentAnimation::AddTransition(const HashString& source, const HashString& target, const HashString& trigger, uint blend)
{
    transitions.push_back(Transition(source, target, trigger, blend));
}

// ---------------------------------------------------------
void ComponentAnimation::RemoveTransition(uint index)
{
    transitions.erase(transitions.begin()+index);
}

// ---------------------------------------------------------
uint ComponentAnimation::FindTransition(const HashString& source, const HashString& trigger) const
{
    uint i=0;

    for(uint count = transitions.size(); i < count; ++i)
    {
        if(transitions[i].source == source && transitions[i].trigger == trigger)
            break;
    }

    return i;
}

