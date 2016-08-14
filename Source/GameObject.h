#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <list>
#include <map>
#include "Math.h"

enum ComponentTypes;
class Component;
class Config;

class GameObject
{
	friend class Component;
public:
	GameObject(GameObject* parent, const char* name);
	GameObject(GameObject* parent, const char* name, const float3& translation, const float3& scale, const Quat& rotation );
	virtual ~GameObject();

	bool Save(Config& config, int&, const GameObject* parent) const;
	void Load(Config* config, std::map<int, GameObject*>& relations);

	bool RecursiveRemoveFlagged();
	Component* CreateComponent(ComponentTypes type);

	void SetNewParent(GameObject* node);

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

	void Draw(bool debug = false) const;
	void OnDebugDraw() const;

	bool WasDirty() const;
	bool WasBBoxDirty() const;
	void Remove();

public:
	std::string name;
	std::list<GameObject*> childs;
	std::list<Component*> components;
	AABB local_bbox;
	OBB global_bbox;
	mutable bool visible = false;
	bool flag_for_removal = false;
	mutable int serialization_id = 0;

private:
	bool calculated_bbox = false;
	bool was_dirty = true;
	mutable bool local_trans_dirty = true;
	mutable float4x4 transform_cache;
	float3 translation = float3::zero;
	Quat rotation = Quat::identity;
	float3 scale = float3::one;
	float4x4 transform_global;
	bool active = true;
	GameObject* parent = nullptr;
};

#endif // __GAMEOBJECT_H__