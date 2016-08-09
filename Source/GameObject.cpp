#include "Globals.h"
#include "GameObject.h"
#include "ModuleMeshes.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "Config.h"
#include "OpenGL.h"
#include "DebugDraw.h"

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
bool GameObject::Save(Config& config) const
{
	// Save my info
	config.AddString("Name", name.c_str());

	bool test[5] = { true,false,true,true,true };
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

	// ANd recursively all children
	config.AddArray("Childs");
	for (list<GameObject*>::const_iterator it = childs.begin(); it != childs.end(); ++it)
	{
		Config child;
		(*it)->Save(child);
		config.AddArrayEntry(child);
	}

	return true;
}

// ---------------------------------------------------------
void GameObject::Load(Config * config)
{

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
		case ComponentTypes::Camera:
			ret = new ComponentCamera(this);
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
void GameObject::Draw(bool debug) const
{
	visible = true;

	// push this matrix before drawing
	glPushMatrix();
	glMultMatrixf(GetOpenGLGlobalTranform());

	if (debug == false)
	{
		glBindTexture(GL_TEXTURE_2D, 0);

		for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
		{
			if ((*it)->IsActive() == false)
				continue;

			if ((*it)->GetType() == ComponentTypes::Material)
			{
				ComponentMaterial* cmaterial = (ComponentMaterial*)(*it);
				glBindTexture(GL_TEXTURE_2D, cmaterial->material_id);
			}
		}
	}

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		if ((*it)->GetType() == ComponentTypes::Geometry)
		{
			ComponentMesh* cmesh = (ComponentMesh*) (*it);
			const Mesh* mesh = cmesh->GetMesh();

			if (debug == false && mesh->vbo_normals > 0)
			{
				glEnable(GL_LIGHTING);
				//glEnableClientState(GL_NORMAL_ARRAY);
				
				glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_normals);
				glNormalPointer(3, GL_FLOAT, NULL);
			}
			else
				glDisable(GL_LIGHTING);
			  
			glEnableClientState(GL_VERTEX_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_vertices);
			glVertexPointer(3, GL_FLOAT, 0, NULL);

			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_texture_coords);
			glTexCoordPointer(3, GL_FLOAT, 0, NULL);
			  
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_indices);
			glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, NULL);

			glDisableClientState(GL_TEXTURE_COORD_ARRAY );
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY );
		}
	}

	// we no longer need this matrix (all global transforms are already calculated)
	glPopMatrix();		
}

// ---------------------------------------------------------
void GameObject::OnDebugDraw() const
{
	BeginDebugDraw();
	DebugDraw(GetGlobalTransformation());

	if (global_bbox.IsFinite() == true) 
		DebugDraw(global_bbox, Green);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 0.0f);

	Draw(true);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

	for (list<Component*>::const_iterator it = components.begin(); it != components.end(); ++it)
		(*it)->OnDebugDraw();

	EndDebugDraw();
}

// ---------------------------------------------------------
bool GameObject::WasDirty() const
{
	return was_dirty;
}

bool GameObject::WasBBoxDirty() const
{
	return calculated_bbox;
}
