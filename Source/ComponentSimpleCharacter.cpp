#include "Globals.h"

#include "ComponentSimpleCharacter.h"
#include "ComponentAnimation.h"
#include "ComponentRootMotion.h"

#include "GameObject.h"

#include "ModuleHints.h"

#include "Application.h"
#include "ModuleInput.h"

#include "mmgr/mmgr.h"

ComponentSimpleCharacter::ComponentSimpleCharacter(GameObject* go) : Component(go, Types::CharacterController)
{
}

ComponentSimpleCharacter::~ComponentSimpleCharacter()
{
}

void ComponentSimpleCharacter::OnSave (Config& config) const
{
}

void ComponentSimpleCharacter::OnLoad(Config* config)
{
}

void ComponentSimpleCharacter::OnUpdate(float dt) 
{
    ComponentAnimation* animation   = GetGameObject()->FindFirstComponent<ComponentAnimation>();
    ComponentRootMotion* motion     = GetGameObject()->FindFirstComponent<ComponentRootMotion>();

    if(animation != nullptr && motion != nullptr)
    {
        float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

        if(App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
        {
            if(App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
            {
                animation->SendTrigger(HashString("move"));
                motion->Move(ComponentRootMotion::Forward, float3::unitX*metric_proportion*0.45f);
            }
            else if(App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
            {
                animation->SendTrigger(HashString("move"));
                motion->Move(ComponentRootMotion::Forward, -float3::unitX*metric_proportion*0.45f);
            }
            else
            {
                animation->SendTrigger(HashString("move"));
                motion->Move(ComponentRootMotion::Forward, float3::unitZ*metric_proportion*0.45f);
            }
        }
        else
        {
            motion->Move(ComponentRootMotion::Forward, float3::zero);
            animation->SendTrigger(HashString("stop"));
        }

        if(App->input->GetKey(SDL_SCANCODE_1))
        {
            animation->SendTrigger(HashString("kill"));
        }

        if(App->input->GetKey(SDL_SCANCODE_2))
        {
            animation->ResetState();
        }
    }
}
