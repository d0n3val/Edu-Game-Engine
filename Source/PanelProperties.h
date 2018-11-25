#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Globals.h"
#include "Panel.h"

class GameObject;
class Component;
class ComponentAudioSource;
class ComponentAudioListener;
class ComponentMaterial;
class ComponentCamera;
class ComponentRigidBody;
class ComponentAnimation;
class ComponentMesh;

class PanelProperties : public Panel
{
public:
	PanelProperties();
	virtual ~PanelProperties();

	void Draw() override;

	UID PickResource(UID resource, int type = -1);
	const GameObject* PickGameObject(const GameObject* current) const;
	void RecursiveDrawTree(const GameObject* go, const GameObject** selected) const;
	bool InitComponentDraw(Component* component, const char * name);

	// TODO move editor code as a component method like component rigidbody
	void DrawMeshComponent(ComponentMesh* component);
	void DrawAudioSourceComponent(ComponentAudioSource * component);
	void DrawMaterialComponent(ComponentMaterial * component);
	void DrawAudioListenerComponent(ComponentAudioListener * component);
	void DrawCameraComponent(ComponentCamera * component);
};

#endif// __PANELPROPERTIES_H__
