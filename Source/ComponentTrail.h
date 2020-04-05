#ifndef __COMPONENT_TRAIL_H_
#define __COMPONENT_TRAIL_H_

#include "Component.h"

#include "imgui/imgui_color_gradient.h"
#include "imgui/imgui_bezier.h"

#include <deque>

class ResourceTexture;
struct CubicSegment;

class ComponentTrail : public Component
{
public:
    ComponentTrail(GameObject* go);
    ~ComponentTrail();

	void                    OnPlay          () override;
	void                    OnStop          () override;
	void                    OnUpdate        (float dt) override;
    void                    OnDebugDraw     (bool selected) const;

    void                    Draw            ();

    virtual void            OnSave          (Config& config) const;
    virtual void            OnLoad          (Config* config);

    static Types            GetClassType    () { return Trail; }

    const ResourceTexture*  GetTextureRes   () const;
    ResourceTexture*        GetTextureRes   ();

    UID                     GetTexture      () const { return texture; }
    void                    SetTexture      (UID tex);

private:

    struct SegmentInstance
    {
        float3 position;
        float3 normal;
        float  life;
        float  size;
    };

    friend void DrawTrailComponent(ComponentTrail* trail);

    void UpdateBuffers      ();
    void CheckExtraVertices ();
    void GetSegmentInfo     (uint index, std::vector<SegmentInstance>& instances) const;
    void CatmullRomFrom     (uint index, CubicSegment& curve) const;


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
        bool  temporal     = true;
        bool generated     = false;

        Segment() {;}
        Segment(const float4x4& t, float l) : transform(t), life_time(l) {;}
    };

    struct ConfigTrail
    {
        float duration              = 2.5f;
        float min_vertex_distance   = 0.1f;
        float width                 = 1.0f;
        float MinAngleToAddVertices = pi / 24.0f;
        uint NumAddVertices         = 4;
        bool linear                 = false;
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

        ColorGradient()
        {
            gradient.setEditAlpha(false);
        }
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

    struct Interpolator
    {
        float   init   = 1.0f;
        float   end    = 1.0f;
        float4  bezier = float4(0.0f, 0.0f, 0.0f, 0.0f);

        Interpolator() = default;

        float GetBezier(float lambda);

        float Interpolate(float lambda) const
        {
            return init+(end-init)*ImGui::BezierValue(lambda, (float*)&bezier);
        }
    };


    ConfigTrail          config_trail;
    RenderObjects	     render_buffers;
    std::deque<Segment>  segments;
	ColorGradient        color_over_time;
    Interpolator         size_over_time;

    UID                  texture         = 0;
    RenderBlendMode      blend_mode      = AdditiveBlend;
    TextureMode          texture_mode    = Stretch;
    uint                 num_billboards  = 0;

};

#endif /* __COMPONENT_TRAIL_H_ */
