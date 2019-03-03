#include "Globals.h"
#include "ComponentSimpleCharacter.h"

ComponentSimpleCharacter::ComponentSimpleCharacter(GameObject* go) : Component(go, Types::Character)
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

