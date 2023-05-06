#include "Globals.h"

#include "ForwardPass.h"
#include "Application.h"
#include "BatchManager.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "ModuleHints.h"
#include "IBLData.h"
#include "ScreenSpaceAO.h"
#include "ShadowmapPass.h"
#include "CascadeShadowPass.h"
#include "CameraUBO.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"


ForwardPass::ForwardPass()
{
}

ForwardPass::~ForwardPass()
{
}

void ForwardPass::executeOpaque(const RenderList &objects, Framebuffer *target, uint width, uint height)
{
    ShadowmapPass* shadowMap = App->renderer->GetShadowmapPass();

    UseProgram();

    program->BindUniform(SHADOW_VIEWPROJ_LOCATION, shadowMap->getFrustum().ViewProjMatrix());
    program->BindUniform(SHADOW_BIAS_LOCATION, App->hints->GetFloatValue(ModuleHints::SHADOW_BIAS)); 
    bindShadows();

    App->renderer->GetCameraUBO()->Bind();
    App->level->GetSkyBox()->Bind();
    App->renderer->GetScreenSpaceAO()->getResult()->Bind(SSAO_TEX_BINDING);

    if (target)
    {
        target->Bind();
        glViewport(0, 0, width, height);
    }

    App->renderer->GetBatchManager()->DoRender(objects.GetOpaques(), BR_FLAG_KEEP_ORDER);

    if (target)
    {
        target->Unbind();
    }
}

void ForwardPass::executeTransparent(const RenderList &objects, Framebuffer *target, uint width, uint height)
{
    UseProgram();

    App->renderer->GetCameraUBO()->Bind();
    App->level->GetSkyBox()->Bind();
    App->renderer->GetScreenSpaceAO()->getResult()->Bind(SSAO_TEX_BINDING);

    if (target)
    {
        target->Bind();
        glViewport(0, 0, width, height);
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    App->renderer->GetBatchManager()->DoRender(objects.GetTransparents(), BR_FLAG_KEEP_ORDER);

    glDisable(GL_BLEND);

    if (target)
    {
        target->Unbind();
    }
}

void ForwardPass::UseProgram()
{
	if(!program)
	{
		GenerateProgram();
	}

	if(program)
	{
		program->Use();
	}
}

bool ForwardPass::GenerateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{
		fragment.reset(Shader::CreateFSFromFile("assets/shaders/defFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "default");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}

void ForwardPass::bindShadows()
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
