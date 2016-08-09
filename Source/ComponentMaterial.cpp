#include "Globals.h"
#include "ComponentMaterial.h"

// ---------------------------------------------------------
ComponentMaterial::ComponentMaterial(GameObject* container) : Component(container)
{
	type = ComponentTypes::Material;
}

// ---------------------------------------------------------
ComponentMaterial::~ComponentMaterial()
{}

// ---------------------------------------------------------
void ComponentMaterial::OnSave(Config& config) const
{
}

// ---------------------------------------------------------
void ComponentMaterial::OnLoad(Config * config)
{
}
