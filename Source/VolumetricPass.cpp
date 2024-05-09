#include "Globals.h"

#include "VolumetricPass.h"

#include "GBufferExportPass.h"
#include "Application.h"
#include "ModuleHints.h"
#include "ModuleRenderer.h"
#include "ModuleResources.h"
#include "ResourceTexture.h"
#include "OpenGL.h"
#include "OGL.h"
#include "CameraUBO.h"
#include "GaussianBlur.h"
#include "DualKawaseBlur.h"
#include "ModuleLevelManager.h"
#include "LightManager.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

VolumetricPass::VolumetricPass()
{
}

VolumetricPass::~VolumetricPass()
{

}

void VolumetricPass::execute(Framebuffer *target, uint width, uint height)
{
    float dt = (float)timer.Read() / 1000.0f;
    timer.Start();
    frame = frame + dt;

    resizeFrameBuffer(width, height);

    GBufferExportPass *exportPass = App->renderer->GetGBufferExportPass();

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "RayMarching");

    struct Parameters
    {
        float4 ambientColour;
        float extinctionCoeff;
        float fogIntensity;
        float frame;
        float noiseScale;
        float noiseSpeed;
        float maxDistance;
        int pad0, pad1;
    } params;

    if (!parametersUBO)
    {
        parametersUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(params), nullptr));
    }

    params.ambientColour = float4(App->hints->GetFloat3Value(ModuleHints::RAYMARCHING_AMBIENT_COLOUR), 1.0f);
    params.extinctionCoeff = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_EXTINCTION_COEFF);
    params.fogIntensity = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_FOG_INTENSITY);
    params.frame = frame;
    params.maxDistance = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_MAX_DISTANCE);
    params.noiseScale = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_NOISE_SCALE);
    params.noiseSpeed = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_NOISE_SPEED);

    parametersUBO->InvalidateData();
    parametersUBO->SetData(0, sizeof(params), &params);

    useProgram();

    // Bindings
    result->BindImage(RAYMARCHING_FOG_DENSITY_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
    const ResourceTexture *blueNoise = App->resources->GetDefaultBlueNoise();
    float2 tiling = float2(float(width) / float(blueNoise->GetMetadata().width), float(height) / float(blueNoise->GetMetadata().height));

    blueNoise->GetTexture()->Bind(RAYMARCHING_BLUENOISE_TEX_BINDING);
    program->BindUniform(RAYMARCHING_WIDHT_LOCATION, int(width));
    program->BindUniform(RAYMARCHING_HEIGHT_LOCATION, int(height));

    program->BindUniform(RAYMACHING_BLUENOISE_UV_TILING_LOCATION, tiling);
    App->level->GetLightManager()->Bind();
    parametersUBO->BindToPoint(RAYMARCHING_PARAMETERS_LOCATION);

    int numWorkGroupsX = (width + (RAYMARCHING_GROUP_SIZE - 1)) / RAYMARCHING_GROUP_SIZE;
    int numWorkGroupsY = (height + (RAYMARCHING_GROUP_SIZE - 1)) / RAYMARCHING_GROUP_SIZE;

    glDispatchCompute(numWorkGroupsX, numWorkGroupsY, 1);

    glPopDebugGroup();

    exportPass->getDepth()->Bind(GBUFFER_DEPTH_TEX_BINDING);
    App->renderer->GetCameraUBO()->Bind();

    frameBuffer->Bind();
    glViewport(0, 0, width, height);

    glDepthMask(GL_FALSE);

    if (!vao)
        vao = std::make_unique<VertexArray>();

    vao->Bind();

    glDrawArrays(GL_TRIANGLES, 0, 3);

    vao->Unbind();
    frameBuffer->Unbind();

    bool doBlur = App->hints->GetBoolValue(ModuleHints::RAYMARCHING_BLUR);
    if (doBlur)
    {
        if (!kawase) kawase = std::make_unique<DualKawaseBlur>();

        kawase->execute(result.get(), GL_RGBA32F, GL_RGBA, GL_FLOAT, width, height);
    }

    useApplyProgram(doBlur);

    if (doBlur)
    {
        kawase->getResult()->Bind(0);
    }

    result->Bind(1);
    target->Bind();
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    vao->Bind();

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void VolumetricPass::useProgram()
{
	if(!program)
	{
        std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/fogRayMarching.glsl");

        if(shader->Compiled())
        {
            program = std::make_unique<Program>(shader.get());
        }
	}

	if(program)
	{
        program->Use();
	}
}

void VolumetricPass::useConeProgram()
{
    if (!coneProgram)
    {
        std::unique_ptr<Shader> vertex, fragment;

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/deferredSpotProxyVS.glsl", 0, 0));
        fragment.reset(Shader::CreateFSFromFile("assets/shaders/rayMarchingSpotProxyFS.glsl", 0, 0));

        if (vertex->Compiled() && fragment->Compiled())
        {
            coneProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "DeferredConeProxy");
            if (!coneProgram->Linked())
            {
                coneProgram.release();
            }
        }
    }

    if (coneProgram)
    {
        coneProgram->Use();
    }
}

void VolumetricPass::useApplyProgram(bool doBlur)
{
    if (doBlur)
    {
        if (!applyProgram) generateApplyProgram(true);
        if (applyProgram) applyProgram->Use();
    }
    else
    {
        if (!applyProgramNoBlur) generateApplyProgram(false);
        if (applyProgramNoBlur) applyProgramNoBlur->Use();
    }
}

void VolumetricPass::generateApplyProgram(bool doBlur)
{
    std::unique_ptr<Shader> vertex, fragment;

    const char* defines[] = { doBlur ?  "#define BLUR 1\n" : "#define BLUR 0\n"  } ;

    vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", defines, 1));

    bool ok = vertex->Compiled();

    if (ok)
    {
        fragment.reset(Shader::CreateFSFromFile("assets/shaders/applyFog.glsl", defines, 1));

        ok = fragment->Compiled();
    }

    if (ok)
    {
        Program* prog = new Program(vertex.get(), fragment.get(), "Apply Fog");

        if (prog->Linked())
        {
            if (doBlur)
            {
                applyProgram.reset(prog);
            }
            else
            {
                applyProgramNoBlur.reset(prog);
            }
        }
        else
        {
            delete prog;
        }
    }
}

void VolumetricPass::resizeFrameBuffer(uint width, uint height)
{
    if(width != fbWidth || height != fbHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        result = std::make_unique<Texture2D>(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr, false);
       
        frameBuffer->AttachColor(result.get(), 0, 0); 
        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth  = width;
        fbHeight = height;
    }
}


