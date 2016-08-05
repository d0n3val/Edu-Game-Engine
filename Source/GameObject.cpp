#include "Globals.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentMesh.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject(const char* name) : name(name)
{
}

// ---------------------------------------------------------
GameObject::GameObject(const char * name, const float3 & translation, const float3 & scale, const Quat & rotation) :
	name(name), translation(translation), scale(scale), rotation(rotation)
{
}

// ---------------------------------------------------------
GameObject::~GameObject()
{
	for(list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		RELEASE(*it);

	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		RELEASE(*it);
}

// ---------------------------------------------------------
void GameObject::AddChild(GameObject* go)
{
	if (std::find(childs.begin(), childs.end(), go) == childs.end())
		childs.push_back(go);
}

// ---------------------------------------------------------
Component* GameObject::CreateComponent(ComponentTypes type)
{
	Component* ret = nullptr;

	switch (type)
	{
		case ComponentTypes::Material:
			ret = new ComponentMaterial(this);
		break;
		case ComponentTypes::Geometry:
			ret = new ComponentMesh(this);
		break;
		case ComponentTypes::AudioListener:
			ret = new ComponentAudioListener(this);
		break;
		case ComponentTypes::AudioSource:
			ret = new ComponentAudioSource(this);
		break;
	}

	if (ret != nullptr)
		components.push_back(ret);

	return ret;
}

// ---------------------------------------------------------
float3 GameObject::GetLocalPosition() const
{
	return translation;
}

// ---------------------------------------------------------
float3 GameObject::GetGlobalPosition() const
{
	return transform_global.TranslatePart();
}

// ---------------------------------------------------------
float3 GameObject::GetLocalRotation() const
{
	return rotation.ToEulerXYZ();
}

// ---------------------------------------------------------
float3 GameObject::GetLocalScale() const
{
	return scale;
}

// ---------------------------------------------------------
void GameObject::SetLocalRotation(const float3& XYZ_euler_rotation)
{
	rotation = Quat::FromEulerXYZ(XYZ_euler_rotation.x, XYZ_euler_rotation.y, XYZ_euler_rotation.z);
	trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalScale(const float3 & scale)
{
	this->scale = scale;
	trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalPosition(const float3 & position)
{
	translation = position;
	trans_dirty = true;
}

// ---------------------------------------------------------
const float4x4& GameObject::GetGlobalTransformation() const
{
	return transform_global;
}

// ---------------------------------------------------------
const float4x4& GameObject::GetLocalTransform() const
{
	if (trans_dirty == true)
	{
		transform_cache = float4x4::FromTRS(translation, rotation, scale);
		trans_dirty = false;
	}

	return transform_cache;
}

// ---------------------------------------------------------
const float* GameObject::GetOpenGLGlobalTranform() const
{
	return transform_global.Transposed().ptr();
}

// ---------------------------------------------------------
void GameObject::RecursiveCalcGlobalTransform(const float4x4& parent)
{
	transform_global = parent * GetLocalTransform();

	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		(*it)->RecursiveCalcGlobalTransform(transform_global);
}

// ---------------------------------------------------------
const AABB& GameObject::RecursiveCalcBoundingBoxes()
{
	// Iterate all components and generate an ABB enclosing everything in local_bbox
	local_bbox.SetNegativeInfinity();

	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::Geometry) {
			AABB comp_box = ((ComponentMesh*)(*it))->GetBoundingBox();
			if (comp_box.IsFinite())
				local_bbox.Enclose(comp_box);
		}
	}

	// Now generate a OBB global_bbox with world coordinates
	global_bbox = local_bbox;

	if (global_bbox.IsFinite() == true)
		global_bbox.Transform(GetGlobalTransformation());

	for (list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
	{
		(*it)->RecursiveCalcBoundingBoxes();
		//const AABB& box = (*it)->RecursiveCalcBoundingBoxes();
		//if (box.IsFinite() == true)
		//	bounding_box.Enclose(box);
	}

	return local_bbox;
}

// ---------------------------------------------------------
bool GameObject::IsActive() const
{
	return active;
}

// ---------------------------------------------------------
void GameObject::SetActive(bool active)
{
	if (this->active != active)
		this->active = active;
}
