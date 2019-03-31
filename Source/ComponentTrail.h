#ifndef __COMPONENT_TRAIL_H_
#define __COMPONENT_TRAIL_H_

#include "Component.h"

#include "imgui/imgui_color_gradient.h"
#include "imgui/imgui_bezier.h"

#include <deque>

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

    ConfigTrail          config_trail;
	RenderObjects	     render_buffers;
    std::deque<Segment>  segments;
	ColorGradient        color_over_time;
};

#endif /* __COMPONENT_TRAIL_H_ */
