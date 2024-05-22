#include "Globals.h"

#include "FogPass.h"

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

FogPass::FogPass()
{
}

FogPass::~FogPass()
{

}

void FogPass::execute(Framebuffer *target, uint width, uint height)
{
    float dt = (float)timer.Read() / 1000.0f;
    timer.Start();
    frame = frame + dt;


    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Fog");

    resizeFrameBuffer(width, height);


    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();

    if (App->hints->GetIntValue(ModuleHints::FOG_TYPE) == FOG_TYPE_RAYMARCHING)
    {
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
        params.maxDistance = 1000.0f;
        params.noiseScale = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_NOISE_SCALE);
        params.noiseSpeed = App->hints->GetFloatValue(ModuleHints::RAYMARCHING_NOISE_SPEED);

        parametersUBO->InvalidateData();
        parametersUBO->SetData(0, sizeof(params), &params);


        useRayMarchingProgram();

        // Bindings
        result->BindImage(RAYMARCHING_FOG_DENSITY_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RGBA32F);
        const ResourceTexture* blueNoise = App->resources->GetDefaultBlueNoise();
        float2 tiling = float2(float(width)/float(blueNoise->GetMetadata().width), float(height)/float(blueNoise->GetMetadata().height));

        blueNoise->GetTexture()->Bind(RAYMARCHING_BLUENOISE_TEX_BINDING);
        rayMarchingProgram->BindUniform(RAYMARCHING_WIDHT_LOCATION, int(width));
        rayMarchingProgram->BindUniform(RAYMARCHING_HEIGHT_LOCATION, int(height));

        rayMarchingProgram->BindUniform(RAYMACHING_BLUENOISE_UV_TILING_LOCATION, tiling);
        App->level->GetLightManager()->Bind();
        parametersUBO->BindToPoint(RAYMARCHING_PARAMETERS_LOCATION);

        int numWorkGroupsX = (width + (RAYMARCHING_GROUP_SIZE - 1)) / RAYMARCHING_GROUP_SIZE;
        int numWorkGroupsY = (height + (RAYMARCHING_GROUP_SIZE - 1)) / RAYMARCHING_GROUP_SIZE;

        glDispatchCompute(numWorkGroupsX, numWorkGroupsY, 1);

        glPopDebugGroup();
    }
    else if(App->hints->GetIntValue(ModuleHints::FOG_TYPE) == FOG_TYPE_DISTANCE)
	{
		useDistanceProgram();
        
        struct FogData
        {
            float4 colour;
            float2 curve;
            float2 distRange;
            float  frame;
            uint   pad0;
        } fogData;

        if (!ubo)
        {
            ubo.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(fogData), nullptr));
        }
        float4 curve = App->hints->GetFloat4Value(ModuleHints::DIST_FOG_CURVE);

        fogData.colour = float4(App->hints->GetFloat3Value(ModuleHints::DIST_FOG_COLOUR), 0.0f);
        fogData.curve = float2(curve.y, curve.w);
        fogData.distRange = float2(App->hints->GetFloatValue(ModuleHints::DIST_FOG_MIN), App->hints->GetFloatValue(ModuleHints::DIST_FOG_MAX));
        fogData.frame = frame;

        ubo->InvalidateData();
        ubo->SetData(0, sizeof(fogData), &fogData);

        ubo->BindToPoint(DISTANCE_FOG_DATA);
	}
	else
	{
		useProgram();

		program->BindUniform(FOG_DENSITY_HEIGHT_FALLOFF_LOCATION, App->hints->GetFloatValue(ModuleHints::HEIGHT_FOG_DENSITY_FALLOFF));
		program->BindUniform(FOG_GLOGAL_DENSITY_LOCATION, App->hints->GetFloatValue(ModuleHints::HEIGHT_FOG_GLOBAL_DENSITY));
		program->BindUniform(FOG_COLOR, App->hints->GetFloat3Value(ModuleHints::HEIGHT_FOG_COLOR));
		program->BindUniform(FOG_SUN_COLOR, App->hints->GetFloat3Value(ModuleHints::HEIGHT_FOG_SUN_COLOR));
	}

	exportPass->getDepth()->Bind(GBUFFER_DEPTH_TEX_BINDING);
    App->renderer->GetCameraUBO()->Bind();

    frameBuffer->Bind();
    glViewport(0, 0, width, height);

	glDepthMask(GL_FALSE);

    if(!vao) vao = std::make_unique<VertexArray>();

    vao->Bind();

    glDrawArrays(GL_TRIANGLES, 0, 3);


    vao->Unbind();
	frameBuffer->Unbind();


    bool doBlur = App->hints->GetBoolValue(ModuleHints::DIST_FOG_BLUR);
    if (doBlur)
    {
        if(!kawase) kawase = std::make_unique<DualKawaseBlur>();
        //if (!kawase2) kawase2 = std::make_unique<DualKawaseBlur>();

        kawase->execute(result.get(), GL_RGBA32F, GL_RGBA, GL_FLOAT, width, height);
        //kawase2->execute(kawase->getResult(), GL_RGBA32F, GL_RGBA, GL_FLOAT, width, height);

        useApplyProgram();
        target->Bind();
        glViewport(0, 0, width, height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        vao->Bind();

        kawase->getResult()->Bind(0);
        result->Bind(1);
    }
    else
    {
        useApplyProgramNoBlur();
        target->Bind();
        glViewport(0, 0, width, height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        vao->Bind();

        result->Bind(1);
    }


    glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

    glPopDebugGroup();

}

void FogPass::useProgram()
{
	if(!program)
	{
		program.reset(generateProgram("Fog", "assets/shaders/fullscreenVS.glsl", "assets/shaders/fogFS.glsl", 0, 0));
	}

	if(program)
	{
		program->Use();
	}
}

void FogPass::useDistanceProgram()
{
	if(!distanceProg)
	{
        distanceProg.reset(generateProgram("Distance Fog", "assets/shaders/fullscreenVS.glsl", "assets/shaders/distanceFog.glsl", 0, 0));
	}

	if(distanceProg)
	{
		distanceProg->Use();
	}
}

void FogPass::useApplyProgramNoBlur()
{
	if(!applyProgNoBlur)
	{
        const char* defines[] = { "#define BLUR 0\n" };
		applyProgNoBlur.reset(generateProgram("Apply Fog No blur", "assets/shaders/fullscreenVS.glsl", "assets/shaders/applyFog.glsl", defines, 1));
	}

	if(applyProgNoBlur)
	{
		applyProgNoBlur->Use();
	}
}

void FogPass::useApplyProgram()
{
	if(!applyProg)
	{
        const char* defines[] = { "#define BLUR 1\n"};
		applyProg.reset(generateProgram("Apply Fog", "assets/shaders/fullscreenVS.glsl", "assets/shaders/applyFog.glsl", defines, 1));
	}

	if(applyProg)
	{
		applyProg->Use();
	}
}

void FogPass::useRayMarchingProgram()
{
	if(!rayMarchingProgram)
	{
        std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/fogRayMarching.glsl");

        if(shader->Compiled())
        {
            rayMarchingProgram = std::make_unique<Program>(shader.get());
        }
	}

	if(rayMarchingProgram)
	{
		rayMarchingProgram->Use();
	}
}

Program* FogPass::generateProgram(const char* name, const char *vertexPath, const char *fragmentPath, const char **defines, unsigned count)
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile(vertexPath, defines, count));

	bool ok = vertex->Compiled();

	if (ok)
	{
        fragment.reset(Shader::CreateFSFromFile(fragmentPath, defines, count));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		Program* prog = new Program(vertex.get(), fragment.get(), name);

		if(prog->Linked())
        {
            return prog;
        }

        delete prog;
	}

    return nullptr;
}

void FogPass::resizeFrameBuffer(uint width, uint height)
{
    if(width != fbWidth || height != fbHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        result = std::make_unique<Texture2D>(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr, false);
        rayMarchingResult = std::make_unique<Texture2D>(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr, false);
       
        frameBuffer->AttachColor(result.get(), 0, 0); 
        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth  = width;
        fbHeight = height;
    }

}
