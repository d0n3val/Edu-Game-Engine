#ifndef _DEFAULT_SHADER_H_
#define _DEFAULT_SHADER_H_

#include "Math.h"

#include "utils/par_string_blocks.h"

#include <unordered_map>
#include <memory>

class Program;
class ComponentCamera;
class ComponentMeshRenderer;
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

	enum UBOTargets
	{
		CAMERA_UBO_TARGET = 0,
		MATERIAL_UBO_TARGET = 1,
		LIGHTS_UBO_TARGET = 2,
		SKINING_UBO_TARGET = 3,
		MORPH_UBO_TARGET = 4
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

	void Draw 				(ComponentMeshRenderer* meshRenderer);
	void UpdateLightUBO 	(ModuleLevelManager* level);
	void UpdateCameraUBO	(ComponentCamera* camera);

private:

	void 		Use 				(uint flags = 0);
	void		UpdateMaterialUBO	(ResourceMaterial* material);
	void 		UpdateMeshUBOs		(const float4x4* skinPalette, const float* morphWeights, const ResourceMesh* mesh);

	const char* GetShaderSource     (uint flags, parsb_context* context);
	bool 		ExistsBlock 	    (parsb_context* blocks, const char* name) const;
	void 		AddBlocksFromFile   (parsb_context* blocks, const char* fileName) const;
	void 		BindUniformBlock    (uint program, const char* name, uint block_index) const;
	void		BindTextures	    (const ResourceMaterial* material) const;
    uint        GetDrawingFlags     (const ResourceMesh* mesh) const;

};

#endif /*_DEFAULT_SHADER_H_*/
