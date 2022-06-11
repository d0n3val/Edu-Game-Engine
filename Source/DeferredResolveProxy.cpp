#include "Globals.h"

#include "DeferredResolveProxy.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "Skybox.h"
#include "GBufferExportPass.h"
#include "ScreenSpaceAO.h"
#include "LightManager.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

void DeferredResolveProxy::execute(Framebuffer *target, uint width, uint height)
{
    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();
	ScreenSpaceAO* ssao = App->renderer->GetScreenSpaceAO();

    createDrawIdVBO();
    useProgram();

	// Additive Blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);

	App->renderer->GetCameraUBO()->BindToPoint(CAMERA_UBO_BINDING);
	target->Bind();
    glViewport(0, 0, width, height);

    exportPass->getAlbedo()->Bind(GBUFFER_ALBEDO_TEX_BINDING);
    exportPass->getSpecular()->Bind(GBUFFER_SPECULAR_TEX_BINDING);
    exportPass->getEmissive()->Bind(GBUFFER_EMISSIVE_TEX_BINDING);
    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    exportPass->getNormal()->Bind(GBUFFER_NORMAL_TEX_BINDING);
    
	ssao->getResult()->Bind(SSAO_TEX_BINDING);
	App->level->GetSkyBox()->BindIBL();

	const ResourceMesh* sphere = App->resources->GetDefaultSphere();
    glBindVertexArray(sphere->GetVAO());
	glDrawElementsInstanced(GL_TRIANGLES, sphere->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetNumPointLights());

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

	target->Unbind();
}

void DeferredResolveProxy::useProgram()
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

bool DeferredResolveProxy::generateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/deferredProxyVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/deferredProxyFS.glsl", 0, 0));

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

void DeferredResolveProxy::createDrawIdVBO()
{
	LightManager* lightManager = App->level->GetLightManager();
	uint pointCount = lightManager->GetNumPointLights();

	if(drawIdCount < pointCount)
	{
		drawIdVBO.reset(Buffer::CreateVBO(GL_STATIC_DRAW, uint(pointCount * sizeof(int)), nullptr));
		int *drawIds = (int *)drawIdVBO->Map(GL_WRITE_ONLY);

		for (uint i = 0; i < pointCount; ++i)
		{
			drawIds[i] = i;
		}
		drawIdVBO->Unmap();

		// add drawId to vao
		glBindVertexArray(App->resources->GetDefaultSphere()->GetVAO());
		drawIdVBO->Bind();
		glEnableVertexAttribArray(DRAW_ID_ATTRIB_LOCATION);
		glVertexAttribIPointer(DRAW_ID_ATTRIB_LOCATION, 1, GL_INT, sizeof(int), (void *)0);
		glVertexAttribDivisor(DRAW_ID_ATTRIB_LOCATION, 1);
		glBindVertexArray(0);

		drawIdCount = pointCount;
	}
}