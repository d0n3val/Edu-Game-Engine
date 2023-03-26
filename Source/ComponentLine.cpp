#include "Globals.h"

#include "ComponentLine.h"
#include "ComponentCamera.h"

#include "Application.h"
#include "ModuleResources.h"
#include "GameObject.h"

#include "OGL.h"
#include "OpenGL.h"

ComponentLine::ComponentLine(GameObject* go) : Component(go, Types::Line)
{
    colorGradient.gradient.setEditAlpha(false);
}

ComponentLine::~ComponentLine()
{
}

void ComponentLine::OnPlay() 
{
    time = 0.0f;
    state = STOPPED;
}

void ComponentLine::OnStop() 
{
    time = 0.0f;
    state = PLAYING;
}

void ComponentLine::Start()
{
    state = STARTING;
}

void ComponentLine::Stop()
{
    state = STOPPING;
    time = 0.0f;
}

void ComponentLine::OnUpdate(float dt) 
{
    if (state == STOPPING && time > fadeOutTime)
    {
        state = STOPPED;
    }
    
    if(state != STOPPED)
    {
        time += dt*speedMult;
    }
}

void ComponentLine::UpdateBuffers()
{
    if(bufferDirty && numBillboards > 0)
    {
        VertexAttrib attribs[]   = { {0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0 }, 
                                     {1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0 },
                                     {2, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0 } };
                                    
        Buffer *vbo_ptr[ATTRIB_COUNT];
        vbo_ptr[ATTRIB_POS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float3)*(numBillboards*2+2), nullptr);
        vbo_ptr[ATTRIB_TEXCOORD] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float2)*(numBillboards*2+2), nullptr);
        vbo_ptr[ATTRIB_COLOR] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(float3)*(numBillboards*2+2), nullptr);

        for (uint i = 0; i < ATTRIB_COUNT; ++i)
            vbo[i].reset(vbo_ptr[i]);

        ebo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(unsigned)*(numBillboards*2+2), nullptr));

        float step = 1.0f/float(numBillboards);

        float3* posPtr = reinterpret_cast<float3*>(vbo[ATTRIB_POS]->Map(GL_WRITE_ONLY));
        float lambda = 0.0f;
        float size = sizeOverTimeRange[0];
        posPtr[0] = float3(0.0f, size*-0.5f, 0.0f);
        posPtr[1] = float3(0.0f, size*0.5f, 0.0f);

        for(uint i=0; i <numBillboards; ++i)
        {
            lambda = step*float(i+1);
            size = sizeOverTimeRange[0]+(sizeOverTimeRange[1]-sizeOverTimeRange[0])*ImGui::BezierValue(lambda, reinterpret_cast<float*>(&sizeOverTimePoints));
            posPtr[i * 2 + 2 + 0] = float3(lambda, size*-0.5f, 0.0f);
            posPtr[i * 2 + 2 + 1] = float3(lambda, size*0.5f, 0.0f);
        }

        vbo[ATTRIB_POS]->Unmap();

        float2* texPtr = reinterpret_cast<float2*>(vbo[ATTRIB_TEXCOORD]->Map(GL_WRITE_ONLY));
        texPtr[0] = float2(0.0f, 0.0f);
        texPtr[1] = float2(0.0f, 1.0f);

        for(uint i=0; i <numBillboards; ++i)
        {
            texPtr[i * 2 + 2 + 0] = float2(step*float(i+1), 0.0f);
            texPtr[i * 2 + 2 + 1] = float2(step*float(i+1), 1.0f);
        }
        
        vbo[ATTRIB_TEXCOORD]->Unmap();
        float3* colorPtr = reinterpret_cast<float3*>(vbo[ATTRIB_COLOR]->Map(GL_WRITE_ONLY));
        lambda = 0.0f;
        float3 color; 
        colorGradient.gradient.getColorAt(lambda, (float*)&color);
        colorPtr[0] = color;
        colorPtr[1] = color;

        for(uint i=0; i <numBillboards; ++i)
        {
            lambda = step*float(i+1);
            colorGradient.gradient.getColorAt(lambda, (float*)&color);
            colorPtr[i * 2 + 2 + 0] = color;
            colorPtr[i * 2 + 2 + 1] = color;
        }
        
        vbo[ATTRIB_COLOR]->Unmap();

        unsigned* indexPtr = reinterpret_cast<unsigned*>(ebo->Map(GL_WRITE_ONLY));
        indexPtr[0] = 0;
        indexPtr[1] = 1;

        for(uint i=0; i <numBillboards; ++i)
        {
            indexPtr[i * 2 + 2 + 0] = i * 2 + 2;
            indexPtr[i * 2 + 2 + 1] = i * 2 + 3;
        }

        ebo->Unmap();

        vao = std::make_unique<VertexArray>(vbo_ptr, ebo.get(), attribs, ATTRIB_COUNT);

        bufferDirty = false;
    }
}

