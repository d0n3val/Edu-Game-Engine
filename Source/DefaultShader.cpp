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

#define MAX_NUM_POINT_LIGHTS 4
#define MAX_NUM_SPOT_LIGHTS 4
#define MAX_BONES 64
#define MAX_MORPH_TARGETS 128

enum ETotalTextureMaps
{
    TextureMap_DiffuseIBL = TextureCount,
    TextureMap_PrefilteredIBL,
    TextureMap_EnvironMentBRDF,
    TextureMap_AO,
    TextureMap_Count
};

static const char* variationsOn[]  = { "SKINING", "MORPH", "NO_SHADOW", "PHONG" };
static const char* variationsOff[] = { "NO_SKINING", "NO_MORPH", "SHADOW", "NO_PHONG" };

DefaultShader::DefaultShader()
{
	blocksVS = parsb_create_context(parsb_options{});
	blocksFS = parsb_create_context(parsb_options{});
	depthFS  = parsb_create_context(parsb_options{});

	AddBlocksFromFile(blocksVS, "assets/shaders/defaultSnippetsVS.glsl");
	AddBlocksFromFile(blocksFS, "assets/shaders/defaultSnippetsFS.glsl");
	AddBlocksFromFile(depthFS, "assets/shaders/depthPrepassSnipetFS.glsl");

    parsb_add_block(blocksVS, "DEPTH_MACROS", "#define DEPTH_PREPASS\n");
    parsb_add_block(blocksFS, "DEPTH_MACROS", "#define DEPTH_PREPASS\n");
    parsb_add_block(depthFS,  "DEPTH_MACROS", "#define DEPTH_PREPASS\n");
}

DefaultShader::~DefaultShader()
{
	parsb_destroy_context(blocksVS);
	parsb_destroy_context(blocksFS);
	parsb_destroy_context(depthFS);
}

void DefaultShader::UseDrawPass(uint flags)
{
	std::unique_ptr<Program>& program = drawPrograms[flags];

	if(!program)
	{
		const char* sourceVS = GetShaderSource(flags, false, blocksVS);
		const char* sourceFS = GetShaderSource(flags, false, blocksFS);

		std::unique_ptr<Shader> vertex = std::make_unique<Shader>(GL_VERTEX_SHADER, &sourceVS, 1);

		if (!vertex->Compiled())
		{
			LOG("Vertex Shader Code \n%s", sourceVS);
		}

		std::unique_ptr<Shader> fragment = std::make_unique<Shader>(GL_FRAGMENT_SHADER, &sourceFS, 1);

		if (!fragment->Compiled())
		{
			LOG("Fragment Shader Code \n%s", sourceFS);
		}

		if(vertex->Compiled() && fragment->Compiled())
		{
			program.reset(new Program(vertex.get(), fragment.get(), "Default program"));
		}
    }

	program->Use();
}

const char* DefaultShader::GetShaderSource(uint flags, bool depthPass, parsb_context* blocks)
{
	std::string variations;

	if(ExistsBlock(blocks, "PREFIX"))
	{
		variations += "PREFIX ";
	}

    if(depthPass)
    {
        variations += "DEPTH_MACROS ";
    }

	if(ExistsBlock(blocks, "DATA"))
	{
		variations += "DATA ";
	}

	for(uint i=0; i< VARIATIONS_COUNT; ++i)
	{		
		if ((flags & (1 << i)) != 0) 
		{
			if(ExistsBlock(blocks, variationsOn[i]))
			{
				variations += variationsOn[i];
				variations += " ";
			}
		}
		else
		{
			if(ExistsBlock(blocks, variationsOff[i]))
			{
				variations += variationsOff[i];
				variations += " ";
			}
		}
	}

	variations += "MAIN";

	return parsb_get_blocks(blocks, variations.c_str());
}

