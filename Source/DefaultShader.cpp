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

#include "Leaks.h"

namespace
{
	const uint transformBlockIndex = 10;
	const uint materialsBlockIndex = 5;
	const uint instancesBlockIndex = 15;
    const uint skinningBlockIndex = 16;
	const uint directionalBlockIndex = 12;
	const uint pointBlockIndex = 13;
	const uint spotBlockIndex = 14;
	const uint cameraBlockIndex = 0;
	const uint levelsLoc = 64;
	const uint diffuseIBL = TextureCount;
	const uint prefilteredIBL = TextureCount + 1;
	const uint environmentIBL = TextureCount + 2;
}

DefaultShader::DefaultShader()
{
}

DefaultShader::~DefaultShader()
{
}

void DefaultShader::Render(BatchManager *batch, const RenderList &objects, const Buffer* cameraUBO)
{
    UseProgram();

    // Bind  camera 
    cameraUBO->BindToPoint(cameraBlockIndex);

	// Bind lights 
	App->level->GetLightManager()->Bind(directionalBlockIndex, pointBlockIndex, spotBlockIndex);

    // Bind IBL 
    App->level->GetSkyBox()->BindIBL(levelsLoc, diffuseIBL, prefilteredIBL, environmentIBL);

    // opaques 
    batch->Render(objects.GetOpaques(), transformBlockIndex, materialsBlockIndex, instancesBlockIndex, skinningBlockIndex, false);

    // transparents 
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    batch->Render(objects.GetTransparents(), transformBlockIndex, materialsBlockIndex, instancesBlockIndex, skinningBlockIndex, true);

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