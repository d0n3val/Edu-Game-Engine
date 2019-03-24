#ifndef __COMPONENT_PARTICLE_SYSTEM_H__
#define __COMPONENT_PARTICLE_SYSTEM_H__

#include "Component.h"
#include <vector>

#include "imgui/imgui_color_gradient.h"
#include "imgui/imgui_bezier.h"

class ResourceTexture;

class ComponentParticleSystem : public Component
{
public:

	ComponentParticleSystem  (GameObject* container);
	~ComponentParticleSystem ();

	void                    OnPlay              () override;
	void                    OnStop              () override;
	void                    OnUpdate            (float dt) override;
    void                    Draw                (bool show_billboard);

	void                    OnSave              (Config& config) const override;
	void                    OnLoad              (Config* config) override;

    static Types            GetClassType        () { return ParticleSystem; }

    const ResourceTexture*  GetTextureRes       () const;
    ResourceTexture*        GetTextureRes       ();

    UID                     GetTexture          () const { return texture_info.texture; }
    void                    SetTexture          (UID tex);


private:

    void                    UpdateInstanceBuffer();
    bool                    AddNewParticle      ();

private:

    friend void DrawParticleSystemComponent(ComponentParticleSystem* component);
    friend void DebugDrawParticles(ComponentParticleSystem* particles);

    struct TextureSheet
    {
        uint x_tiles  = 1;
        uint y_tiles  = 1;
        float speed   = 24.0f;
        UID texture   = 0;
    };

    struct Particle
    {
		float4x4 transform     = float4x4::identity;
        float    init_life     = 0.0f;
        float    init_size     = 1.0f;
        float3   init_speed    = float3::one;
        float    init_rotation = 0.0f;
        float    life          = 0.0f;
        float    size          = 1.0f;
        float    rotation      = 0.0f;
        float3   speed         = float3::zero;
        float4   color         = float4::one;
        float    texture_frame = 0.0f;
        float    distance      = 0.0f;
    };

    struct RandomValue
    {
        float range[2] = {0.0f, 0.0f};
		bool random = false;

        float GetValue() const;

        void  Save    (const char* name, Config& config) const;
        void  Load    (const char* name, const Config& config);
    };

    struct InitParams
    {
        uint        max_particles = 100;
        bool        loop          = false;
        float       duration;
        RandomValue life;
        RandomValue speed;
        RandomValue size;
        RandomValue rotation;
        float       whole_speed;
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
        float4  bezier = float4(0.0f, 1.0f, 0.0f, 1.0f);

        Interpolator() {;}
        Interpolator(const T& i, const T& e) : init(i), end(e) {;}

        float GetBezier(float lambda);

        T Interpolate(float lambda)
        {
            return init+(end-init)*ImGui::BezierValue(lambda, (float*)&bezier);
        }
    };

    struct RenderObjects
    {
        uint vao               = 0;
        uint vbo               = 0;
        uint ibo               = 0;
    };

    struct ColorGradient
    {
        ImGradient      gradient;
        ImGradientMark* draggingMark = nullptr;
        ImGradientMark* selectedMark = nullptr;
    };

    enum RenderBlendMode
    {
        AdditiveBlend = 0, 
        AlphaBlend,
        BlendCount
    };

    struct TSortParticles
    {
        ComponentParticleSystem* component;

        explicit TSortParticles(ComponentParticleSystem* co) : component(co) {;}

        bool operator()(uint first, uint second) const
        {
            return component->particles[first].distance > component->particles[second].distance;
        }
    };
    
private:


    uint                   instance_vbo      = 0;
    uint                   num_instances     = 0;

    InitParams             init;
    Emissor                emissor;
    EmissorShape           shape;
    Interpolator<float3>   speed_over_time = Interpolator<float3>(float3::zero, float3::zero);       
    Interpolator<float>    size_over_time  = Interpolator<float>(1.0f, 1.0f);       
	ColorGradient          color_over_time;

    RenderObjects          render_buffers;
    TextureSheet           texture_info;

    std::vector<Particle>  particles;  
    std::vector<uint>      sorted;
    uint                   alive_particles    = 0;
    uint                   last_used_particle = 0;
    float                  elapsed_emission   = 0;
    RenderBlendMode        blend_mode         = AdditiveBlend;
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

