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
void ComponentMaterial::OnActivate()
{
}

// ---------------------------------------------------------
void ComponentMaterial::OnDeActivate()
{
}

// ---------------------------------------------------------
void ComponentMaterial::OnStart()
{
}

// ---------------------------------------------------------
void ComponentMaterial::OnUpdate()
{
}

// ---------------------------------------------------------
void ComponentMaterial::OnFinish()
{
}