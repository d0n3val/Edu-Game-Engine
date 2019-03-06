#include "Globals.h"

#include "ComponentSimpleCharacter.h"
#include "ComponentAnimation.h"
#include "ComponentRootMotion.h"

#include "GameObject.h"

#include "Application.h"
#include "ModuleInput.h"

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
        if(App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
        {
            animation->SendTrigger(HashString("move"));
        }
        else if(App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
        {
            animation->SendTrigger(HashString("move"));
        }
        else if(App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
        {
            animation->SendTrigger(HashString("move"));
        }
        else if(App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
        {
            animation->SendTrigger(HashString("move"));
        }
        else
        {
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
