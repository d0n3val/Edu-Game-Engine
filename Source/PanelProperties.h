#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Globals.h"
#include "Panel.h"
#include "Resource.h"
#include "ResourceMaterial.h"
#include "Math.h"

#include "ShowTextureDlg.h"
#include "SelectResourceDlg.h"

class GameObject;
class Component;
class ComponentAudioSource;
class ComponentAudioListener;
class ComponentCamera;
class ComponentRigidBody;
class ComponentAnimation;
class ComponentRootMotion;
class ComponentMeshRenderer;
class ComponentParticleSystem;
class ComponentGrass;
class ComponentDecal;
class ResourceMaterial;
class ResourceTexture;
class ResourceMesh;
class AmbientLight;
class DirLight;
class PointLight;
class SpotLight;
class PerlinProperties;
class Skybox;
class SkyboxRollout;

class Texture2D;
class Framebuffer;

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
    void DrawDirLight(DirLight* light);
    void DrawPointLight(PointLight* light);
    void DrawSpotLight(SpotLight* light);
	// TODO move editor code as a component method like component rigidbody
    void DrawParticleSystemComponent(ComponentParticleSystem* component);
	void DrawBatchProperties(ComponentMeshRenderer* component);
	void DrawMeshRendererComponent(ComponentMeshRenderer* component);
    void DrawGrassComponent(ComponentGrass* component);
    void DrawDecalComponent(ComponentDecal* decal);
	void DrawAudioSourceComponent(ComponentAudioSource * component);
	void DrawAudioListenerComponent(ComponentAudioListener * component);
    bool TextureButton(ResourceMaterial* material, ResourceMesh* mesh, uint texture, const char* name);
    UID TextureButton(ResourceTexture* texture, ResourceMesh* mesh, const char* name, int uniqueId);
	void DrawAnimationComponent(ComponentAnimation * component);
	void DrawRootMotionComponent(ComponentRootMotion * component);
    void DrawMaterialResource(ResourceMaterial* material, ResourceMesh* mesh);
    UID DrawResourceType(Resource::Type type, bool opened);
    void DrawMesh(const ResourceMesh* mesh);

private:
    typedef std::unique_ptr<PerlinProperties> PerlinPtr;
    typedef std::unique_ptr<Framebuffer> FramebufferPtr;
    typedef std::unique_ptr<Texture2D> TexturePtr;
    typedef std::unique_ptr<SkyboxRollout> SkyboxPtr;

    FramebufferPtr      convert_fb;
    TexturePtr          diffuse;
    TexturePtr          specular;
    TexturePtr          occlusion;
    TexturePtr          depth;

    SkyboxPtr           skybox;
    PerlinPtr           perlin;

    ShowTextureDlg      show_texture;
    SelectResourceDlg   selectTexture;

};

#endif// __PANELPROPERTIES_H__
