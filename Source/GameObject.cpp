#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleMeshes.h"
#include "ModuleTextures.h"
#include "ModuleLevelManager.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "ComponentBone.h"
#include "ComponentRigidBody.h"
#include "ComponentAnimation.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "Config.h"
#include "OpenGL.h"
#include "DebugDraw.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject(GameObject* parent, const char* name) : name(name)
{
	SetNewParent(parent);
}

// ---------------------------------------------------------
GameObject::GameObject(GameObject* parent, const char * name, const float3 & translation, const float3 & scale, const Quat & rotation) :
	name(name), translation(translation), scale(scale), rotation(rotation)
{
	SetNewParent(parent);
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
bool GameObject::Save(Config& parent_config, int& serialization_id, const GameObject* parent) const
{
	Config config;

	// Save my info
	this->serialization_id = serialization_id++;
	config.AddInt("File UID", this->serialization_id);
	config.AddInt("Parent UID", parent->serialization_id);
	config.AddString("Name", name.c_str());

	config.AddArrayFloat("Translation", (float*) &translation, 3);
	config.AddArrayFloat("Scale", (float*) &scale, 3);
	config.AddArrayFloat("Rotation", (float*) &rotation, 4);

	// Now Save all my components
	config.AddArray("Components");
	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		Config component;
		component.AddInt("Type", (*it)->GetType());
		(*it)->OnSave(component);
		config.AddArrayEntry(component);
	}

	parent_config.AddArrayEntry(config);

	// Recursively all children
	for (list<GameObject*>::const_iterator it = childs.begin(); it != childs.end(); ++it)
	{
		(*it)->Save(parent_config, serialization_id, this);
	}

	return true;
}

// ---------------------------------------------------------
void GameObject::Load(Config * config, map<int, GameObject*>& relations)
{
	static int num = 0;

	// Store me for later reference
	relations[config->GetInt("File UID")] = this;
	serialization_id = config->GetInt("Parent UID");

	// Name
	name = config->GetString("Name", "Unnamed");

	// Transform
	translation.x = config->GetFloat("Translation", 0.f, 0);
	translation.y = config->GetFloat("Translation", 0.f, 1);
	translation.z = config->GetFloat("Translation", 0.f, 2);

	scale.x = config->GetFloat("Scale", 1.f, 0);
	scale.y = config->GetFloat("Scale", 1.f, 1);
	scale.z = config->GetFloat("Scale", 1.f, 2);

	rotation.x = config->GetFloat("Rotation", 0.f, 0);
	rotation.y = config->GetFloat("Rotation", 0.f, 1);
	rotation.z = config->GetFloat("Rotation", 0.f, 2);
	rotation.w = config->GetFloat("Rotation", 1.f, 3);

	// Now Load all my components
	int count = config->GetArrayCount("Components");

	for (int i = 0; i < count; ++i)
	{
		Config component_conf(config->GetArray("Components", i));
		Component::Types type = (Component::Types)component_conf.GetInt("Type", Component::Types::Unknown);
		if (type != Component::Types::Unknown)
		{
			Component* component = CreateComponent(type);
			component->OnLoad(&component_conf);
		}
		else
			LOG("Cannot load component type UNKNOWN for gameobject %s", name.c_str());
	}
}

// ---------------------------------------------------------
void GameObject::OnStart()
{
	// Called after all loading is done
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnStart();
}

// ---------------------------------------------------------
void GameObject::OnFinish()
{
	// Called just before deleting the component
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnFinish();
}

// ---------------------------------------------------------
void GameObject::OnEnable()
{
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnActivate();
}

// ---------------------------------------------------------
void GameObject::OnDisable()
{
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnDeActivate();
}

// ---------------------------------------------------------
void GameObject::OnPlay()
{
	// Save transform setup from the editor
	original_transform = float4x4::FromTRS(translation, rotation, scale);

	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnPlay();
}

// ---------------------------------------------------------
void GameObject::OnStop()
{
	// go back to the original transform
	original_transform.Decompose(translation, rotation, scale);
	local_trans_dirty = true;

	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnStop();
}

// ---------------------------------------------------------
void GameObject::OnPause()
{
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnPause();
}

// ---------------------------------------------------------
void GameObject::OnUnPause()
{
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnUnPause();
}

