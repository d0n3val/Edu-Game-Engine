#include "Globals.h"

#define PAR_STRING_BLOCKS_IMPLEMENTATION

#include "DefaultShader.h"

#include "Application.h"
#include "ModuleLevelManager.h"
#include "ComponentCamera.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "DirLight.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "SpotLight.h"

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

static const char* variationsOn[]  = {"MORPH", "SKINING", "NO_SHADOW", "PHONG"}; 
static const char* variationsOff[] = {"NO_MORPH", "NO_SKINING", "SHADOW", "NO_PHONG"};

DefaultShader::DefaultShader()
{
	blocksVS = parsb_create_context(parsb_options{});
	blocksFS = parsb_create_context(parsb_options{});

	AddBlocksFromFile(blocksVS, "assets/shaders/defaultSnippetsVS.glsl");
	AddBlocksFromFile(blocksFS, "assets/shaders/defaultSnippetsFS.glsl");
}

DefaultShader::~DefaultShader()
{
	parsb_destroy_context(blocksVS);
	parsb_destroy_context(blocksFS);
}

void DefaultShader::Use(uint flags)
{
	std::unique_ptr<Program>& program = programs[flags];

	if(!program)
	{
		const char* sourceVS = GetShaderSource(flags, blocksVS);
		const char* sourceFS = GetShaderSource(flags, blocksFS);

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
			program.reset(new Program(vertex.get(), fragment.get(), 2, "Default program"));
		}

        if(program->Linked())
        {
            BindUniformBlock(program->Id(), "Camera", 0);
            BindUniformBlock(program->Id(), "Material", 1);
            BindUniformBlock(program->Id(), "Lights", 2);
            BindUniformBlock(program->Id(), "Skining", 3);
            BindUniformBlock(program->Id(), "Morph", 4);

            int location = glGetUniformLocation(program->Id(), "materialMaps");
            if (location >= 0)
            {
                static int maps[] = { 0, 1, 2, 3, 4, 5 };
                glUniform1iv(location, sizeof(maps) / sizeof(int), &maps[0]);
            }
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
    }

    cameraData.proj     = camera->GetProjectionMatrix();  
    cameraData.view     = camera->GetViewMatrix();
    cameraData.view_pos = cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart());

    cameraUBO->SetData(0, sizeof(CameraData), &cameraData);
    cameraUBO->BindToTargetIdx(0);
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
    lightsUBO->BindToTargetIdx(2);
}

void DefaultShader::UpdateMaterialUBO (ResourceMaterial* material)
{
    struct MaterialData
    {
        float4    diffuse_color;
        float4    specular_color;
        float4    emissive_color;
        float     smoothness;
        float     normal_strength;
        float     alpha_test;
        uint      mapMask;
    };

    MaterialData materialData = { material->GetDiffuseColor(), material->GetSpecularColor(), material->GetEmissiveColor(), 
                                  material->GetSmoothness(), material->GetNormalStrength(), material->GetAlphaTest(), 
                                  material->GetMapMask()};

    if (!materialUBO)
    {
        materialUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(MaterialData), nullptr));
    }

    materialUBO->SetData(0, sizeof(MaterialData), &materialData);
    materialUBO->BindToTargetIdx(1);
}

void DefaultShader::UpdateMeshUBO(const float4x4& model, float4x4* skinPalette, float* morphWeights, ResourceMesh* mesh)
{
    if(mesh->HasAttrib(ATTRIB_BONES))
    {
        struct SkiningData
        {
            float4x4 palette[MAX_BONES];
        };

        SkiningData* data = reinterpret_cast<SkiningData*>(skinPalette);

        if (!skiningUBO)
        {
            skiningUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(SkiningData), nullptr));
        }

        skiningUBO->SetData(0, sizeof(SkiningData), data);
        skiningUBO->BindToTargetIdx(3);
    }

    if(morphWeights != nullptr)
    {
        struct MorphData
        {
            int target_stride;
            int normals_stride;
            int tangents_stride;
            float weights[MAX_MORPH_TARGETS];
            int num_targets;
        };

        MorphData data;
        data.target_stride   = mesh->GetNumVertices()*mesh->GetMorphNumAttribs();
        data.normals_stride  = mesh->GetNumVertices();
        data.tangents_stride = mesh->GetNumVertices()*2;
        data.num_targets   = mesh->GetNumMorphTargets();

        for (int i = 0; i < data.num_targets; ++i)
        {
            data.weights[i] = morphWeights[i];
        }

        if(!morphUBO)
        {
            morphUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(MorphData), nullptr));
        }

        morphUBO->SetData(0, sizeof(MorphData), &data);
        morphUBO->BindToTargetIdx(4);
    }
}

const char* DefaultShader::GetShaderSource(uint flags, parsb_context* blocks)
{
	std::string variations;

	if(ExistsBlock(blocks, "PREFIX"))
	{
		variations += "PREFIX ";
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
    int index = glGetUniformBlockIndex(program, "Camera");
    if (index != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(program, index, block_index);
    }
}

