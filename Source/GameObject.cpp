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
#include "ComponentSkeleton.h"
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
		ComponentTypes type = (ComponentTypes)component_conf.GetInt("Type", ComponentTypes::Invalid);
		if (type != ComponentTypes::Invalid)
		{
			Component* component = CreateComponent(type);
			component->OnLoad(&component_conf);
		}
		else
			LOG("Cannot load component type INVALID for gameobject %s", name.c_str());
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
		case ComponentTypes::Camera:
			ret = new ComponentCamera(this);
		break;
		case ComponentTypes::Bone:
			ret = new ComponentBone(this);
		break;
		case ComponentTypes::Skeleton:
			ret = new ComponentSkeleton(this);
		break;
		case ComponentTypes::Animation:
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
// TODO: game objects that have mesh components AND childs with bbox
const OBB& GameObject::RecursiveCalcBoundingBoxes(bool& needs_recalc)
{
	// Enclose all childs in a AABB with world coordinates
	AABB tmp;
	tmp.SetNegativeInfinity();

	bool did_change = false;
	calculated_bbox = false;

	for (list<GameObject*>::iterator it = childs.begin(); it != childs.end(); ++it)
	{
		const OBB& box = (*it)->RecursiveCalcBoundingBoxes(did_change);
		if (did_change == true)
			calculated_bbox = true;
		if (box.IsFinite() == true)
			tmp.Enclose(box);
	}

	if (was_dirty == true || calculated_bbox == true)
	{
		needs_recalc = true;

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

		// If we did not have a bbox so far, create an OBB with all childs
		// No transformation since we already used world coordinates
		if (global_bbox.IsFinite() == false)
			global_bbox.SetFrom(tmp);
	}

	return global_bbox;
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

// ---------------------------------------------------------
// TODO: move the draw to ModuleRenderer
void GameObject::Draw(bool debug) const
{
	visible = true;

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

		if ((*it)->GetType() == ComponentTypes::Material)
		{
			ComponentMaterial* cmaterial = (ComponentMaterial*)(*it);
			const ResourceTexture* tex = (const ResourceTexture*) cmaterial->GetResource();
			if(tex != nullptr && tex->gpu_id > 0)
				glBindTexture(GL_TEXTURE_2D, tex->gpu_id);
		}
	}

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		if ((*it)->GetType() == ComponentTypes::Geometry)
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
