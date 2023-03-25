#pragma once

#include "Component.h"

#include "imgui_color_gradient.h"

#include <memory>

#include "ResHandle.h"
#include "Math.h"

class ResourceTexture;
class Buffer;
class VertexArray;

class ComponentLine : public Component
{
public:
    struct ColorGradient
    {
        ImGradient      gradient;
        ImGradientMark* draggingMark = nullptr;
        ImGradientMark* selectedMark = nullptr;
    };


public:

    ComponentLine(GameObject* go);
    ~ComponentLine();

	void                    OnPlay          () override;
	void                    OnStop          () override;
	void                    OnUpdate        (float dt) override;

    void                    OnSave          (Config& config) const override;
    void                    OnLoad          (Config* config) override;

    const ResHandle&        GetTexture      () const { return texture; }
    ResHandle&              GetTexture()    { return texture; }
    void                    SetTexture      (UID tex) { texture = tex; }

    const VertexArray*      GetVAO          () const {return vao.get();}
    float4x4                GetModelMatrix  () const;
    float                   GetTime         () const { return time; }

    float                   GetSpeedMult    () const { return speedMult; }
    void                    SetSpeedMult    (float mult) { speedMult = mult; }

    const float2&           GetTiling       () const {return tiling;}
    const float2&           GetOffset       () const {return offset;}
    
    void                    SetTiling       (const float2& t) {tiling = t;}
    void                    SetOffset       (const float2& o) {offset = o;}

    const float4&           GetSizeOverTimePoints () const {return sizeOverTimePoints;}
    void                    SetSizeOverTimePoints (const float4& points) { sizeOverTimePoints = points; }

    const float2&           GetSizeOverTimeRange() const {return sizeOverTimeRange; }
    void                    SetSizeOverTimeRange(const float2& range) {sizeOverTimeRange = range; }

    ColorGradient&          GetColorGradient() { return colorGradient; }

private:

    friend void DrawLineComponent(ComponentLine* trail);

    enum Attribs
    {
        ATTRIB_POS = 0,
        ATTRIB_TEXCOORD,
        ATTRIB_COUNT
    };


    ResHandle texture;
    std::unique_ptr<Buffer> vbo[ATTRIB_COUNT];
    std::unique_ptr<Buffer> ebo;
    std::unique_ptr<VertexArray> vao;
    float time = 0.0f;
    float speedMult = 1.0f;
    float2 tiling = float2::one;
    float2 offset = float2::zero;
    float4 sizeOverTimePoints = float4::zero;
    float2 sizeOverTimeRange = float2::one;
    ColorGradient colorGradient;
};