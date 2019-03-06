#include "Globals.h"
#include "ComponentRootMotion.h"

#define MAX_ANGULAR_SPEED PI/8.0f;
#define MAX_LINEAR_SPEED 2.0f;

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
    float speed_norm = speed.Lenght();

    if(speed_norm > 0.0001f)
    {
        GameObject* go     = GetGameObject();
        float4x4 transform = go->GetGlobalTransformation();
        float3 cur_dir     = float3::zero;

        switch(direction)
        {
            case Forward:
                cur_dir = transform.Col3(2);
                break;
            case Backward:
                cur_dir = -transform.Col3(2);
                break;
            case Left:
                cur_dir = transform.Col3(0);
                break;
            case Right:
                cur_dir = -transform.Col3(0);
                break;
        };

        float3 speed_dir = speed/speed_norm;
        float angle_diff = math::acos(speed_dir.Dot(cur_dir));
        float max_angle  = MAX_ANGULAR_SPEED*dt;
        float angle      = angle_diff > 0.0f ? min(max_angle, angle_diff) : min(-max_angle, angle_diff);
        Quat rotation(float3::unitY, angle);

        go->SetLocalRotation(go->GetLocalRotation()*rotation);

        cur_dir = rotation.Transform(cur_dir);
        
        // Update linear
    }
}
