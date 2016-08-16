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
	ALIGN_CLASS_TO_16

	GameObject(GameObject* parent, const char* name);
	GameObject(GameObject* parent, const char* name, const float3& translation, const float3& scale, const Quat& rotation );
	virtual ~GameObject();

	bool Save(Config& config, int&, const GameObject* parent) const;
	void Load(Config* config, std::map<int, GameObject*>& relations);

	bool RecursiveRemoveFlagged();
	Component* CreateComponent(ComponentTypes type);

	void SetNewParent(GameObject* node, bool recalc_transformation = false);

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
	const AABB& GetLocalBBox() const;

private:
	AABB local_bbox;
	Quat rotation = Quat::identity;
	mutable float4x4 transform_cache;
	float4x4 transform_global;
	bool calculated_bbox = false;
	bool was_dirty = true;
	mutable bool local_trans_dirty = true;
	float3 translation = float3::zero;
	float3 scale = float3::one;
	bool active = true;
	GameObject* parent = nullptr;

public:
	std::string name;
	std::list<GameObject*> childs;
	std::list<Component*> components;
	OBB global_bbox;
	mutable bool visible = false;
	bool flag_for_removal = false;
	mutable int serialization_id = 0;
};

#endif // __GAMEOBJECT_H__