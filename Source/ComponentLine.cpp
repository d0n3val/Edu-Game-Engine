#include "Globals.h"

#include "ComponentLine.h"
#include "Application.h"
#include "ModuleResources.h"
#include "GameObject.h"

#include "OGL.h"
#include "OpenGL.h"

namespace
{
    VertexAttrib attribs[]   = { {0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0 }, 
                                 {1, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, 0 } };
                                 
    const float positions[]   = { 0.0f,  -0.5f, 0.0f , 
                                   1.0f,  -0.5f, 0.0f , 
                                   1.0f,   0.5f, 0.0f , 
                                   0.0f,  0.5f, 0.0f };
                                  
    const float texCoord[]   = { 0.0f,  0.0f, 
                                 1.0f,  0.0f, 
                                 1.0f,  1.0f, 
                                 0.0f,  1.0f };
                                  

    const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

}

ComponentLine::ComponentLine(GameObject* go) : Component(go, Types::Line)
{
    Buffer* vbo_ptr[ATTRIB_COUNT];
    vbo_ptr[ATTRIB_POS] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(positions), positions);
    vbo_ptr[ATTRIB_TEXCOORD] = Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(texCoord), texCoord);

    for(uint i=0; i<ATTRIB_COUNT; ++i) vbo[i].reset(vbo_ptr[i]);


    ebo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(indices), indices));

    vao = std::make_unique<VertexArray>(vbo_ptr, ebo.get(), attribs, ATTRIB_COUNT);
}

ComponentLine::~ComponentLine()
{
}

void ComponentLine::OnPlay() 
{
    time = 0.0f;
}

void ComponentLine::OnStop() 
{
    time = 0.0f;
}

void ComponentLine::OnUpdate(float dt) 
{
    time += dt*speedMult;
}

float4x4 ComponentLine::GetModelMatrix() const
{
    float4x4 transform = float4x4::identity;
    const GameObject* owner = GetGameObject();
    const GameObject* targetObj = owner->FindChild("Target", false);
    float3 ownerPos = owner->GetLocalPosition();
    if(targetObj)
    {
        float3 targetPos = targetObj->GetGlobalPosition(); 
        float3 right = targetPos-ownerPos;
        float3 normalized = right.Normalized();
        
        float3 up = owner->GetGlobalTransformation().Col3(1);
        float3 front = normalized.Cross(up);
        up = front.Cross(normalized);

        transform = float4x4(float4(right, 0.0), float4(up, 0.0), float4(front, 0.0), owner->GetGlobalTransformation().Col(3));
    }

    return transform;
}

void ComponentLine::OnSave(Config &config) const 
{
	config.AddUID("Texture", texture.GetUID());
}

void ComponentLine::OnLoad(Config *config) 
{
    texture = config->GetUID("Texture", 0);
}
