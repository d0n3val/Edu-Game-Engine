#include "Globals.h"
#include "ComponentRootMotion.h"

#include "GameObject.h"

#include "Application.h"
#include "ModuleHints.h"

#include "Leaks.h"

#define MAX_ANGULAR_SPEED PI/4.0f
#define MAX_LINEAR_SPEED 2.0f

const float3 ComponentRootMotion::local_dir[Count] = { float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f)//, 
                                                       /*float3(1.0f, 0.0f, 0.0f), float3(-1.0f, 0.0f, 0.0f)*/ };

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

void ComponentRootMotion::OnUpdate(float dt)
{
    float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

    GameObject* go   = GetGameObject();

    float angle_diff = atan2(dir.x, dir.z)- atan2(local_dir[dir_axis].x, local_dir[dir_axis].z);
    while(angle_diff < -PI) angle_diff += PI*2.0f;
    while(angle_diff > PI) angle_diff -= PI*2.0f;

    float max_angle  = MAX_ANGULAR_SPEED*dt;
    float angle      = angle_diff > 0.0f ? min(max_angle, angle_diff) : max(-max_angle, angle_diff);

    go->SetLocalRotation((Quat(float3::unitY, angle)*go->GetLocalRotationQ()).Normalized());

    float4x4 transform = go->GetGlobalTransformation();
    float3 global_dir  = transform.TransformDir(local_dir[dir_axis]);

    go->SetLocalPosition(go->GetLocalPosition()+global_dir*(min(MAX_LINEAR_SPEED*metric_proportion, speed)*dt));
}
