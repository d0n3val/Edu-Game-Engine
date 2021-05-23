#include "Globals.h"

#define PAR_STRING_BLOCKS_IMPLEMENTATION

#include "DefaultShader.h"

#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleHints.h"
#include "GameObject.h"
#include "ComponentMeshRenderer.h"
#include "ComponentCamera.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "DirLight.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Skybox.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

#include <string>
#include <fstream>
#include <streambuf>

#define MAX_NUM_POINT_LIGHTS 4
#define MAX_NUM_SPOT_LIGHTS 4
#define MAX_BONES 64
#define MAX_MORPH_TARGETS 128

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

        if(program && program->Linked())
        {
            BindUniformBlock(program->Id(), "Camera",   CAMERA_UBO_TARGET);
            BindUniformBlock(program->Id(), "Material", MATERIAL_UBO_TARGET);
            BindUniformBlock(program->Id(), "Lights",   LIGHTS_UBO_TARGET);
            BindUniformBlock(program->Id(), "Skining",  SKINING_UBO_TARGET);
            BindUniformBlock(program->Id(), "Morph",    MORPH_UBO_TARGET);
        }
    }

    int location = glGetUniformLocation(program->Id(), "materialMaps");

    if (location >= 0)
    {
        static int maps[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        glUniform1iv(location, sizeof(maps) / sizeof(int), &maps[0]);
    }

    location = glGetUniformLocation(program->Id(), "diffuseIBL");
    if(location >= 0)
    {
        glUniform1i(location, TextureCount+0);
    }

    location = glGetUniformLocation(program->Id(), "prefilteredIBL");
    if(location >= 0)
    {
        glUniform1i(location, TextureCount + 1);
    }

    location = glGetUniformLocation(program->Id(), "prefilteredLevels");
    if(location >= 0)
    {
        glUniform1i(location, App->level->GetSkyBox()->GetPrefilterdLevels());
    }

    location = glGetUniformLocation(program->Id(), "environmentBRDF");
    if(location >= 0)
    {
        glUniform1i(location, TextureCount + 2);
    }

	program->Use();
}

void DefaultShader::UseDepthPrePass(uint flags)
{
    std::unique_ptr<Program>& program = depthPrograms[flags];

    if(!program)
    {
        const char* sourceVS = GetShaderSource(flags, true, blocksVS);
        const char* sourceFS = GetShaderSource(flags, true, depthFS);

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

        if(program && program->Linked())
        {
            BindUniformBlock(program->Id(), "Camera",   CAMERA_UBO_TARGET);
            BindUniformBlock(program->Id(), "Skining",  SKINING_UBO_TARGET);
            BindUniformBlock(program->Id(), "Morph",    MORPH_UBO_TARGET);
        }
    }

    program->Use();
}

void DefaultShader::UpdateCameraUBO(ComponentCamera* camera)
{
    struct CameraData
    {
        float4x4 proj     = float4x4::identity;
        float4x4 view     = float4x4::identity;
        float3   view_pos = float3::zero;
    } cameraData;

    if(!cameraUBO)
    {
        cameraUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(cameraData), nullptr));
        cameraUBO->BindToTargetIdx(CAMERA_UBO_TARGET);
    }

    cameraData.proj     = camera->GetProjectionMatrix();  
    cameraData.view     = camera->GetViewMatrix();
    cameraData.view_pos = cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart());

    cameraUBO->SetData(0, sizeof(CameraData), &cameraData);
}

void DefaultShader::UpdateLightUBO(ModuleLevelManager* level)
{
    struct AmbientLightData
    {
        float4 color;
    };

    struct DirLightData
    {
        float4 dir;
        float4 color;
    };

    struct PointLightData
    {
        float4 position;
        float4 color;
        float4 attenuation;
    };

    struct SpotLightData
    {
        float4 position;
        float4 direction;
        float4 color;
        float4 attenuation;
        float inner;
        float outer;
    };

    struct LightsData
    {
        AmbientLightData ambient;
        DirLightData     directional;
        PointLightData   points[MAX_NUM_POINT_LIGHTS];
        uint             num_point;
        SpotLightData    spots[MAX_NUM_SPOT_LIGHTS];
        uint             num_spot;
    };

    if(!lightsUBO)
    {
        lightsUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(LightsData), nullptr));
        lightsUBO->BindToTargetIdx(LIGHTS_UBO_TARGET);

    }

    LightsData data;

    const DirLight* directional = App->level->GetDirLight();
    const AmbientLight* ambient = App->level->GetAmbientLight();

    data.ambient.color     =  float4(ambient->GetColor(), 1.0);
    data.directional.dir   =  float4(directional->GetDir(), 0.0);
    data.directional.color =  float4(directional->GetColor(), 1.0);

    data.num_point = 0;

    for(uint i=0, count = min(App->level->GetNumPointLights(), MAX_NUM_POINT_LIGHTS); i < count; ++i)
    {
        const PointLight* light = App->level->GetPointLight(i);

        if(light->GetEnabled())
        {
            uint index = data.num_point++;

            data.points[index].position    = float4(light->GetPosition(), 0.0);
            data.points[index].color       = float4(light->GetColor(), 0.0);
            data.points[index].attenuation = float4(light->GetConstantAtt(), light->GetLinearAtt(), light->GetQuadricAtt(), 0.0);
        }
    }

    data.num_spot = 0;

    for(uint i=0, count = min(App->level->GetNumSpotLights(), MAX_NUM_SPOT_LIGHTS); i< count; ++i)
    {
        const SpotLight* light = App->level->GetSpotLight(i);

        if(light->GetEnabled())
        {
            uint index = data.num_spot++;

            data.spots[index].position    = float4(light->GetPosition(), 0.0f);
            data.spots[index].direction   = float4(light->GetDirection(), 0.0f);
            data.spots[index].color       = float4(light->GetColor(), 1.0);
            data.spots[index].attenuation = float4(light->GetConstantAtt(), light->GetLinearAtt(), light->GetQuadricAtt(), 0.0f);
            data.spots[index].inner       = light->GetInnerCutoff();
            data.spots[index].outer       = light->GetOutterCutoff();
        }
    }

    lightsUBO->SetData(0, sizeof(LightsData), &data);
}