float4x4 ComponentLine::GetModelMatrix(const ComponentCamera* camera) const
{
    float4x4 transform = float4x4::identity;
    const GameObject* owner = GetGameObject();
    const GameObject* targetObj = owner->FindChild("Target", false);
    float3 ownerPos = owner->GetLocalPosition();
    if(targetObj)
    {
        float3 targetPos = targetObj->GetGlobalPosition(); 
        float3 center = (targetPos+ownerPos)*0.5f;
        float3 centerToCam = (camera->GetPos()-center).Normalized();
        float3 right = targetPos-ownerPos;
        float3 normalized = right.Normalized();
        
        float3 up = centerToCam.Cross(normalized);
        float3 front = normalized.Cross(up);

        transform = float4x4(float4(right, 0.0), float4(up, 0.0), float4(front, 0.0), owner->GetGlobalTransformation().Col(3));
    }

    return transform;
}

void ComponentLine::OnSave(Config &config) const 
{
	config.AddUID("Texture", texture.GetUID());
    config.AddFloat("SpeedMult", speedMult);
    config.AddFloat("FadeInTime", fadeInTime);
    config.AddFloat("FadeOutTime", fadeOutTime);
    config.AddFloat2("Tiling", tiling);
    config.AddFloat2("Offset", offset);
    config.AddFloat4("SizeOverTimePoints", sizeOverTimePoints);
    config.AddFloat2("SizeOverTimeRange", sizeOverTimeRange);
    config.AddArray("ColorOverTime");

    const std::list<ImGradientMark*>& marks = colorGradient.gradient.getMarks();

    for(ImGradientMark* markPtr : marks)
    {
        Config mark;
        mark.AddBool("alpha", markPtr->alpha);
        if(markPtr->alpha)
        {
            mark.AddFloat("color", markPtr->color[0]);
        }
        else
        {
            mark.AddFloat4("color", float4((markPtr->color)));
        }

        mark.AddFloat("position", markPtr->position);
        config.AddArrayEntry(mark);
    }

    config.AddInt("NumBillboards", numBillboards);
}

void ComponentLine::OnLoad(Config *config) 
{
    texture = config->GetUID("Texture", 0);
    speedMult = config->GetFloat("SpeedMult", 1.0f);
    fadeInTime = config->GetFloat("FadeInTime", 0.0f);
    fadeOutTime = config->GetFloat("FadeOutTime", 0.0f);
    tiling = config->GetFloat2("Tiling", float2::one);
    offset = config->GetFloat2("Offset", float2::zero);
    sizeOverTimePoints = config->GetFloat4("SizeOverTimePoints", float4::zero);
    sizeOverTimeRange = config->GetFloat2("SizeOverTimeRange", float2::one);

    colorGradient.gradient.clearMarks();

    uint count = config->GetArrayCount("ColorOverTime");
    for(uint i=0; i< count; ++i)
    {
        Config mark = config->GetArray("ColorOverTime", i);
        
        bool alpha = mark.GetBool("alpha", false);
        float position = mark.GetFloat("position", 0.0f); 
        if(alpha)
        {
            float color = mark.GetFloat("color", 1.0f); 
            colorGradient.gradient.addAlphaMark(position, color);
        }
        else
        {
            float4 color = mark.GetFloat4("color", float4::one); 
            colorGradient.gradient.addMark(position, ImColor(color.x, color.y, color.z, 1.0f));
        }
    }

    if(colorGradient.gradient.getMarks().empty())
    {
        colorGradient.gradient.addMark(0.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
    }

    numBillboards = config->GetInt("NumBillboards", 1);

    bufferDirty = true;
}
