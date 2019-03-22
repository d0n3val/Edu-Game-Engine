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

    UID                     GetTexture          () const { return texture_info.texture; }
    void                    SetTexture          (UID tex);


private:

    void                    UpdateInstanceBuffer();
    void                    AddNewParticle      ();

private:
    friend void DrawParticleSystemComponent(ComponentParticleSystem* component);
    friend void DebugDrawParticles(ComponentParticleSystem* particles);

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
        float  size        = 1.0f;
		float4x4 transform = float4x4::identity;
        float life         = 0.0f;
        float3 speed       = float3::zero;
        float4 color       = float4::one;
    };

    struct RandomValue
    {
        float init = 0.0f;
        float end  = 0.0f;

        float GetValue() const;
    };

    struct InitParams
    {
        uint max_particles = 100;
        bool loop          = false;
        float duration     = 0;
        float life         = 0.0f;
        float speed        = 0.0f;
        float size         = 0.0f;
        float whole_speed  = 0.0f;
    };

    struct Emissor
    {
        uint particles_per_second   = 0;
        uint particles_per_distance = 0;
        uint bursts                 = 0;
    };

    enum ShapeType
    {
        Circle = 0,
        Cone,
        ShapeCount
    };

    struct EmissorShape
    {
        ShapeType type   = Circle;
        float     angle  = 0.0f;
        float     radius = 1.0f;
    };

    template<class T>
    struct Interpolator
    {
        T init;
        T end;

        Interpolator() {;}
        Interpolator(const T& i, const T& e) : init(i), end(e) {;}

        T Interpolate(float lambda)
        {
            return init*lambda+end*(1.0f-lambda);
        }
    };

    struct RenderObjects
    {
        uint vao               = 0;
        uint vbo               = 0;
        uint ibo               = 0;
    };
    
private:


    uint                   instance_vbo      = 0;
    uint                   num_instances     = 0;

    InitParams             init;
    Emissor                emissor;
    EmissorShape           shape;
    Interpolator<float3>   speed_over_time = Interpolator<float3>(float3::zero, float3::zero);       
    Interpolator<float>    size_over_time  = Interpolator<float>(1.0f, 1.0f);       
	Interpolator<float4>   color_over_time = Interpolator<float4>(float4::one, float4::one);

    RenderObjects          render_buffers;
    TextureSheet           texture_info;

    std::vector<Particle>  particles;  
    uint                   alive_particles    = 0;
    uint                   last_used_particle = 0;
    float                  elapsed_emission   = 0;
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

