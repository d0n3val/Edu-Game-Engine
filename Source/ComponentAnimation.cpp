#include "Globals.h"
#include "ComponentAnimation.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceAnimation.h"
#include "ResourceStateMachine.h"
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
    ResourceStateMachine* res = GetResource();

    if(res != nullptr && res->GetNumClips() > 0)
    {
        UID anim = res->GetClipRes(0);

        if(anim != 0)
        {
            controller->Play(anim, 0);
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
	config.AddUID("Resource", resource);
    config.AddBool("DebugDraw", debug_draw);
}

// ---------------------------------------------------------
void ComponentAnimation::OnLoad(Config* config) 
{
    SetResource(config->GetUID("Resource", 0));
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
bool ComponentAnimation::SetResource(UID uid)
{
    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::state_machine)
    {
        if(res->LoadToMemory() == true)
        {
            resource = uid;

            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------
const ResourceStateMachine* ComponentAnimation::GetResource () const
{
    return static_cast<const ResourceStateMachine*>(App->resources->Get(resource));
}

// ---------------------------------------------------------
ResourceStateMachine* ComponentAnimation::GetResource ()
{
    return static_cast<ResourceStateMachine*>(App->resources->Get(resource));
}

