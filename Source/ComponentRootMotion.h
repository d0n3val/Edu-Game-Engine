#ifndef __COMPONENT_ROOT_MOTION_H__
#define __COMPONENT_ROOT_MOTION_H__

#include "Component.h"

class ComponentRootMotion : public Component
{
public:
    explicit ComponentRootMotion(GameObject* go);
    ~ComponentRootMotion();

    void OnSave     (Config& config) const override;
    void OnLoad     (Config* config) override;

    void Forward    (const float3& speed);
    void Backward   (const float3& speed);

    void StrafeLeft (const float3& speed);
    void StrafeRight(const float3& speed);

private:
};

#endif /* __COMPONENT_ROOT_MOTION_H__ */


