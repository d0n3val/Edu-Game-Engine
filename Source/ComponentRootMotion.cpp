#include "Globals.h"
#include "ComponentRootMotion.h"

#include "GameObject.h"

#include "Application.h"
#include "ModuleHints.h"

#include "mmgr/mmgr.h"

#define MAX_ANGULAR_SPEED PI/8.0f
#define MAX_LINEAR_SPEED 2.0f

const float3 ComponentRootMotion::local_dir[Count] = { float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f), 
                                                       float3(1.0f, 0.0f, 0.0f), float3(-1.0f, 0.0f, 0.0f) };

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
    float speed_norm = speed.Length();

    if(speed_norm > 0.0001f)
    {
        float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

        GameObject* go   = GetGameObject();

        float3 speed_dir = speed/speed_norm;
        float angle_diff = atan2(speed_dir.x, speed_dir.z)- atan2(local_dir[direction].x, local_dir[direction].z);
		float max_angle  = MAX_ANGULAR_SPEED*dt;
        float angle      = angle_diff > 0.0f ? min(max_angle, angle_diff) : max(-max_angle, angle_diff);

        go->SetLocalRotation((Quat(float3::unitY, angle)*go->GetLocalRotationQ()).Normalized());

        float4x4 transform = go->GetGlobalTransformation();
        float3 global_dir = transform.TransformDir(local_dir[direction]);

        float speed = min(MAX_LINEAR_SPEED*metric_proportion, speed_norm);

        go->SetLocalPosition(go->GetLocalPosition()+global_dir*(speed*dt));
    }
}
