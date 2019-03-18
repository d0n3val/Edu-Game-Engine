#ifndef __COMPONENT_PARTICLE_SYSTEM_H__
#define __COMPONENT_PARTICLE_SYSTEM_H__

#include "Component.h"
#include <vector>

class ResourceTexture;

class ComponentParticleSystem : public Component
{
public:

	ComponentParticleSystem  (GameObject* container);
	~ComponentParticleSystem ();

	void                    OnPlay          () override;
	void                    OnStop          () override;
	void                    OnUpdate        (float dt) override;
    void                    Draw            ();

	void                    OnSave          (Config& config) const override;
	void                    OnLoad          (Config* config) override;

    static Types            GetClassType    () { return ParticleSystem; }

    const ResourceTexture*  GetTextureRes   () const;
    ResourceTexture*        GetTextureRes   ();

    UID                     GetTexture      () const { return texture; }
    void                    SetTexture      (UID tex);

    void                    GetSheetTiles   (uint& x, uint& y) const { x = sheet_animation.x_tiles; y = sheet_animation.y_tiles;}
    void                    SetSheetTiles   (uint x, uint y)  { sheet_animation.x_tiles = x; sheet_animation.y_tiles = y;}
    float                   GetSheetSpeed   () const { return sheet_animation.speed; }
    void                    SetSheetSpeed   (float s)  { sheet_animation.speed = s; }

    void                    AddParticle     ();

private:

    void                    UpdateParticles ();

private:

    struct TextureSheet
    {
        uint x_tiles  = 1;
        uint y_tiles  = 1;
        float current = 0.0f;
        float speed   = 24.0f;
    };

    uint vao               = 0;
    uint vbo               = 0;
    uint ibo               = 0;
    const uint vertex_size = sizeof(float3)+sizeof(float2);

    UID texture = 0;

    TextureSheet sheet_animation;

    struct TParticle
    {
        float3    position;
        float2    size;
        float4x4  transform;
    };

    std::vector<TParticle> particles; 
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

