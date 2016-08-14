#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Globals.h"
#include "Panel.h"

class GameObject;
class Component;
class ComponentMesh;
class ComponentAudioSource;
class ComponentAudioListener;
class ComponentMaterial;
class ComponentCamera;
class ComponentBone;

class PanelProperties : public Panel
{
public:
	PanelProperties();
	virtual ~PanelProperties();

	void Draw() override;

	UID DrawResource(UID resource, int type = -1);

	bool InitComponentDraw(Component* component, const char * name);
	void DrawMeshComponent(ComponentMesh* component);
	void DrawAudioSourceComponent(ComponentAudioSource * component);
	void DrawMaterialComponent(ComponentMaterial * component);
	void DrawAudioListenerComponent(ComponentAudioListener * component);
	void DrawCameraComponent(ComponentCamera * component);
	void DrawBoneComponent(ComponentBone * component);
};

#endif// __PANELPROPERTIES_H__