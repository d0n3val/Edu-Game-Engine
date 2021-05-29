#include "Globals.h"
#include "ScreenSpaceAO.h"
#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleHints.h"
#include "DefaultShader.h"
#include "DepthPrePass.h"
#include "GaussianBlur.h"

#include "OGL.h"
#include "OpenGL.h"

#include <random>

#define KERNEL_UBO_TARGET 5
#define KERNEL_SIZE 128
#define RANDOM_SIZE 16

ScreenSpaceAO::ScreenSpaceAO()
{
    std::unique_ptr<Shader> fullScreenVS(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl"));
    std::unique_ptr<Shader> aoFS(Shader::CreateFSFromFile("assets/shaders/ssaoFS.glsl"));

    if(fullScreenVS->Compiled() && aoFS->Compiled())
    {
        program = std::make_unique<Program>(fullScreenVS.get(), aoFS.get(), "SSAO program");
    }

    assert(program->Linked());

    blur = std::make_unique<GaussianBlur>();
}

ScreenSpaceAO::~ScreenSpaceAO()
{
}

void ScreenSpaceAO::Execute()
{
    GenerateKernelUBO();

    DepthPrepass* depthPrePass = App->renderer->GetDepthPrepass();

    uint width = depthPrePass->getWidth();
    uint height = depthPrePass->getHeight();
    ResizeFrameBuffer(width, height);

    frameBuffer->Bind();
    
    glViewport(0, 0, width, height);

    program->Use();
    program->BindTextureFromName("positions", 0, depthPrePass->getPositionTexture());
    program->BindTextureFromName("normals", 1, depthPrePass->getNormalTexture());
    program->BindUniformFromName("screenSize", float2(float(width), float(height)));
    program->BindUniformFromName("radius", std::get<float>(App->hints->GetDHint(std::string("SSAO radius"), 2.0f))); 
    program->BindUniformFromName("bias", -std::get<float>(App->hints->GetDHint(std::string("SSAO bias"), 0.1f))); 
    program->BindUniformBlock("Camera", DefaultShader::CAMERA_UBO_TARGET);
    program->BindUniformBlock("Kernel", KERNEL_UBO_TARGET);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    if(std::get<bool>(App->hints->GetDHint(std::string("SSAO blur"), true)))
    {
        blur->Execute(result.get(), blurred.get(), width, height);
    }
}

void ScreenSpaceAO::ResizeFrameBuffer(uint width, uint height)
{
    if(width != fbWidth || height != fbHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        result  = std::make_unique<Texture2D>(width, height, GL_RGB8, GL_RED, GL_FLOAT, nullptr, false);
        blurred = std::make_unique<Texture2D>(width, height, GL_RGB8, GL_RED, GL_FLOAT, nullptr, false);
       
        frameBuffer->AttachColor(result.get(), 0, 0); 
        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth  = width;
        fbHeight = height;
    }
}

void ScreenSpaceAO::GenerateKernelUBO()
{
    if(!kernel)
    {
        struct Kernel
        {
            float4 dirs[KERNEL_SIZE]; 
            float4 rots[RANDOM_SIZE];
        } kernelData;

        std::uniform_real_distribution<float> randoms(0.0f, 1.0f);
        std::default_random_engine generator;

        //  Generate kernel
        for (uint i = 0; i < KERNEL_SIZE; ++i)
        {
            float3 dir;
            dir.x = randoms(generator)*2.0f-1.0f;
            dir.y = randoms(generator)*2.0f-1.0f;
            dir.z = randoms(generator);

            dir.Normalize();

            dir *= randoms(generator); // random size

            // try to force closer samples
            float scale = float(i)/float(KERNEL_SIZE);

            // lerp
            scale = 0.1f+(scale*scale)*(1.0f-0.1f);

            dir *= scale;

            kernelData.dirs[i] = float4(dir, 0.0f);
        }

        //  Generate dir noise
        for(uint i=0; i< RANDOM_SIZE; ++i)
        {
            float3 dir;
            dir.x = randoms(generator)*2.0f-1.0f;
            dir.y = randoms(generator)*2.0f-1.0f;
            dir.z = 0.0f;

            dir.Normalize();

            kernelData.rots[i] = float4(dir, 0.0f);
        }


        kernel = std::make_unique<Buffer>(GL_UNIFORM_BUFFER, GL_STATIC_DRAW, sizeof(kernelData), &kernelData);
        kernel->BindToTargetIdx(KERNEL_UBO_TARGET);
    }
}

const Texture2D* ScreenSpaceAO::getResult() const
{
    //if(std::get<bool>(App->hints->GetDHint(std::string("SSAO blur"), true)))
    {
        return blurred.get();
    }

    //return result.get();
}