#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include "Math.h"
#include "Component.h"
#include <list>
#include <map>
#include <vector>

class Config;

class GameObject
{
	friend class Component;
public:
	ALIGN_CLASS_TO_16

	GameObject(GameObject* parent, const char* name);
	GameObject(GameObject* parent, const char* name, const float3& translation, const float3& scale, const Quat& rotation );
	virtual ~GameObject();

	bool Save(Config& config, std::map<uint,uint>* duplicate = nullptr) const;
	void Load(Config* config, std::map<GameObject*, uint>& relations);

	void OnStart();
	void OnFinish();
	void OnEnable();
	void OnDisable();

	void OnPlay();
	void OnUpdate(float dt);
	void OnStop();
	void OnPause();
	void OnUnPause();
	void OnGoDestroyed();

	uint GetUID() const;

	void RecalculateBoundingBox();

	bool RecursiveRemoveFlagged();
	Component* CreateComponent(Component::Types type);

	void SetNewParent(GameObject* node, bool recalc_transformation = false);
	GameObject* GetParent() const;

	float3 GetLocalPosition() const;
	float3 GetGlobalPosition() const;

	float3 GetLocalRotation() const;
	Quat GetLocalRotationQ() const;
	float3 GetLocalScale() const;

	void SetLocalPosition(const float3& position);
	void Move(const float3& velocity);
	void Rotate(float angular_velocity);
	void SetLocalRotation(const float3& XYZ_euler_rotation);
	void SetLocalRotation(const Quat& rotation);
	void SetLocalScale(const float3& scale);
	void SetLocalTransform(const float4x4& transform);

	const float4x4& GetGlobalTransformation() const;
	const float4x4& GetLocalTransform() const;

	const float* GetOpenGLGlobalTransform() const;

	void RecursiveCalcGlobalTransform(const float4x4& parent, bool force_recalc);
	void RecursiveCalcBoundingBoxes();

	bool IsActive() const;
	void SetActive(bool active);

	void Draw(bool debug = false) const;
	void OnDebugDraw(bool selected) const;

	bool WasDirty() const;
	bool WasBBoxDirty() const;
	void Remove();
	const AABB& GetLocalBBox() const;

	bool IsUnder(const GameObject* go) const;
	void FindComponents(Component::Types type, std::vector<Component*>& results) const;
	bool HasComponent(Component::Types type) const;

	float3 GetVelocity() const;
	float GetRadius() const;

private:
	AABB local_bbox;
	Quat rotation = Quat::identity;
	float3 rotation_editor = float3::zero;
	mutable float4x4 transform_cache;
	float4x4 transform_global;
	bool calculated_bbox = false;
	bool was_dirty = true;
	mutable bool local_trans_dirty = true;
	float3 velocity = float3::zero;
	float3 last_translation = float3::zero;
	float3 translation = float3::zero;
	float3 scale = float3::one;
	bool active = true;
	GameObject* parent = nullptr;
	float4x4 original_transform;
	uint uid = 0;

public:
	std::string name;
	std::list<GameObject*> childs;
	std::list<Component*> components;
	OBB global_bbox;
	mutable bool visible = false;
	bool flag_for_removal = false;
};

#endif // __GAMEOBJECT_H__