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
		Right,
        Count
	};

    explicit ComponentRootMotion(GameObject* go);
    ~ComponentRootMotion();

    void            OnSave          (Config& config) const override;
    void            OnLoad          (Config* config) override;

	void            OnUpdate        (float dt) override;

    void            Move            (Direction dir, const float3& local_speed) { direction = dir, speed = local_speed; }

    static Types    GetClassType    () { return RootMotion; }

private:

    Direction direction = Forward;
    float3    speed = float3::zero;
	static const float3 local_dir[Count];
};

#endif /* __COMPONENT_ROOT_MOTION_H__ */


