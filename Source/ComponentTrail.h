#ifndef __COMPONENT_TRAIL_H_
#define __COMPONENT_TRAIL_H_

#include "Component.h"

#include "imgui/imgui_color_gradient.h"
#include "imgui/imgui_bezier.h"

#include <deque>

class ResourceTexture;

class ComponentTrail : public Component
{
public:
    ComponentTrail(GameObject* go);
    ~ComponentTrail();

	void         OnPlay       () override;
	void         OnStop       () override;
	void         OnUpdate     (float dt) override;

    void         Draw         ();

    virtual void OnSave(Config& config) const;
    virtual void OnLoad(Config* config);

    static Types GetClassType () { return Trail; }

    const ResourceTexture*  GetTextureRes       () const;
    ResourceTexture*        GetTextureRes       ();

    UID                     GetTexture   () const { return texture; }
    void                    SetTexture   (UID tex);

private:

    friend void DrawTrailComponent(ComponentTrail* trail);

    void UpdateBuffers();

private:

    struct Vertex
    {
        float3 pos;
        float4 color;
        float2 uv;
    };

    struct Segment
    {
        float4x4 transform = float4x4::identity;
        float life_time    = 0.0f;

        Segment() {;}
        Segment(const float4x4& t, float l) : transform(t), life_time(l) {;}
    };

    struct ConfigTrail
    {
        float duration              = 2.5f;
        float min_vertex_distance   = 0.1f;
        float width                 = 1.0f;
    };

    struct RenderObjects
    {
        uint vao                = 0;
        uint vbo                = 0;
        uint ibo                = 0;
        uint reserved_vertices  = 0;
        uint reserved_indices   = 0;
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

    enum TextureMode
    {
        Stretch,
        Wrap,
        TextureCount
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


    ConfigTrail          config_trail;
    UID                  texture            = 0;
    RenderObjects	     render_buffers;
    std::deque<Segment>  segments;
	ColorGradient        color_over_time;
    RenderBlendMode      blend_mode = AdditiveBlend;
    TextureMode          texture_mode = Stretch;
    Interpolator<float>  size_over_time  = Interpolator<float>(1.0f, 1.0f);       
};

#endif /* __COMPONENT_TRAIL_H_ */
