#include "Globals.h"

#include "DeferredResolvePass.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "Skybox.h"
#include "GBufferExportPass.h"
#include "ScreenSpaceAO.h"
#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

DeferredResolvePass::DeferredResolvePass()
{
}

void DeferredResolvePass::execute(Framebuffer *target, uint width, uint height)
{
    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();
	ScreenSpaceAO* ssao = App->renderer->GetScreenSpaceAO();

    useProgram();

    target->Bind();
    glViewport(0, 0, width, height);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    exportPass->getAlbedo()->Bind(GBUFFER_ALBEDO_TEX_BINDING);
    exportPass->getSpecular()->Bind(GBUFFER_SPECULAR_TEX_BINDING);
    exportPass->getEmissive()->Bind(GBUFFER_EMISSIVE_TEX_BINDING);
    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    exportPass->getNormal()->Bind(GBUFFER_NORMAL_TEX_BINDING);

	ssao->bindResult();

	App->level->GetSkyBox()->BindIBL();

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    target->Unbind();
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


	vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/deferred.glsl", 0, 0));

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