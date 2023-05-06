#include "Globals.h"

#include "DeferredResolveProxy.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "IBLData.h"
#include "GBufferExportPass.h"
#include "ScreenSpaceAO.h"
#include "LightManager.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "OGL.h"
#include "OpenGL.h"
#include "CameraUBO.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

//#define DEBUG_RENDER

void DeferredResolveProxy::execute(Framebuffer *target, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "DeferredResolveProxyLights");
    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();
	ScreenSpaceAO* ssao = App->renderer->GetScreenSpaceAO();
	const ResourceMesh* sphere = App->resources->GetDefaultSphere();


    createDrawIdVBO();
	target->Bind();

    useProgram();

    App->renderer->GetCameraUBO()->Bind();

	// Additive Blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);

	App->renderer->GetCameraUBO()->Bind();
    glViewport(0, 0, width, height);

    exportPass->getAlbedo()->Bind(GBUFFER_ALBEDO_TEX_BINDING);
    exportPass->getSpecular()->Bind(GBUFFER_SPECULAR_TEX_BINDING);
    exportPass->getEmissive()->Bind(GBUFFER_EMISSIVE_TEX_BINDING);
    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    exportPass->getNormal()->Bind(GBUFFER_NORMAL_TEX_BINDING);
    
	ssao->getResult()->Bind(SSAO_TEX_BINDING);
	App->level->GetSkyBox()->Bind();

    glBindVertexArray(sphere->GetVAO());
	glDrawElementsInstanced(GL_TRIANGLES, sphere->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetEnabledPointLights());

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

#ifdef DEBUG_RENDER
	glEnable(GL_BLEND);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	useDebug();
    glPolygonMode(GL_FRONT, GL_LINE);
    glBindVertexArray(sphere->GetVAO());
	glDrawElementsInstanced(GL_TRIANGLES, sphere->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetEnabledPointLights()());
    glPolygonMode(GL_FRONT, GL_FILL);
	glDisable(GL_BLEND);
#endif 

	target->Unbind();

    glPopDebugGroup();
}

void DeferredResolveProxy::useDebug()
{
	if(!debugProgram)
	{
		generateDebug();
	}

	if(debugProgram)
	{
        debugProgram->Use();
	}
}

bool DeferredResolveProxy::generateDebug()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/deferredProxyVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/showProxyFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
        debugProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "ShowProxy");

		ok = debugProgram->Linked();
	}

	if (!ok)
	{
        debugProgram.release();
	}

	return ok;

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
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "DeferredProxy");

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
	uint pointCount = lightManager->GetEnabledPointLights();

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