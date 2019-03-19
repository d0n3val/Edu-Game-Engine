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

	void                    OnPlay              () override;
	void                    OnStop              () override;
	void                    OnUpdate            (float dt) override;
    void                    Draw                ();

	void                    OnSave              (Config& config) const override;
	void                    OnLoad              (Config* config) override;

    static Types            GetClassType        () { return ParticleSystem; }

    const ResourceTexture*  GetTextureRes       () const;
    ResourceTexture*        GetTextureRes       ();

    UID                     GetTexture          () const { return texture; }
    void                    SetTexture          (UID tex);

    void                    GetSheetTiles       (uint& x, uint& y) const { x = sheet_animation.x_tiles; y = sheet_animation.y_tiles;}
    void                    SetSheetTiles       (uint x, uint y)  { sheet_animation.x_tiles = x; sheet_animation.y_tiles = y;}
    float                   GetSheetSpeed       () const { return sheet_animation.speed; }
    void                    SetSheetSpeed       (float s)  { sheet_animation.speed = s; }

    void                    AddParticle         ();

private:

    void                    UpdateInstanceBuffer();
    void                    UpdateParticles     ();

private:

    struct TextureSheet
    {
        uint x_tiles  = 1;
        uint y_tiles  = 1;
        float current = 0.0f;
        float speed   = 24.0f;
        UID texture   = 0;
    };

    struct Particle
    {
        float    size      = 1.0f;
		float4x4 transform = float4x4::identity;
    };

    struct InitParams
    {
        uint duration     = 0;
        bool loop         = false;
        uint life         = 0;
        float speed       = 0.0f;
        float size        = 0.0f;
        float whole_speed = 0.0f;
    };

    struct Emissor
    {
        uint particles_per_second   = 0;
        uint particles_per_distance = 0;
        uint bursts                 = 0;
    };

    enum ShapeType
    {
        Circle = 0
    };

    struct EmissorShape
    {
        ShapeType type;
        float     radius;
    };

    template<class T>
    struct Interpolator
    {
        T init_value;
        T end_value;
    };

    struct RenderObjects
    {
        uint vao               = 0;
        uint vbo               = 0;
        uint ibo               = 0;
    };
    
private:


    uint instance_vbo      = 0;
    uint num_instances     = 0;

    InitParams             init;
    Emissor                emissor;
    EmissorShape           shape;
    Interpolator<float>    velocity_over_time;       
    Interpolator<float>    size_over_time;       
    Interpolator<float4>   color_over_time;       

    RenderObjects          render_buffers;
    TextureSheet           texture_info;

    std::vector<Particle>  particles;  // manage life/deads etc ==> todo: particle shape optimization (hummus), smooth particles, gpu particles (compute)
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

