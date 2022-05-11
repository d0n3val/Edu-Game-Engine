#include "Globals.h"

#include "GBufferExportPass.h"

#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "LightManager.h"
#include "Skybox.h"
#include "BatchManager.h"
#include "RenderList.h"
#include "OGL.h"
#include "OpenGL.h"

#include <assert.h>

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

GBufferExportPass::GBufferExportPass()
{
}

void GBufferExportPass::execute(const RenderList &nodes, uint width, uint height)
{
    resizeFrameBuffer(width, height);

    useProgram();

    glDisable(GL_BLEND);
    glViewport(0, 0, width, height);

    float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    frameBuffer->ClearColor(0, clearColor);
    frameBuffer->ClearColor(1, clearColor);
    frameBuffer->ClearColor(2, clearColor);
    frameBuffer->ClearColor(3, clearColor);
    frameBuffer->ClearColor(4, clearColor);
    frameBuffer->ClearDepth(1.0f);

    frameBuffer->Bind();
    
    App->renderer->GetBatchManager()->Render(nodes.GetOpaques(), false);

    frameBuffer->Unbind();
}

void GBufferExportPass::useProgram()
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

void GBufferExportPass::resizeFrameBuffer(uint width, uint height)
{
    if (width != fbWidth || height != fbHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        albedoTex   = std::make_unique<Texture2D>(width, height, GL_RGB8, GL_RGB, GL_UNSIGNED_INT, nullptr, false);
        specularTex = std::make_unique<Texture2D>(width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT, nullptr, false);
        emissiveTex = std::make_unique<Texture2D>(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr, false);
        depthTex    = std::make_unique<Texture2D>(width, height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        positionTex = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
        normalTex   = std::make_unique<Texture2D>(width, height, GL_RGB8, GL_RGB, GL_UNSIGNED_INT, nullptr, false);

        albedoTex->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        specularTex->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        emissiveTex->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        depthTex->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        positionTex->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
        normalTex->SetWrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

        frameBuffer->AttachColor(albedoTex.get(), 0, 0);
        frameBuffer->AttachColor(specularTex.get(), 1, 0);
        frameBuffer->AttachColor(emissiveTex.get(), 2, 0);
        frameBuffer->AttachColor(positionTex.get(), 3, 0);
        frameBuffer->AttachColor(normalTex.get(), 4, 0);
        frameBuffer->AttachDepthStencil(depthTex.get(), GL_DEPTH_ATTACHMENT);

        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth = width;
        fbHeight = height;
    }
}

bool GBufferExportPass::generateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{
		fragment.reset(Shader::CreateFSFromFile("assets/shaders/gbufferFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "ExportGBuffer");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}