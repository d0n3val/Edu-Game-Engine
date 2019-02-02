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
void ComponentAnimation::OnStart()
{
    uint index = FindClip(HashString("default"));
    if(index < clips.size())
    {
        controller->Play(clips[index].resource, 0);
    }
}

// ---------------------------------------------------------
void ComponentAnimation::OnFinish()
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
