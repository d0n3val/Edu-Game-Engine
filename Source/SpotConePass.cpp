#include "Globals.h"

#include "ResourceMesh.h"
#include "SpotConePass.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleRenderer.h"
#include "ModuleResources.h"
#include "GBufferExportPass.h"
#include "ComponentSpotCone.h"
#include "CameraUBO.h"
#include "ResourceTexture.h"

#include "OpenGL.h"
#include "OGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

SpotConePass::SpotConePass()
{

}

SpotConePass::~SpotConePass()
{

}

void SpotConePass::execute(const RenderList& objects, Framebuffer *target, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Spot Cones");

    target->Bind();
    glViewport(0, 0, width, height);

    useProgram();

    App->renderer->GetCameraUBO()->Bind();
	App->renderer->GetGBufferExportPass()->getDepth()->Bind(SPOTCONE_DEPTH_BINDING);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for(const TRenderInfo& info : objects.GetSpotCones())
    {
        const ResourceMesh* mesh = info.spotCone->getMeshRes();

        if (mesh)
        {
            program->BindUniform(MODEL_MATRIX_BINDING, info.go->GetGlobalTransformation());

			program->BindUniform(SPOTCONE_COLOUR, info.spotCone->getColour());
			program->BindUniform(SPOTCONE_FOG_TILING0, info.spotCone->getFog0Tiling());
			program->BindUniform(SPOTCONE_FOG_OFFSET0, info.spotCone->getFog0Offset());
			program->BindUniform(SPOTCONE_FOG_SPEED0, info.spotCone->getFog0Speed());

			program->BindUniform(SPOTCONE_FOG_TILING1, info.spotCone->getFog1Tiling());
			program->BindUniform(SPOTCONE_FOG_OFFSET1, info.spotCone->getFog1Offset());
			program->BindUniform(SPOTCONE_FOG_SPEED1, info.spotCone->getFog1Speed());

			program->BindUniform(SPOTCONE_FOG_TIME, info.spotCone->getTime());

			program->BindUniform(SPOTCONE_FOG_TRANSPARENCY, info.spotCone->getTransparency());
			program->BindUniform(SPOTCONE_FOG_SMOOTH_PARTICLE_AMOUNT, info.spotCone->getSmoothAmount());
			program->BindUniform(SPOTCONE_FOG_SMOOTH_FRESNEL_AMOUNT, info.spotCone->getFresnelAmount());

			ResourceTexture* res =info.spotCone->getFog0Res();
			
			if(res)
			{
				res->GetTexture()->Bind(SPOTCONE_FOG_BINDING0);
			}

			res = info.spotCone->getFog1Res();

			if(res)
			{
				res->GetTexture()->Bind(SPOTCONE_FOG_BINDING1);
			}


            glBindVertexArray(mesh->GetVAO());
            glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
        }
    }

    glDisable(GL_BLEND);

    glPopDebugGroup();
}

void SpotConePass::useProgram()
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

bool SpotConePass::generateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/spotConeVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/spotConeFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "SpotCone");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}