bool DefaultShader::ExistsBlock(parsb_context* blocks, const char* name) const
{
	bool found = false;
	for(uint i=0, count = parsb_get_num_blocks(blocks); !found && i < count; ++i)
	{
		const char* blockName = nullptr;
		const char* blockBody = nullptr;

		parsb_get_block(blocks, i, &blockName, &blockBody);

		if(name != nullptr )
		{
			found = !strcmp(name, blockName);
		}
	}

	return found;
}

void DefaultShader::AddBlocksFromFile(parsb_context* blocks, const char* fileName) const
{
	std::ifstream t(fileName);
	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(unsigned(t.tellg()));
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	parsb_add_blocks(blocks, str.c_str(), str.length());

    if(parsb_get_num_blocks(blocks) == 0)
    {
    	parsb_add_block(blocks, "MAIN", str.c_str());
    }
}

void DefaultShader::BindUniformBlock (uint program, const char* name, uint block_index) const
{
    int index = glGetUniformBlockIndex(program, name);
    if (index != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(program, index, block_index);
    }
}

void DefaultShader::BindTextures(const ResourceMaterial* material) const
{
    if (material)
    {
        for (uint i = 0; i < TextureCount; ++i)
        {
            const ResourceTexture* texture = material->GetTextureRes(MaterialTexture(i));

            unsigned id = 0;
            
            if (texture) id = texture->GetID();

            glActiveTexture(GL_TEXTURE0+i);
            glBindTexture(GL_TEXTURE_2D, id);
        }

        Skybox* skybox = App->level->GetSkyBox();

        if (skybox->GetDiffuseIBL() && skybox->GetPrefilterdIBL() && skybox->GetEnvironmentBRDF())
        {
            glActiveTexture(GL_TEXTURE0 + TextureMap_DiffuseIBL);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetDiffuseIBL()->Id());
            glActiveTexture(GL_TEXTURE0 + TextureMap_PrefilteredIBL);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetPrefilterdIBL()->Id());

            glActiveTexture(GL_TEXTURE0 + TextureMap_EnvironMentBRDF);
            glBindTexture(GL_TEXTURE_2D, skybox->GetEnvironmentBRDF()->Id());

            glActiveTexture(GL_TEXTURE0 + TextureMap_AO);
            glBindTexture(GL_TEXTURE_2D, App->renderer->GetScreenSpaceAO()->getResult()->Id());
        }
    }
}

uint DefaultShader::GetDrawingFlags(const ResourceMesh* mesh) const
{
    uint flags = 0;

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING)) flags |= (1<< SHADOWS_ON);
    if(mesh->HasAttrib(ATTRIB_BONES)) flags |=(1<< SKINING_ON);
    if(mesh->GetNumMorphTargets() > 0) flags |=(1<< MORPH_TARGETS_ON);

    return flags;
}

void DefaultShader::Render(BatchManager *batch, const RenderList &objects, const Buffer* cameraUBO)
{
    UseDrawPass(0);

    // \todo: Add bindings into a header shared with shaders
    const uint transformBlockIndex = 10;
    const uint materialsBlockIndex = 11;
    const uint directionalBlockIndex = 12;
    const uint pointBlockIndex = 13;
    const uint spotBlockIndex = 14;
    const uint cameraBlockIndex = 0;
    const uint levelsLoc = 64;

    // Bind  camera
    cameraUBO->BindToPoint(cameraBlockIndex);

	// Bind lights
	App->level->GetLightManager()->Bind(directionalBlockIndex, pointBlockIndex, spotBlockIndex);

    // Bind IBL
    App->level->GetSkyBox()->BindIBL(levelsLoc, TextureMap_DiffuseIBL, TextureMap_PrefilteredIBL, TextureMap_EnvironMentBRDF);

    // opaques
    batch->Render(objects.GetOpaques(), transformBlockIndex, materialsBlockIndex, false);

    // transparents
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    batch->Render(objects.GetTransparents(), transformBlockIndex, materialsBlockIndex, true);

    glDisable(GL_BLEND);
}