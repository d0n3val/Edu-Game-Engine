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
}

// ---------------------------------------------------------
void ComponentAnimation::OnPlay()
{
    uint index = FindClip(HashString("default"));
    if(index < clips.size())
    {
        if(App->resources->Get(clips[index].resource) != nullptr)
        {
            controller->Play(clips[index].resource, 0);
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
    std::vector<Clip>::iterator it = std::lower_bound(clips.begin(), clips.end(), name, TNearestClip());
    if(it != clips.end() && it->name == name)
    {
        it->resource = resource;
        it->loop = loop;
    }
    else
    {
        clips.insert(it, Clip(name, resource, loop));
    }
}

// ---------------------------------------------------------
uint ComponentAnimation::FindClip(const HashString& name) const
{
	std::vector<Clip>::const_iterator it = std::lower_bound(clips.begin(), clips.end(), name, TNearestClip());
    if(it != clips.end() && it->name == name)
    {
        return it-clips.begin();
    }

    return clips.size();
}
