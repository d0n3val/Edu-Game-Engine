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
#include "ComponentSteering.h"
#include "ComponentPath.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "Config.h"
#include "OpenGL.h"
#include "DebugDraw.h"

using namespace std;

// ---------------------------------------------------------
GameObject::GameObject(GameObject* parent, const char* name) : name(name)
{
	uid = App->random->Int();
	SetNewParent(parent);
}

// ---------------------------------------------------------
GameObject::GameObject(GameObject* parent, const char * name, const float3 & translation, const float3 & scale, const Quat & rotation) :
	name(name), translation(translation), scale(scale), rotation(rotation)
{
	uid = App->random->Int();
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
bool GameObject::Save(Config& parent_config, map<uint,uint>* duplicate) const
{
	Config config;

	// This is only useful when we are duplicating already existing gameobjects
	uint uid_to_save = uid;
	uint parent_uid = (parent) ? parent->GetUID() : 0;

	if (duplicate != nullptr)
	{
		uid_to_save = App->random->Int();
		(*duplicate)[uid] = uid_to_save;

		map<uint, uint>::iterator it = duplicate->find(parent_uid);
		if (it != duplicate->end())
			parent_uid = it->second;
	}

	// Save my info
	config.AddUInt("UID", uid_to_save);
	config.AddUInt("ParentUID", parent_uid);

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
		(*it)->Save(parent_config, duplicate);
	}

	return true;
}

// ---------------------------------------------------------
void GameObject::Load(Config * config, map<GameObject*, uint>& relations)
{
	 // UID
	uid = config->GetUInt("UID", uid);
	uint parent = config->GetUInt("ParentUID", 0);
	relations[this] = parent;

	// Name
	name = config->GetString("Name", "Unnamed");

	// Transform
	translation.x = config->GetFloat("Translation", 0.f, 0);
	translation.y = config->GetFloat("Translation", 0.f, 1);
	translation.z = config->GetFloat("Translation", 0.f, 2);

	scale.x = config->GetFloat("Scale", 1.f, 0);
	scale.y = config->GetFloat("Scale", 1.f, 1);
	scale.z = config->GetFloat("Scale", 1.f, 2);

	Quat r;
	r.x = config->GetFloat("Rotation", 0.f, 0);
	r.y = config->GetFloat("Rotation", 0.f, 1);
	r.z = config->GetFloat("Rotation", 0.f, 2);
	r.w = config->GetFloat("Rotation", 1.f, 3);

	SetLocalRotation(r);

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
	last_translation = translation;

	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnPlay();
}

// ---------------------------------------------------------
void GameObject::OnUpdate(float dt)
{
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnUpdate(dt);

	velocity = (last_translation - translation) / dt;
	last_translation = translation;
}

