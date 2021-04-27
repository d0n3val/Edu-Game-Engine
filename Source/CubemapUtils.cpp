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
    if(!vao) Init();

    skybox->Use();
    skybox->BindTextureFromName("skybox", 0, cubeMap);
    skybox->BindUniformFromName("proj", proj);
    skybox->BindUniformFromName("view", view);

    glDepthFunc(GL_ALWAYS);
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6*6);
    glDepthFunc(GL_LESS);
}

TextureCube* CubemapUtils::DiffuseIBL(TextureCube* texture, uint width, uint height)
{
    if (!vao) Init();

    return RenderCube(diffuseIBL.get(), texture, width, height);
}

TextureCube* CubemapUtils::ConvertToCubemap(Texture2D* texture, uint width, uint height)
{
    if (!vao) Init();

    return RenderCube(equirectangular.get(), texture, width, height);
}

TextureCube* CubemapUtils::RenderCube(Program* program, Texture* texture, uint width, uint height)
{
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
        cubeMap->SetData(i, 0, width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr);
    }

    program->Use();
    program->BindTextureFromName("skybox", 0, texture);

    // Render each cube plane
    for(uint i=0; i<6; ++i)
    {
        frameBuffer.ClearAttachments();
        frameBuffer.AttachColor(cubeMap, i, 0, 0);
        frameBuffer.Bind();
        glViewport(0, 0, width, height);

        frustum.front = front[i];
        frustum.up = up[i];

        float4x4 proj = frustum.ProjectionMatrix();
        float4x4 view = frustum.ViewMatrix();

        program->BindUniformFromName("proj", proj);
        program->BindUniformFromName("view", view);

        glDepthFunc(GL_ALWAYS);
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6*6);
        glDepthFunc(GL_LESS);
    }

    return cubeMap;
}

void CubemapUtils::Init()
{
    VertexAttrib attribs[] = { { 0, 3, GL_FLOAT, false, 0, 0 } };

    vbo = std::unique_ptr<Buffer>(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(skybox_vertices), skybox_vertices));
    vao = std::make_unique<VertexArray>(vbo.get(), nullptr, attribs, sizeof(attribs) / sizeof(VertexAttrib));

    std::unique_ptr<Shader> vertex(Shader::CreateVSFromFile("Assets/Shaders/skybox.vs", nullptr, 0));
    std::unique_ptr<Shader> skyboxFS(Shader::CreateFSFromFile("Assets/Shaders/skybox.fs", nullptr, 0));
    std::unique_ptr<Shader> equirectangularFS(Shader::CreateFSFromFile("Assets/Shaders/equirectangular.fs", nullptr, 0));
    std::unique_ptr<Shader> diffuseFS(Shader::CreateFSFromFile("Assets/Shaders/diffuseIBL.glsl", nullptr, 0));

    if(vertex->Compiled() && skyboxFS->Compiled()) skybox.reset(new Program(vertex.get(), skyboxFS.get(), "Skybox program"));
    if(vertex->Compiled() && equirectangularFS->Compiled()) equirectangular.reset(new Program(vertex.get(), equirectangularFS.get(), "Equirectangular program" ));
    if(vertex->Compiled() && diffuseFS->Compiled()) diffuseIBL.reset(new Program(vertex.get(), diffuseFS.get(), "Diffuse IBL program" ));
}
