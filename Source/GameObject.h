#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <list>
#include "Math.h"

enum ComponentTypes;
class Component;

class GameObject
{
	friend class Component;
public:
	GameObject(const char* name);
	GameObject(const char* name, const float3& translation, const float3& scale, const Quat& rotation );
	virtual ~GameObject();

	void AddChild(GameObject* go);
	Component* CreateComponent(ComponentTypes type);

	float3 GetLocalPosition() const;
	float3 GetGlobalPosition() const;

	float3 GetLocalRotation() const;
	float3 GetLocalScale() const;

	void SetLocalPosition(const float3& position);
	void SetLocalRotation(const float3& XYZ_euler_rotation);
	void SetLocalScale(const float3& scale);

	const float4x4& GetGlobalTransformation() const;
	const float4x4& GetLocalTransform() const;

	const float* GetOpenGLGlobalTranform() const;

	void RecursiveCalcGlobalTransform(const float4x4& parent, bool force_recalc);
	const OBB& RecursiveCalcBoundingBoxes(bool& needs_recalc);

	bool IsActive() const;
	void SetActive(bool active);

	void OnDebugDraw() const;

	bool WasDirty() const;
	bool WasBBoxDirty() const;

public:
	std::string name;
	std::list<GameObject*> childs;
	std::list<Component*> components;
	AABB local_bbox;
	OBB global_bbox;

private:
	bool calculated_bbox = false;
	bool was_dirty = false;
	mutable bool local_trans_dirty = true;
	mutable float4x4 transform_cache;
	float3 translation = float3::zero;
	Quat rotation = Quat::identity;
	float3 scale = float3::one;
	float4x4 transform_global;
	bool active = true;
};

#endif // __GAMEOBJECT_H__