void DefaultShader::UpdateMaterialUBO (ResourceMaterial* material)
{
    struct MaterialData
    {
        float4 diffuse_color;
        float4 specular_color;
        float4 emissive_color;
        float2 tiling;
        float2 offset;
        float2 secondary_tiling;
        float2 secondary_offset;
        float  smoothness;
        float  normal_strength;
        float  alpha_test;
        uint   mapMask;
    };

    if (material)
    {
        MaterialData materialData = { material->GetDiffuseColor(), material->GetSpecularColor(), material->GetEmissiveColor(),
                                      material->GetUVTiling(), material->GetUVOffset(), material->GetSecondUVTiling(),
                                      material->GetSecondUVOffset(), material->GetSmoothness(), material->GetNormalStrength(), 
                                      material->GetAlphaTest(), material->GetMapMask() };

        if (!materialUBO)
        {
            materialUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(MaterialData), nullptr));
            materialUBO->BindToTargetIdx(MATERIAL_UBO_TARGET);
        }

        materialUBO->SetData(0, sizeof(MaterialData), &materialData);
    }
}

void DefaultShader::UpdateMeshUBOs(const float4x4* skinPalette, const float* morphWeights, const ResourceMesh* mesh)
{
    if(mesh->HasAttrib(ATTRIB_BONES))
    {
        struct SkiningData
        {
            float4x4 palette[MAX_BONES];
        };

        const SkiningData* data = reinterpret_cast<const SkiningData*>(skinPalette);

        if (!skiningUBO)
        {
            skiningUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(SkiningData), nullptr));
        }

        skiningUBO->SetData(0, sizeof(SkiningData), data);
        skiningUBO->BindToTargetIdx(SKINING_UBO_TARGET);
    }

    if(morphWeights != nullptr)
    {
        struct MorphData
        {
            int   target_stride;
            int   normals_stride;
            int   tangents_stride;
            float weights[MAX_MORPH_TARGETS];
            int   num_targets;
        };

        MorphData data;
        data.target_stride   = mesh->GetNumVertices()*mesh->GetMorphNumAttribs();
        data.normals_stride  = mesh->GetNumVertices();
        data.tangents_stride = mesh->GetNumVertices()*2;
        data.num_targets     = mesh->GetNumMorphTargets();

        for (int i = 0; i < data.num_targets; ++i)
        {
            data.weights[i] = morphWeights[i];
        }

        if(!morphUBO)
        {
            morphUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(MorphData), nullptr));
        }

        morphUBO->SetData(0, sizeof(MorphData), &data);
        morphUBO->BindToTargetIdx(MORPH_UBO_TARGET);
    }
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
        glActiveTexture(GL_TEXTURE0+TextureCount+0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetDiffuseIBL()->Id());
        glActiveTexture(GL_TEXTURE0+TextureCount+1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->GetPrefilterdIBL()->Id());

        glActiveTexture(GL_TEXTURE0+TextureCount+2);
        glBindTexture(GL_TEXTURE_2D, skybox->GetEnvironmentBRDF()->Id());
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

void DefaultShader::DrawPass(ComponentMeshRenderer* meshRenderer)
{
    const ResourceMesh* mesh   = meshRenderer->GetMeshRes();
    ResourceMaterial* material = meshRenderer->GetMaterialRes();
    GameObject* go             = meshRenderer->GetGameObject();

    uint flags = GetDrawingFlags(mesh);

    UpdateMaterialUBO(material);
    UpdateMeshUBOs(meshRenderer->UpdateSkinPalette(), meshRenderer->GetMorphTargetWeights(), mesh);

    UseDrawPass(flags);

    glUniformMatrix4fv(glGetUniformLocation(drawPrograms[flags]->Id(), "model"), 1, GL_TRUE, reinterpret_cast<const float*>(&go->GetGlobalTransformation()));
    BindTextures(material);

    mesh->Draw();
}

void DefaultShader::DepthPrePass(ComponentMeshRenderer* meshRenderer)
{
    const ResourceMesh* mesh   = meshRenderer->GetMeshRes();
    ResourceMaterial* material = meshRenderer->GetMaterialRes();
    GameObject* go             = meshRenderer->GetGameObject();

    uint flags = GetDrawingFlags(mesh);

    UpdateMeshUBOs(meshRenderer->UpdateSkinPalette(), meshRenderer->GetMorphTargetWeights(), mesh);

    UseDepthPrePass(flags);

    glUniformMatrix4fv(glGetUniformLocation(depthPrograms[flags]->Id(), "model"), 1, GL_TRUE, reinterpret_cast<const float*>(&go->GetGlobalTransformation()));

    mesh->Draw();
}
