#include "Globals.h"

#include "DeferredDecalPass.h"
#include "RenderList.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleRenderer.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ComponentDecal.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "GBufferExportPass.h"
#include "CameraUBO.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

namespace
{

	const float cube_vertices[6 * 6 * 3] = {
		-0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

		0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,

		-0.5f, 0.5f, -0.5f,
        0.5f, 0.5f, 0.5f,
        0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, 0.5f,

		-0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f };

}

DeferredDecalPass::DeferredDecalPass()
{
}

void DeferredDecalPass::execute(ComponentCamera* camera, const RenderList& objects, uint width, uint height)
{
    const NodeList& decals = objects.GetDecals();

    // TODO: Start rendering first one, later we will efficiently render the others 

    if(!decals.empty())
    {
		ComponentDecal* decal = decals.front().decal;
        if(decal->IsValid())
        { 
            generateFramebuffer(width, height);
            generateCube();
            useProgram();

            float3 cameraPos = camera->GetPos();

            GBufferExportPass* gbuffer = App->renderer->GetGBufferExportPass();

            // bind model
            float4x4 model = decal->GetGameObject()->GetGlobalTransformation();
            float4x4 invModel = model;
            invModel.InverseColOrthogonal();

            float3 cameraPosLocal = (invModel * float4(cameraPos, 1.0f)).xyz();

            bool insideDecalBox = fabs(cameraPosLocal.x) < 0.5 && fabs(cameraPosLocal.y) < 0.5 && fabs(cameraPosLocal.z) < 0.5;

            if (insideDecalBox)
            {
                glDisable(GL_DEPTH_TEST);
                glFrontFace(GL_CW);
            }

            glDepthMask(GL_FALSE);

            program->BindUniform(MODEL_LOCATION, model);
            program->BindUniform(INV_MODEL_LOCATION, invModel);
            program->BindUniform(NORMAL_STRENGTH_LOCATION, decal->GetNormalStrength());

            // bind G-buffer textures
            gbuffer->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
            decal->GetAlbedoRes()->GetTexture()->Bind(DECAL_ALBEDO_TEX_BINDING);
            decal->GetNormalRes()->GetTexture()->Bind(DECAL_NORMAL_TEX_BINDING);
            decal->GetSpecularRes()->GetTexture()->Bind(DECAL_SPECULAR_TEX_BINDING);

            App->renderer->GetCameraUBO()->Bind();

            frameBuffer->Bind();
            glViewport(0, 0, width, height);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, GLuint(decal->GetAlbedoRes()->GetID()));


            glDisable(GL_BLEND);

            // Draw cube
            vao->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
            glDisable(GL_DEPTH_TEST);

            vao->Unbind();

            frameBuffer->Unbind();

            if (insideDecalBox)
            {
                glEnable(GL_DEPTH_TEST);
                glFrontFace(GL_CCW);
            }
            glDepthMask(GL_TRUE);
        }
	}
}

void DeferredDecalPass::useProgram()
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

bool DeferredDecalPass::generateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/decalVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/decalFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "DeferredDecal");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}

void DeferredDecalPass::generateCube()
{
	if(!vbo || ! vao)
	{
		VertexAttrib attribs[] = {{0, 3, GL_FLOAT, false, 0, 0}};

		vbo = std::unique_ptr<Buffer>(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(cube_vertices), cube_vertices));
		vao = std::make_unique<VertexArray>(vbo.get(), nullptr, attribs, uint(sizeof(attribs) / sizeof(VertexAttrib)));
	}
}

void DeferredDecalPass::generateFramebuffer(uint width, uint height)
{
	if(!frameBuffer)
	{
		frameBuffer = std::make_unique<Framebuffer>();
	}

	frameBuffer->ClearAttachments();

	GBufferExportPass *gbuffer = App->renderer->GetGBufferExportPass();

    worldPosTex = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
    objPosTex = std::make_unique<Texture2D>(width, height, GL_RGB32F, GL_RGB, GL_FLOAT, nullptr, false);
    
    frameBuffer->AttachColor(gbuffer->getAlbedo(), 0, 0);
	frameBuffer->AttachColor(gbuffer->getNormal(), 1, 0);
	frameBuffer->AttachColor(gbuffer->getSpecular(), 2, 0);
    frameBuffer->AttachColor(worldPosTex.get(), 3, 0);
    frameBuffer->AttachColor(objPosTex.get(), 4, 0);

	frameBuffer->AttachDepthStencil(gbuffer->getDepth(), GL_DEPTH_ATTACHMENT);

	assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);
}