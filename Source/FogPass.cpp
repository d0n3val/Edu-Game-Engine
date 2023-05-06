#include "Globals.h"

#include "FogPass.h"

#include "GBufferExportPass.h"
#include "Application.h"
#include "ModuleHints.h"
#include "ModuleRenderer.h"
#include "OpenGL.h"
#include "OGL.h"
#include "CameraUBO.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

FogPass::FogPass()
{
}

void FogPass::execute(Framebuffer *target, uint width, uint height)
{
    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();

    useProgram();

    program->BindUniform(FOG_DENSITY_HEIGHT_FALLOFF_LOCATION, std::get<float>(App->hints->GetDHint(std::string("fog density falloff"), 0.01f)));
    program->BindUniform(FOG_GLOGAL_DENSITY_LOCATION, std::get<float>(App->hints->GetDHint(std::string("fog global density"), 0.01f)));
    program->BindUniform(FOG_COLOR, std::get<float3>(App->hints->GetDHint(std::string("fog color"), float3(0.5f, 0.6f, 0.7f))));
    program->BindUniform(FOG_SUN_COLOR, std::get<float3>(App->hints->GetDHint(std::string("fog sun color"), float3(1.0f, 0.9f, 0.7f))));


    exportPass->getPosition()->Bind(GBUFFER_POSITION_TEX_BINDING);
    App->renderer->GetCameraUBO()->Bind();

    target->Bind();
    glViewport(0, 0, width, height);

	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    if(!vao) vao = std::make_unique<VertexArray>();

    vao->Bind();

    glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

    vao->Unbind();
	target->Unbind();
}

void FogPass::useProgram()
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

bool FogPass::generateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/fogFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "Fog");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}