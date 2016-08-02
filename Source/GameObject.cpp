#include "Globals.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentMesh.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject(const aiMatrix4x4& transformation) : name("Unnamed")
{
	aiQuaternion quat;
	transformation.Decompose(extra_scale, quat, extra_translation);

	local_trans = aiMatrix4x4(aiVector3D(), quat, aiVector3D());
}

// ---------------------------------------------------------
GameObject::GameObject(const char* name, const aiMatrix4x4& transformation) : name(name)
{
	aiQuaternion quat;
	transformation.Decompose(extra_scale, quat, extra_translation);
	aiMatrix4x4 m(quat.GetMatrix());
	extra_rotation = m.GetEuler();

	//extra_rotation = quat.GetMatrix().GetEuler();

	//local_trans = aiMatrix4x4(quat.GetMatrix());
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
aiVector3D GameObject::GetLocalForwardVec() const
{
	return aiVector3D(local_trans.c1, local_trans.c2, local_trans.c3);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetGlobalForwardVec() const
{
	return aiVector3D(global_trans.c1, global_trans.c2, global_trans.c3);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetLocalRightVec() const
{
	return aiVector3D(local_trans.a1, local_trans.a2, local_trans.a3);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetGlobalRightVec() const
{
	return aiVector3D(global_trans.a1, global_trans.a2, global_trans.a3);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetLocalUpVec() const
{
	return aiVector3D(local_trans.b1, local_trans.b2, local_trans.b3);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetGlobalUpVec() const
{
	return aiVector3D(global_trans.b1, global_trans.b2, global_trans.b3);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetLocalPosition() const
{
	return extra_translation;
}

// ---------------------------------------------------------
aiVector3D GameObject::GetGlobalPosition() const
{
	return aiVector3D(global_trans.a4, global_trans.b4, global_trans.c4);
}

// ---------------------------------------------------------
aiVector3D GameObject::GetLocalRotation() const
{
	return extra_rotation;
}

// ---------------------------------------------------------
aiVector3D GameObject::GetLocalScale() const
{
	return extra_scale;
}

// ---------------------------------------------------------
void GameObject::SetLocalRotation(const aiVector3D & XYZ_euler_rotation)
{
	extra_rotation = XYZ_euler_rotation;
	trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalScale(const aiVector3D & scale)
{
	extra_scale = scale;
	trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalPosition(const aiVector3D & position)
{
	extra_translation = position;
	trans_dirty = true;
}

// ---------------------------------------------------------
const aiMatrix4x4 GameObject::GetLocalTransformation() const
{
	if (trans_dirty == true)
	{
		aiQuaternion q(extra_rotation.y, extra_rotation.z, extra_rotation.x);
		aiMatrix4x4 extra(extra_scale, q, extra_translation);

		cached_trans = extra * local_trans;
		trans_dirty = false;
	}

	return cached_trans;
}

// ---------------------------------------------------------
const aiMatrix4x4 GameObject::GetGlobalTransformation() const
{
	return global_trans;
}

// ---------------------------------------------------------
const float* GameObject::GetOpenGLGlobalTranform() const
{
	return (const float*)&(aiMatrix4x4(global_trans).Transpose().a1);
}

// ---------------------------------------------------------
void GameObject::RecursiveCalcGlobalTransform(const aiMatrix4x4& parent)
{
	global_trans = parent * GetLocalTransformation();

	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		(*it)->RecursiveCalcGlobalTransform(global_trans);
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
