#include "Globals.h"

#include "DeferredResolveProxy.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "SpotShadowMapPass.h"
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

    target->Bind();
    glViewport(0, 0, width, height);

    // Additive Blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);

    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();
	ScreenSpaceAO* ssao = App->renderer->GetScreenSpaceAO();

    // spheres
    useSphereProgram();
    App->renderer->GetCameraUBO()->Bind();
    exportPass->getAlbedo()->Bind(GBUFFER_ALBEDO_TEX_BINDING);
    exportPass->getSpecular()->Bind(GBUFFER_SPECULAR_TEX_BINDING);
    exportPass->getEmissive()->Bind(GBUFFER_EMISSIVE_TEX_BINDING);
    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    exportPass->getNormal()->Bind(GBUFFER_NORMAL_TEX_BINDING);
    
	ssao->getResult()->Bind(SSAO_TEX_BINDING);
	App->level->GetSkyBox()->Bind();

    glBindVertexArray(App->resources->GetDefaultSphere()->GetVAO());
	glDrawElementsInstanced(GL_TRIANGLES, App->resources->GetDefaultSphere()->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetEnabledPointLights());

    // cones 
    useConeProgram();
    App->renderer->GetCameraUBO()->Bind();
    exportPass->getAlbedo()->Bind(GBUFFER_ALBEDO_TEX_BINDING);
    exportPass->getSpecular()->Bind(GBUFFER_SPECULAR_TEX_BINDING);
    exportPass->getEmissive()->Bind(GBUFFER_EMISSIVE_TEX_BINDING);
    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    exportPass->getNormal()->Bind(GBUFFER_NORMAL_TEX_BINDING);

    ssao->getResult()->Bind(SSAO_TEX_BINDING);
    App->level->GetSkyBox()->Bind();

    glBindVertexArray(App->resources->GetDefaultCone()->GetVAO());
    glDrawElementsInstanced(GL_TRIANGLES, App->resources->GetDefaultCone()->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetEnabledSpotLights());

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

#ifdef DEBUG_RENDER
	glEnable(GL_BLEND);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//useDebugSphere();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //  glBindVertexArray(App->resources->GetDefaultSphere()->GetVAO());
//    glDrawElementsInstanced(GL_TRIANGLES, App->resources->GetDefaultSphere()->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetEnabledPointLights());

    useDebugCone();
    glBindVertexArray(App->resources->GetDefaultCone()->GetVAO());
    glDrawElementsInstanced(GL_TRIANGLES, App->resources->GetDefaultCone()->GetNumIndices(), GL_UNSIGNED_INT, nullptr, App->level->GetLightManager()->GetEnabledSpotLights());
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glDisable(GL_BLEND);
#endif 

	target->Unbind();

    glPopDebugGroup();
}

void DeferredResolveProxy::useDebugSphere()
{
	if(!debugSphereProgram)
	{
		generateDebugSphere();
	}

	if(debugSphereProgram)
	{
        debugSphereProgram->Use();
	}
}

bool DeferredResolveProxy::generateDebugSphere()
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
        debugSphereProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "ShowProxy");

		ok = debugSphereProgram->Linked();
	}

	if (!ok)
	{
        debugSphereProgram.release();
	}

	return ok;

}

void DeferredResolveProxy::useDebugCone()
{
    if (!debugConeProgram)
    {
        generateDebugCone();
    }

    if (debugConeProgram)
    {
        debugConeProgram->Use();
    }
}

bool DeferredResolveProxy::generateDebugCone()
{
    std::unique_ptr<Shader> vertex, fragment;

    vertex.reset(Shader::CreateVSFromFile("assets/shaders/deferredSpotProxyVS.glsl", 0, 0));

    bool ok = vertex->Compiled();

    if (ok)
    {

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/showProxyFS.glsl", 0, 0));

        ok = fragment->Compiled();
    }

    if (ok)
    {
        debugConeProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "ShowProxy");

        ok = debugConeProgram->Linked();
    }

    if (!ok)
    {
        debugConeProgram.release();
    }

    return ok;

}

void DeferredResolveProxy::useConeProgram()
{
    if (!coneProgram)
    {
        generateConeProgram();
    }

    if (coneProgram)
        coneProgram->Use();
}

void DeferredResolveProxy::useSphereProgram()
{
	if(!sphereProgram)
	{
		generateSphereProgram();
	}

	if(sphereProgram)
	{
		sphereProgram->Use();
	}
}

bool DeferredResolveProxy::generateConeProgram()
{
    std::unique_ptr<Shader> vertex, fragment;

    vertex.reset(Shader::CreateVSFromFile("assets/shaders/deferredSpotProxyVS.glsl", 0, 0));

    bool ok = vertex->Compiled();

    if (ok)
    {

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/deferredSpotProxyFS.glsl", 0, 0));

        ok = fragment->Compiled();
    }

    if (ok)
    {
        coneProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "DeferredConeProxy");

        ok = coneProgram->Linked();
    }

    if (!ok)
    {
        coneProgram.release();
    }

    return ok;
}


bool DeferredResolveProxy::generateSphereProgram()
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
		sphereProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "DeferredProxy");

		ok = sphereProgram->Linked();
	}

	if (!ok)
	{
        sphereProgram.release();
	}

	return ok;
}


