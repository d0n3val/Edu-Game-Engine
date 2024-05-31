#include "Globals.h"

#include "DeferredResolvePass.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "modulehints.h"
#include "CascadeShadowPass.h"
#include "ShadowmapPass.h"
#include "PlanarReflectionPass.h"
#include "IBLData.h"
#include "GBufferExportPass.h"
#include "TileCullingPass.h"
#include "ScreenSpaceAO.h"
#include "LightManager.h"
#include "OGL.h"
#include "OpenGL.h"
#include "CameraUBO.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"


DeferredResolvePass::DeferredResolvePass()
{
}

void DeferredResolvePass::execute(Framebuffer *target, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "DeferredResolve");
    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();
	ScreenSpaceAO* ssao = App->renderer->GetScreenSpaceAO();
    TileCullingPass* tileCulling = App->renderer->GetTileCullingPass();

    useProgram();

    bindShadows();
    program->BindUniform(DEFERRED_WIDTH_LOCATION, int(width));
    program->BindUniform(DEFERRED_HEIGHT_LOCATION, int(height));
    App->renderer->GetCameraUBO()->Bind();
    App->level->GetLightManager()->Bind();
    App->renderer->GetPlanarPass()->Bind();
    target->Bind();
    glViewport(0, 0, width, height);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    exportPass->getAlbedo()->Bind(GBUFFER_ALBEDO_TEX_BINDING);
    exportPass->getSpecular()->Bind(GBUFFER_SPECULAR_TEX_BINDING);
    exportPass->getEmissive()->Bind(GBUFFER_EMISSIVE_TEX_BINDING);
    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    exportPass->getNormal()->Bind(GBUFFER_NORMAL_TEX_BINDING);
    exportPass->getDepth()->Bind(GBUFFER_DEPTH_TEX_BINDING);
    tileCulling->getPointLightList()->Bind(POINT_LIGHT_LIST_BINDING);
    tileCulling->getSpotLightList()->Bind(SPOT_LIGHT_LIST_BINDING);
    
	ssao->getResult()->Bind(SSAO_TEX_BINDING);

	App->level->GetSkyBox()->Bind();

    App->renderer->GetCameraUBO()->Bind();

    if(!vao) vao = std::make_unique<VertexArray>();

    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    vao->Unbind();
    target->Unbind();
    glPopDebugGroup();
}

void DeferredResolvePass::useProgram()
{
	if(!program)
	{
		generateProgram();
	}

	if(program)
	{
		program->Use();
	}
}

bool DeferredResolvePass::generateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

    const char* defines[] = { "#define GEN_WORLD_POSITION\n" };
	vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", defines, 1));

	bool ok = vertex->Compiled();

	if (ok)
	{
        fragment.reset(Shader::CreateFSFromFile("assets/shaders/deferred.glsl", defines, 1));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "DeferredResolve");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}

void DeferredResolvePass::bindShadows()
{
    if (App->hints->GetBoolValue(ModuleHints::ENABLE_CASCADE_SHADOW))
    {
        CascadeShadowPass* shadowMap = App->renderer->GetCascadeShadowPass();

        for (uint i = 0; i < CascadeShadowPass::CASCADE_COUNT; ++i)
        {
            program->BindUniform(SHADOW_VIEWPROJ_LOCATION + i, shadowMap->getFrustum(i).ViewProjMatrix());
            shadowMap->getDepthTex(i)->Bind(SHADOWMAP_TEX_BINDING + i);
        }

        program->BindUniform(SHADOW_BIAS_LOCATION, App->hints->GetFloatValue(ModuleHints::SHADOW_BIAS));
        program->BindUniform(SHADOW_SLOPEBIAS_LOCATION, App->hints->GetFloatValue(ModuleHints::SHADOW_SLOPEBIAS));
    }
    else
    {
        ShadowmapPass* shadowMap = App->renderer->GetShadowmapPass();
        program->BindUniform(SHADOW_VIEWPROJ_LOCATION, shadowMap->getFrustum().ViewProjMatrix());
        program->BindUniform(SHADOW_BIAS_LOCATION, App->hints->GetFloatValue(ModuleHints::SHADOW_BIAS));
        program->BindUniform(SHADOW_SLOPEBIAS_LOCATION, App->hints->GetFloatValue(ModuleHints::SHADOW_SLOPEBIAS));
        shadowMap->getDepthTex()->Bind(SHADOWMAP_TEX_BINDING);
        shadowMap->getVarianceTex()->Bind(VARIANCE_TEX_BINDING);
    }
}

