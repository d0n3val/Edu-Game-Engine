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
    void                    OnDebugDraw         (bool selected) const;

	void                    OnSave              (Config& config) const override;
	void                    OnLoad              (Config* config) override;

    static Types            GetClassType        () { return ParticleSystem; }

    const ResourceTexture*  GetTextureRes       () const;
    ResourceTexture*        GetTextureRes       ();

    UID                     GetTexture          () const { return texture_info.texture; }
    void                    SetTexture          (UID tex);

    bool                    GetVisible          () const { return visible; }
    float                   GetLayer            () const { return layer; }


private:

    void                    UpdateInstanceBuffer();
    bool                    AddNewParticle      ();

private:

    friend void DrawParticleSystemComponent(ComponentParticleSystem* component);
    friend void DebugDrawParticles(ComponentParticleSystem* particles);

    struct Particle
    {
		float4x4 transform     = float4x4::identity;
        float    init_life     = 0.0f;
        float    init_size     = 1.0f;
        float3   init_speed    = float3::one;
        float    init_rotation = 0.0f;
        float4   init_color    = float4::one;
        float    life          = 0.0f;
        float    size          = 1.0f;
        float    rotation      = 0.0f;
        float    gravity       = 0.0f;
        float3   speed         = float3::zero;
        float4   color         = float4::one;
        float    texture_frame = 0.0f;
        float    distance      = 0.0f;
    };

    struct RandomValue
    {
        float range[2] = {0.0f, 0.0f};
		bool random = false;

		RandomValue() { ; }
        RandomValue(float a, float b) { range[0] = a; range[1] = b; }

        float GetValue() const;

        void  Save    (const char* name, Config& config) const;
        void  Load    (const char* name, const Config& config);
    };

    struct InitParams
    {
        uint        max_particles = 100;
        bool        loop          = true;
        float       duration      = 10.0f;
        RandomValue life          = RandomValue(1.0f, 1.0f);
        RandomValue speed         = RandomValue(1.0f, 1.0f);
        RandomValue size          = RandomValue(1.0f, 1.0f);
        RandomValue rotation      = RandomValue(0.0f, 0.0f);
        RandomValue gravity       = RandomValue(1.0f, 1.0f);
        float4      color         = float4::one;
        float       whole_speed   = 1.0f;

    };

    struct Emitter
    {
        uint particles_per_second   = 10;
        uint particles_per_distance = 0;
        uint bursts                 = 0;
    };

    enum ShapeType
    {
        Circle = 0,
        Cone,
        ShapeCount
    };

    struct EmitterShape
    {
        ShapeType type   = Cone;
        float     angle  = 0.45f;
        float     radius = 0.2f;
    };

    template<class T>
    struct Interpolator
    {
        T init;
        T end;
        float4  bezier = float4(0.0f, 0.0f, 0.0f, 0.0f);

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

    struct TextureSheet
    {
        uint                x_tiles         = 1;
        uint                y_tiles         = 1;
        UID                 texture         = 0;
        Interpolator<float> frame_over_time = Interpolator<float>(0.0f, 0.0f);
        bool                random          = false;
    };

    
private:


    uint                   instance_vbo      = 0;
    uint                   num_instances     = 0;

    InitParams             init;
    Emitter                emitter;
    EmitterShape           shape;
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
    bool                   visible            = true;
    float                  layer              = 0;
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

