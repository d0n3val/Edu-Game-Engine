#ifndef _DEFAULT_SHADER_H_
#define _DEFAULT_SHADER_H_

#include "Math.h"
#include "utils/par_string_blocks.h"
#include "RenderList.h"

#include <unordered_map>
#include <memory>

class Program;
class ComponentCamera;
class ComponentMeshRenderer;
class ModuleLevelManager;
class ResourceMaterial;
class ResourceMesh;
class Buffer;
class BatchManager;
struct SceneBuffers;

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
		CAMERA_UBO_TARGET   = 0,
		MATERIAL_UBO_TARGET = 1,
		LIGHTS_UBO_TARGET 	= 2,
		SKINING_UBO_TARGET  = 3,
		MORPH_UBO_TARGET 	= 4
	};

	typedef std::unordered_map<uint, std::unique_ptr<Program> > ProgramList;

	parsb_context* blocksVS = nullptr;
	parsb_context* blocksFS = nullptr;
	parsb_context* depthFS  = nullptr;

	ProgramList 			drawPrograms;
	ProgramList 			depthPrograms;
    std::unique_ptr<Buffer> skiningUBO;
    std::unique_ptr<Buffer> morphUBO;

public:

	DefaultShader();
	~DefaultShader();

	void Render 			(BatchManager* batch, const RenderList& objects, const Buffer* cameraUBO);

private:

	void 		UseDrawPass 	    (uint flags = 0);

	const char* GetShaderSource     (uint flags, bool depthPass, parsb_context* context);
	bool 		ExistsBlock 	    (parsb_context* blocks, const char* name) const;
	void 		AddBlocksFromFile   (parsb_context* blocks, const char* fileName) const;
	void 		BindUniformBlock    (uint program, const char* name, uint block_index) const;
	void		BindTextures	    (const ResourceMaterial* material) const;
    uint        GetDrawingFlags     (const ResourceMesh* mesh) const;

};

#endif /*_DEFAULT_SHADER_H_*/
