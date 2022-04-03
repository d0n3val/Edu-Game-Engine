#include "Globals.h"

#include "CubemapUtils.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "ResourceTexture.h"

#include "OGL.h"
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

    //glDepthFunc(GL_ALWAYS);
    //glDepthMask(GL_TRUE);
    //glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6*6);
    //glDepthFunc(GL_LESS);
}

void CubemapUtils::RenderSkyboxLod(TextureCube* cubeMap, const float4x4& proj, const float4x4& view, float lod)
{
    if(!vao) Init();

    skyboxLod->Use();
    skyboxLod->BindTextureFromName("skybox", 0, cubeMap);
    skyboxLod->BindUniformFromName("proj", proj);
    skyboxLod->BindUniformFromName("view", view);
    skyboxLod->BindUniformFromName("lod", lod);

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

TextureCube* CubemapUtils::PrefilteredSpecular(TextureCube* texture, uint width, uint height, uint prefilteredLevels)
{
    if (!vao) Init();

    Frustum frustum;
    frustum.type = FrustumType::PerspectiveFrustum;

    frustum.pos = float3::zero;

    frustum.nearPlaneDistance = 0.1f;
    frustum.farPlaneDistance  = 100.0f;
    frustum.verticalFov       = math::pi / 2.0f;
    frustum.horizontalFov     = math::pi / 2.0f;

    Framebuffer frameBuffer;
    TextureCube* cubeMap = new TextureCube();

    // initialize each cubemap plane
    for (uint i = 0; i < 6; ++i)
    {
        cubeMap->SetData(i, 0, width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr);
    }

    cubeMap->SetMinMaxFiler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
    cubeMap->GenerateMipmaps(0, prefilteredLevels);

    for (uint roughness = 0; roughness < prefilteredLevels; ++roughness)
    {
        assert(width > 0);
        assert(height > 0);

        prefilteredIBL->Use();
        prefilteredIBL->BindTextureFromName("skybox", 0, texture);

        // Render each cube plane
        for (uint i = 0; i < 6; ++i)
        {
            frameBuffer.ClearAttachments();
            frameBuffer.AttachColor(cubeMap, i, 0, roughness);
            frameBuffer.Bind();
            glViewport(0, 0, width, height);

            frustum.front = front[i];
            frustum.up = up[i];

            float4x4 proj = frustum.ProjectionMatrix();
            float4x4 view = frustum.ViewMatrix();

            prefilteredIBL->BindUniformFromName("proj", proj);
            prefilteredIBL->BindUniformFromName("view", view);
            prefilteredIBL->BindUniformFromName("roughness", float(roughness)/float(prefilteredLevels));

            glDepthFunc(GL_ALWAYS);
            vao->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
            glDepthFunc(GL_LESS);
        }

        width = width >> 1;
        height = height >> 1;
    }

    return cubeMap;
}

Texture2D* CubemapUtils::EnvironmentBRDF(uint width, uint height)
{
    Texture2D* texture = new Texture2D(width, height, GL_RG32F, GL_RG, GL_FLOAT, nullptr, false);
    texture->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    Framebuffer frameBuffer;

    frameBuffer.AttachColor(texture);
    frameBuffer.Bind();
    glViewport(0, 0, width, height);

    //frameBuffer.Clear(width, height);

    environmentBRDF->Use();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    return texture;
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
    vao = std::make_unique<VertexArray>(vbo.get(), nullptr, attribs, uint(sizeof(attribs) / sizeof(VertexAttrib)));

    const char* diffuseIBLMacros[] = { "#define DIFFUSE_IBL\n" };
    const char* prefilteredIBLMacros[] = { "#define PREFILTERED_IBL\n" };
    const char* skyboxLodMacros[] = { "#define USE_LOD\n" };

    std::unique_ptr<Shader> vertex(Shader::CreateVSFromFile("Assets/Shaders/skybox.vs", nullptr, 0));
    std::unique_ptr<Shader> fullScreen(Shader::CreateVSFromFile("Assets/Shaders/fullScreenVS.glsl", nullptr, 0));
    std::unique_ptr<Shader> skyboxFS(Shader::CreateFSFromFile("Assets/Shaders/skybox.fs", nullptr, 0));
    std::unique_ptr<Shader> skyboxLodFS(Shader::CreateFSFromFile("Assets/Shaders/skybox.fs", &skyboxLodMacros[0], 1));
    std::unique_ptr<Shader> equirectangularFS(Shader::CreateFSFromFile("Assets/Shaders/equirectangular.fs", nullptr, 0));
    std::unique_ptr<Shader> diffuseFS(Shader::CreateFSFromFile("Assets/Shaders/cubemapIBL.glsl", &diffuseIBLMacros[0], 1));
    std::unique_ptr<Shader> prefilteredFS(Shader::CreateFSFromFile("Assets/Shaders/cubemapIBL.glsl", &prefilteredIBLMacros[0], 1));
    std::unique_ptr<Shader> brdfFS(Shader::CreateFSFromFile("Assets/Shaders/cubemapIBL.glsl", nullptr, 0));

    if(vertex->Compiled() && skyboxFS->Compiled()) skybox.reset(new Program(vertex.get(), skyboxFS.get(), "Skybox program"));
    if(vertex->Compiled() && skyboxLodFS->Compiled()) skyboxLod.reset(new Program(vertex.get(), skyboxLodFS.get(), "Skybox LOD program"));
    if(vertex->Compiled() && equirectangularFS->Compiled()) equirectangular.reset(new Program(vertex.get(), equirectangularFS.get(), "Equirectangular program" ));
    if(vertex->Compiled() && diffuseFS->Compiled()) diffuseIBL.reset(new Program(vertex.get(), diffuseFS.get(), "Diffuse IBL program" ));
    if(vertex->Compiled() && prefilteredFS->Compiled()) prefilteredIBL.reset(new Program(vertex.get(), prefilteredFS.get(), "Prefiltered IBL program"));
    if(fullScreen->Compiled() && brdfFS->Compiled()) environmentBRDF.reset(new Program(fullScreen.get(), brdfFS.get(), "Environment BRDF program"));
}
