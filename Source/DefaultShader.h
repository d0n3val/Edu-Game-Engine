#ifndef _DEFAULT_SHADER_H_
#define _DEFAULT_SHADER_H_

#include "Math.h"

#include "utils/par_string_blocks.h"

#include <unordered_map>
#include <memory>

class Program;
class ComponentCamera;
class ModuleLevelManager;
class ResourceMaterial;
class ResourceMesh;
class Buffer;

class DefaultShader
{
public:

	enum Vatiations
	{
		SKINING_ON = 0,
		MORPH_TARGETS_ON,
		SHADOWS_ON,
		PHONG_BRDF_ON,

		VARIATIONS_COUNT
	};

	typedef std::unordered_map<uint, std::unique_ptr<Program> > ProgramList;

	parsb_context* blocksVS = nullptr;
	parsb_context* blocksFS = nullptr;
	ProgramList 			programs;
    std::unique_ptr<Buffer> cameraUBO;
    std::unique_ptr<Buffer> lightsUBO;
    std::unique_ptr<Buffer> materialUBO;
    std::unique_ptr<Buffer> skiningUBO;
    std::unique_ptr<Buffer> morphUBO;

public:

	DefaultShader();
	~DefaultShader();

	void Use 				(uint flags = 0);

	void UpdateCameraUBO 	(ComponentCamera* camera);
	void UpdateLightUBO 	(ModuleLevelManager* level);
	void UpdateMaterialUBO 	(ResourceMaterial* material);
	void UpdateMeshUBO		(const float4x4& model, float4x4* skinPalette, float* morphWeights, ResourceMesh* mesh);

private:

	const char* GetShaderSource  (uint flags, parsb_context* context);
	bool 		ExistsBlock 	 (parsb_context* blocks, const char* name) const;
	void 		AddBlocksFromFile(parsb_context* blocks, const char* fileName) const;
	void 		BindUniformBlock (uint program, const char* name, uint block_index) const;

};

#endif /*_DEFAULT_SHADER_H_*/