// ---------------------------------------------------------
void GameObject::OnStop()
{
	// go back to the original transform
	SetLocalTransform(original_transform);
	velocity = float3::zero;

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
void GameObject::OnGoDestroyed()
{
	for (list<Component*>::iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnGoDestroyed();
}

// ---------------------------------------------------------
uint GameObject::GetUID() const
{
	return uid;
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
			App->level->quadtree.Erase(*it);
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
	static_assert(Component::Types::Unknown == 10, "code needs update");

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
		case Component::Types::Steering:
			ret = new ComponentSteering(this);
		break;
		case Component::Types::Path:
			ret = new ComponentPath(this);
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
		SetLocalTransform(current_global * new_parent->GetLocalTransform().Inverted());
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
	// We should never go from quat to euler, so we keep a local copy of the angles
	// exposed to the editor. More info why euler angles are evil:
	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
	return rotation_editor;
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
	float3 diff = XYZ_euler_rotation - rotation_editor;
	Quat mod = Quat::FromEulerXYZ(diff.x, diff.y, diff.z);
	rotation = rotation * mod;
	rotation_editor = XYZ_euler_rotation;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalRotation(const Quat& rotation)
{
	this->rotation = rotation;
	rotation_editor = rotation.ToEulerXYZ().Abs();
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalScale(const float3 & scale)
{
	this->scale = scale;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalTransform(const float4x4 & transform)
{
	transform.Decompose(translation, rotation, scale);
	rotation_editor = rotation.ToEulerXYZ().Abs();
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::SetLocalPosition(const float3 & position)
{
	translation = position;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::Move(const float3 & velocity)
{
	translation += velocity;
	local_trans_dirty = true;
}

// ---------------------------------------------------------
void GameObject::Rotate(float angular_velocity)
{
	rotation = rotation * Quat::RotateY(angular_velocity);
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
	return transform_cache;
}

// ---------------------------------------------------------
const float* GameObject::GetOpenGLGlobalTransform() const
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
		local_trans_dirty = false;
		transform_cache = float4x4::FromTRS(translation, rotation, scale);
		transform_global = parent * transform_cache;
		for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
			(*it)->OnUpdateTransform();

		// mmmm ... too brute ?
		App->level->quadtree.Erase(this);
		App->level->quadtree.Insert(this);
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
	bool texture = false;
	bool transparency = false;

	// push this matrix before drawing
	glPushMatrix();
	glMultMatrixf(GetOpenGLGlobalTransform());

	if (debug == true)
		glColor3f(1.0f, 1.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, 0);

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->GetType() == Component::Types::Material && (*it)->IsActive() == true)
		{
			ComponentMaterial* cmaterial = (ComponentMaterial*)(*it);
			const ResourceTexture* tex = (const ResourceTexture*) cmaterial->GetResource();
			if (tex != nullptr && tex->gpu_id > 0)
			{
				texture = true;
				glMatrixMode(GL_TEXTURE);
				glPushMatrix();
				glMultMatrixf(cmaterial->tex_transform.Transposed().ptr());
				glMatrixMode(GL_MODELVIEW);
				//glPushMatrix();

				if (tex->format == ResourceTexture::Format::rgba || tex->format == ResourceTexture::Format::bgra)
				{
					glDisable(GL_CULL_FACE);
					glEnable(GL_BLEND);
					if (cmaterial->alpha_test > 0.0f)
					{
						glEnable(GL_ALPHA_TEST);
						glAlphaFunc(GL_GREATER, cmaterial->alpha_test);
					}
					transparency = true;
				}
				glBindTexture(GL_TEXTURE_2D, tex->gpu_id);
			}
		}
	}

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->GetType() == Component::Types::Geometry && (*it)->IsActive() == true)
		{
			ComponentMesh* cmesh = (ComponentMesh*) (*it);
			const ResourceMesh* mesh = (const ResourceMesh*)  cmesh->GetResource();

			if (mesh == nullptr)
				continue;
			
			if(debug == false)
				glColor4f(cmesh->tint.r, cmesh->tint.g, cmesh->tint.b, cmesh->tint.a);

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
				glEnableClientState(GL_NORMAL_ARRAY);

				glBindBuffer(GL_ARRAY_BUFFER, (cmesh->deformable) ? cmesh->deformable->vbo_normals : mesh->vbo_normals);
				glNormalPointer(GL_FLOAT, 0, NULL);
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

	if (texture == true)
	{
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	
	if (transparency == true)
	{
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}

	glPopMatrix();		
	glUseProgram(0);
}

// ---------------------------------------------------------
void GameObject::OnDebugDraw(bool selected) const
{
	float3 pos = GetGlobalPosition();
	float4x4 m = float4x4::Translate(GetGlobalPosition());

	if(velocity.LengthSq() > 0.0f)
		DebugDrawArrow(-velocity, float3(0.f, local_bbox.HalfSize().y, 0.f), Blue, m);

	if (selected == true)
	{
		DebugDraw(GetGlobalTransformation());

		if (global_bbox.IsFinite() == true)
			DebugDraw(global_bbox, Green);

		Draw(true);
	}

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnDebugDraw(selected);
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

// ---------------------------------------------------------
float3 GameObject::GetVelocity() const
{
	return velocity;
}
