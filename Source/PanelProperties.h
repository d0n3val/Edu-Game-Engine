#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Globals.h"
#include "Panel.h"
#include "Resource.h"

class GameObject;
class Component;
class ComponentAudioSource;
class ComponentAudioListener;
class ComponentMaterial;
class ComponentCamera;
class ComponentRigidBody;
class ComponentAnimation;
class ComponentRootMotion;
class ComponentMesh;
class ComponentParticleSystem;
class ResourceMaterial;
class AmbientLight;
class DirLight;
class PointLight;
class SpotLight;

class PanelProperties : public Panel
{

public:

	static const uint default_width = 325;
	static const uint default_height = 578;
	static const uint default_posx = 956;
	static const uint default_posy = 21;
    
public:
	PanelProperties();
	virtual ~PanelProperties();

	void Draw() override;

	UID PickResource(UID resource, int type = -1);
	UID PickResourceModal(int type);
    UID OpenResourceModal(int type, const char* popup_name);
	const GameObject* PickGameObject(const GameObject* current) const;
	void RecursiveDrawTree(const GameObject* go, const GameObject** selected) const;
	bool InitComponentDraw(Component* component, const char * name);

	void DrawCameraComponent(ComponentCamera * component);
private:
    void DrawGameObject(GameObject* go);
    void DrawAmbientLight(AmbientLight* light);
    void DrawDirLight(DirLight* light);
    void DrawPointLight(PointLight* light);
    void DrawSpotLight(SpotLight* light);
	// TODO move editor code as a component method like component rigidbody
	void DrawMeshComponent(ComponentMesh* component);
	void DrawAudioSourceComponent(ComponentAudioSource * component);
	void DrawMaterialComponent(ComponentMaterial * component);
	void DrawAudioListenerComponent(ComponentAudioListener * component);
    bool TextureButton(ResourceMaterial* material, uint texture, const char* name);
	void DrawAnimationComponent(ComponentAnimation * component);
	void DrawRootMotionComponent(ComponentRootMotion * component);
    void DrawMaterialResource(ResourceMaterial* material);
    UID DrawResourceType(Resource::Type type, bool opened);
};

#endif// __PANELPROPERTIES_H__