// ---------------------------------------------------------
void GameObject::RecalculateBoundingBox()
{
	local_bbox.SetNegativeInfinity();

	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->IsActive()) 
			(*it)->GetBoundingBox(local_bbox);
	}
}

// ---------------------------------------------------------
bool GameObject::RecursiveRemoveFlagged()
{
	bool ret = false;

	for (list<Component*>::iterator it = components.begin(); it != components.end();)
	{
		if ((*it)->flag_for_removal == true)
		{
			(*it)->OnFinish();
			RELEASE(*it);
			it = components.erase(it);
		}
		else
			++it;
	}

	for (list<GameObject*>::iterator it = childs.begin(); it != childs.end();)
	{
		if ((*it)->flag_for_removal == true)
		{
			(*it)->OnFinish();
			RELEASE(*it);
			it = childs.erase(it);
			ret = true;
		}
		else
		{
			// Keep looking, hay millones de premios
			ret |= (*it)->RecursiveRemoveFlagged();
			++it;
		}
	}

	return ret;
}

// ---------------------------------------------------------
Component* GameObject::CreateComponent(Component::Types type)
{
	Component* ret = nullptr;

	switch (type)
	{
		case Component::Types::Material:
			ret = new ComponentMaterial(this);
		break;
		case Component::Types::Geometry:
			ret = new ComponentMesh(this);
		break;
		case Component::Types::AudioListener:
			ret = new ComponentAudioListener(this);
		break;
		case Component::Types::AudioSource:
			ret = new ComponentAudioSource(this);
		break;
		case Component::Types::Camera:
			ret = new ComponentCamera(this);
		break;
		case Component::Types::Bone:
			ret = new ComponentBone(this);
		break;
		case Component::Types::RigidBody:
			ret = new ComponentRigidBody(this);
		break;
		case Component::Types::Animation:
			ret = new ComponentAnimation(this);
		break;
	}

	if (ret != nullptr)
		components.push_back(ret);

	return ret;
}

// ---------------------------------------------------------
void GameObject::SetNewParent(GameObject * new_parent, bool recalc_transformation)
{
	float4x4 current_global = GetGlobalTransformation();

	if (new_parent == parent)
		return;

	if (parent)
		parent->childs.remove(this);

	parent = new_parent;

	if (new_parent)
		new_parent->childs.push_back(this);

	local_trans_dirty = true;

	// we want to keep the same global transformation even if we are somewhere else in
	// transformation hierarchy
	if (recalc_transformation == true)
	{
		float4x4 new_local = current_global * new_parent->GetLocalTransform().Inverted();
		new_local.Decompose(translation, rotation, scale);
	}
}

