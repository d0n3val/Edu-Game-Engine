#ifndef __PANELPROPERTIES_H__
#define __PANELPROPERTIES_H__

// Editor Panel to show the properties of a single GameObject and its components
#include "Globals.h"
#include "Panel.h"
#include "Resource.h"
#include "Math.h"

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
class ResourceMaterial;
class ResourceTexture;
class ResourceMesh;
class AmbientLight;
class DirLight;
class PointLight;
class SpotLight;

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
    void DrawAmbientLight(AmbientLight* light);
    void DrawDirLight(DirLight* light);
    void DrawPointLight(PointLight* light);
    void DrawSpotLight(SpotLight* light);
	// TODO move editor code as a component method like component rigidbody
	void DrawMeshRendererComponent(ComponentMeshRenderer* component);
    void DrawGrassComponent(ComponentGrass* component);
	void DrawAudioSourceComponent(ComponentAudioSource * component);
	void DrawAudioListenerComponent(ComponentAudioListener * component);
    bool TextureButton(ResourceMaterial* material, ResourceMesh* mesh, uint texture, const char* name);
	void DrawAnimationComponent(ComponentAnimation * component);
	void DrawRootMotionComponent(ComponentRootMotion * component);
    void DrawMaterialResource(ResourceMaterial* material, ResourceMesh* mesh);
    UID DrawResourceType(Resource::Type type, bool opened);
    void ShowTextureModal(const ResourceTexture* texture, const ResourceMesh* mesh);

    void GeneratePreview(uint width, uint height, Texture2D* texture, const ResourceMesh* mesh);
    void GeneratePreviewFB(uint width, uint height);
    void GeneratePreviewBlitFB(Texture2D* texture);
    void DrawPreviewUVs(const ResourceMesh* mesh, uint width, uint height);
    void DrawMesh(const ResourceMesh* mesh);

private:

    std::unique_ptr<Framebuffer> convert_fb;
    std::unique_ptr<Texture2D>   diffuse;
    std::unique_ptr<Texture2D>   specular;
    std::unique_ptr<Texture2D>   occlusion;
    std::unique_ptr<Texture2D>   depth;

    std::unique_ptr<Framebuffer> preview_blit_fb;
    std::unique_ptr<Framebuffer> preview_fb;
    std::unique_ptr<Texture2D>   preview_texture;
    uint                         preview_width   = 0;
    uint                         preview_height  = 0;
    bool                         preview_uvs     = false;
    bool                         preview_text    = true;
    math::float4                 uv_color        = math::float4(1.0f, 1.0f, 1.0f, 1.0f);
    uint                         preview_set     = 0;
    float                        preview_zoom    = 100.0;
};

#endif// __PANELPROPERTIES_H__
