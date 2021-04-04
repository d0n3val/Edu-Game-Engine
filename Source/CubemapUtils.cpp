#include "Globals.h"

#include "CubemapUtils.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "ResourceTexture.h"

#include "OpenGL.h"

#include "Leaks.h"

const float CubemapUtils::skybox_vertices[6*6*3] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
};

const float3 CubemapUtils::front[6] = { 
    float3(1.0f, 0.0f, 0.0f), float3(-1.0f, 0.0f, 0.0f), 
    float3(0.0f, 1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f), 
    float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f) 
};

float3 CubemapUtils::up[6] = {  
    float3(0.0f, -1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f),
    float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, -1.0f),
    float3(0.0f, -1.0f, 0.0f), float3(0.0f, -1.0f, 0.0f) 
};


CubemapUtils::CubemapUtils()
{
}

void CubemapUtils::RenderSkybox(TextureCube* cubeMap, const float4x4& proj, const float4x4& view)
{
    if(!vao) InitBuffers();

    App->programs->UseProgram("skybox", 0);

    cubeMap->Bind(0, App->programs->GetUniformLocation("skybox"));

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

    glDepthFunc(GL_ALWAYS);
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6*6);
    glDepthFunc(GL_LESS);
}

TextureCube* CubemapUtils::DiffuseIBL(UID cubemap)
{
    return nullptr;
}

TextureCube* CubemapUtils::ConvertToCubemap(Texture2D* texture, uint width, uint height)
{
    if(!vao) InitBuffers();

	Frustum frustum;
	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.pos  = float3::zero;

	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance  = 100.0f;
	frustum.verticalFov       = math::pi/2.0f; 
	frustum.horizontalFov     = math::pi/2.0f; 

    Framebuffer frameBuffer;
    TextureCube* cubeMap = new TextureCube();

    // initialize each cubemap plane
    for(uint i=0; i< 6; ++i)
    {
        cubeMap->SetData(i, 0, width, height, GL_RGB16F, GL_RGB, GL_FLOAT, nullptr);
    }

    App->programs->UseProgram("equirectangular");

    texture->Bind(0, App->programs->GetUniformLocation("skybox"));

    // Render each cube plane
    for(uint i=0; i<6; ++i)
    {
        frameBuffer.AttachColor(cubeMap, i, 0, 0);
        frameBuffer.Clear(width, height);

        frustum.front = front[i];
        frustum.up = up[i];

        float4x4 proj = frustum.ProjectionMatrix();
        float4x4 view = frustum.ViewMatrix();

        glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
        glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

        glDepthFunc(GL_ALWAYS);
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6*6);
        glDepthFunc(GL_LESS);
    }

    return cubeMap;
}

void CubemapUtils::InitBuffers()
{
    VertexAttrib attribs[] = { { 0, 3, GL_FLOAT, false, 0, 0 } };

    vbo = std::unique_ptr<Buffer>(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(skybox_vertices), skybox_vertices));
    vao = std::make_unique<VertexArray>(vbo.get(), nullptr, attribs, sizeof(attribs) / sizeof(VertexAttrib));
}
