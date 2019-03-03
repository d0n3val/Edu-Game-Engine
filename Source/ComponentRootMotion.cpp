#include "Globals.h"
#include "ComponentRootMotion.h"


ComponentRootMotion::ComponentRootMotion(GameObject* go) : Component(go, Types::RootMotion)
{
}

ComponentRootMotion::~ComponentRootMotion()
{
}

void ComponentRootMotion::OnSave(Config& config) const
{
}

void ComponentRootMotion::OnLoad(Config* config)
{
}

void ComponentRootMotion::Forward(const float3& speed)
{
}

void ComponentRootMotion::Backward(const float3& speed)
{
}

void ComponentRootMotion::StrafeLeft(const float3& speed)
{
}

void ComponentRootMotion::StrafeRight(const float3& speed)
{
}

