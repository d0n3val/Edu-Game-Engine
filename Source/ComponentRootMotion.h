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
		//Left,
		//Right,
        Count
	};

    explicit ComponentRootMotion(GameObject* go);
    ~ComponentRootMotion();

    void            OnSave          (Config& config) const override;
    void            OnLoad          (Config* config) override;

	void            OnUpdate        (float dt) override;

    void            Move            (Direction axis, const float3& d, float s) { dir_axis = axis, dir = d.Normalized(); speed = s;}

    static Types    GetClassType    () { return RootMotion; }

private:

    Direction dir_axis = Forward;
    float3    dir      = float3::zero;
    float     speed    = 0.0f;

	static const float3 local_dir[Count];
};

#endif /* __COMPONENT_ROOT_MOTION_H__ */