// ---------------------------------------------------------
GameObject* GameObject::GetParent() const
{
	return parent;
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
Quat GameObject::GetLocalRotationQ() const
{
	return rotation;
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
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalRotation(const Quat& rotation)
{
	this->rotation = rotation;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalScale(const float3 & scale)
{
	this->scale = scale;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalPosition(const float3 & position)
{
	translation = position;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
const float4x4& GameObject::GetGlobalTransformation() const
{
	return transform_global;
}

// ---------------------------------------------------------
const float4x4& GameObject::GetLocalTransform() const
{
	if (local_trans_dirty == true)
	{
		local_trans_dirty = false;
		transform_cache = float4x4::FromTRS(translation, rotation, scale);
	}

	return transform_cache;
}

// ---------------------------------------------------------
const float* GameObject::GetOpenGLGlobalTranform() const
{
	return transform_global.Transposed().ptr();
}

// ---------------------------------------------------------
void GameObject::RecursiveCalcGlobalTransform(const float4x4& parent, bool force_recalc)
{
	if (local_trans_dirty == true || force_recalc)
	{
		force_recalc = true;
		was_dirty = true;
		transform_global = parent * GetLocalTransform();
		for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
			(*it)->OnUpdateTransform();
	}
	else
		was_dirty = false;

	for(list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		(*it)->RecursiveCalcGlobalTransform(transform_global, force_recalc);
}

// ---------------------------------------------------------
void GameObject::RecursiveCalcBoundingBoxes()
{
	if (was_dirty == true)
	{
		RecalculateBoundingBox();

		// Now generate a OBB global_bbox with world coordinates
		global_bbox = local_bbox;

		if (global_bbox.IsFinite() == true)
			global_bbox.Transform(GetGlobalTransformation());
	}

	for (list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
		(*it)->RecursiveCalcBoundingBoxes();
}

// ---------------------------------------------------------
bool GameObject::IsActive() const
{
	return active;
}

// ---------------------------------------------------------
void GameObject::SetActive(bool active)
{
	// TODO: should this disable all childs recursively ?
	if (this->active != active) 
	{
		this->active = active;
		if (active)
			OnEnable();
		else
			OnDisable();
	}
}

// ---------------------------------------------------------
// TODO: move the draw to ModuleRenderer
void GameObject::Draw(bool debug) const
{
	visible = true;
	bool transparency = false;

	// push this matrix before drawing
	glPushMatrix();
	glMultMatrixf(GetOpenGLGlobalTranform());

	if (debug == true)
		glColor3f(1.0f, 1.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, 0);

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		if ((*it)->GetType() == Component::Types::Material)
		{
			ComponentMaterial* cmaterial = (ComponentMaterial*)(*it);
			const ResourceTexture* tex = (const ResourceTexture*) cmaterial->GetResource();
			if (tex != nullptr && tex->gpu_id > 0)
			{
				if (tex->format == ResourceTexture::Format::rgba || tex->format == ResourceTexture::Format::bgra)
				{
					glEnable(GL_BLEND);
					transparency = true;
				}
				glBindTexture(GL_TEXTURE_2D, tex->gpu_id);
			}
		}
	}

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		if ((*it)->GetType() == Component::Types::Geometry)
		{
			ComponentMesh* cmesh = (ComponentMesh*) (*it);
			const ResourceMesh* mesh = (const ResourceMesh*)  cmesh->GetResource();

			if (mesh == nullptr)
				continue;
			
			if (cmesh->deformable != nullptr)
			{
				glBindBuffer(GL_ARRAY_BUFFER, cmesh->deformable->vbo_vertices);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, cmesh->deformable->vertices, GL_STATIC_DRAW);

				if (mesh->normals != nullptr)
				{
					glBindBuffer(GL_ARRAY_BUFFER, cmesh->deformable->vbo_normals);
					glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, cmesh->deformable->vertices, GL_STATIC_DRAW);
				}
			}

			if (debug == false && mesh->vbo_normals > 0)
			{
				glEnable(GL_LIGHTING);
				//glEnableClientState(GL_NORMAL_ARRAY);

				glBindBuffer(GL_ARRAY_BUFFER, (cmesh->deformable) ? cmesh->deformable->vbo_normals : mesh->vbo_normals);
				glNormalPointer(3, GL_FLOAT, NULL);
			}
			else
				glDisable(GL_LIGHTING);

			glEnableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, (cmesh->deformable) ? cmesh->deformable->vbo_vertices : mesh->vbo_vertices);
			glVertexPointer(3, GL_FLOAT, 0, NULL);

			if (mesh->vbo_texture_coords > 0)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_texture_coords);
				glTexCoordPointer(3, GL_FLOAT, 0, NULL);
			}

			if (mesh->vbo_colors > 0)
			{
				glEnableClientState(GL_COLOR_ARRAY);
				glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_colors);
				glColorPointer(3, GL_FLOAT, 0, NULL);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_indices);
			glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
	}

	if (transparency == true)
		glDisable(GL_BLEND);

	glPopMatrix();		
	glUseProgram(0);
}

// ---------------------------------------------------------
void GameObject::OnDebugDraw() const
{
	DebugDraw(GetGlobalTransformation());

	if (global_bbox.IsFinite() == true) 
		DebugDraw(global_bbox, Green);

	Draw(true);

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnDebugDraw();
}

// ---------------------------------------------------------
bool GameObject::WasDirty() const
{
	return was_dirty;
}

// ---------------------------------------------------------
bool GameObject::WasBBoxDirty() const
{
	return calculated_bbox;
}

// ---------------------------------------------------------
void GameObject::Remove()
{
	flag_for_removal = true;
}

// ---------------------------------------------------------
const AABB& GameObject::GetLocalBBox() const
{
	return local_bbox;
}

// ---------------------------------------------------------
bool GameObject::IsUnder(const GameObject* go) const
{
	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
	{
		if (this == *it || IsUnder(*it) == true)
			return true;
	}

	return false;
}

// ---------------------------------------------------------
void GameObject::FindComponents(Component::Types type, vector<Component*>& results) const
{
	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
		if ((*it)->GetType() == type)
			results.push_back(*it);
}