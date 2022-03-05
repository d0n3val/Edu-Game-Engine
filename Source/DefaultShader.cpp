#define PAR_STRING_BLOCKS_IMPLEMENTATION

#include "Globals.h"

#include "DefaultShader.h"

#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleHints.h"
#include "GameObject.h"
#include "ComponentMeshRenderer.h"
#include "ModuleRenderer.h"
#include "ScreenSpaceAO.h"
#include "ComponentCamera.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "LightManager.h"
#include "Skybox.h"
#include "BatchManager.h"

#include "OGL.h"
#include "OpenGL.h"
#include "DefaultShaderBindings.h"

#include "Leaks.h"

DefaultShader::DefaultShader()
{
}

DefaultShader::~DefaultShader()
{
}

void DefaultShader::Render(const RenderList &objects)
{
    UseProgram();

    // Bind  camera 
	App->renderer->GetCameraUBO()->BindToPoint(cameraBlockIndex);

	// Bind lights 
	App->level->GetLightManager()->Bind();

    // Bind IBL 
    App->level->GetSkyBox()->BindIBL();

    // opaques 
    App->renderer->GetBatchManager()->Render(objects.GetOpaques(), false);

    // transparents 
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    App->renderer->GetBatchManager()->Render(objects.GetTransparents(), true);

    glDisable(GL_BLEND);
}

void DefaultShader::UseProgram()
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

bool DefaultShader::GenerateProgram()
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