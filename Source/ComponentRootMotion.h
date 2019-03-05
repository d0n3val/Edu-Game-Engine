#ifndef __COMPONENT_ROOT_MOTION_H__
#define __COMPONENT_ROOT_MOTION_H__

#include "Component.h"

class ComponentRootMotion : public Component
{
public:

    enum Direction
    {
        Forward = 0,
        Backward,
        Left,
        Right
    }

    explicit ComponentRootMotion(GameObject* go);
    ~ComponentRootMotion();

    void            OnSave          (Config& config) const override;
    void            OnLoad          (Config* config) override;

	void            OnUpdate        (float dt) override;

    void            Move            (Direction dir, const float3& sp) { direction = dir, speed = sp; }

    static Types    GetClassType    () { return RootMotion; }

private:

    MovementType direction;
    float3       speed;
};

#endif /* __COMPONENT_ROOT_MOTION_H__ */


