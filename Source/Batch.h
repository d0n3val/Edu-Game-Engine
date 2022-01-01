#ifndef _BATCH_H_
#define _BATCH_H_

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "TextureBatch.h"
#include "OGL.h"
#include "HashString.h"

#include <memory>
#include <vector>
#include <unordered_map>

class ComponentMeshRenderer;

class Batch
{

	struct MeshData
	{
		uint refCount       = 0;
        uint baseVertex     = 0;
        uint vertexCount    = 0;
        uint baseIndex      = 0;
        uint indexCount     = 0;
	};

    struct MaterialData
    {
        uint refCount = 0;
        uint offset   = 0;
        TextureBatch::Handle handle[TextureCount];
    };

    struct DrawCommand
    {
        uint count;
        uint instanceCount;
        uint firstIndex;
        uint baseVertex;
        uint baseInstance;
    }; 

    typedef std::unordered_map<UID, MeshData>       MeshList;
    typedef std::unordered_map<UID, MaterialData>   MaterialList;
    typedef std::vector<DrawCommand>                CommandList;
    typedef std::vector<ComponentMeshRenderer*>     ObjectList;
    typedef std::vector<uint>                       ObjectHandleList;

    uint                            texture_size[TextureCount][2];

    uint                            vertex_size  = 0;
    uint                            attrib_flags = 0;
    uint                            attrib_count = 0;
    VertexAttrib                    attribs[ATTRIB_COUNT+1];

    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         ibo;
    std::unique_ptr<Buffer>         vbo;
    std::unique_ptr<Buffer>         drawIdVBO;
    std::unique_ptr<Buffer>         transformSSBO;
    std::unique_ptr<Buffer>         materialSSBO;
    std::unique_ptr<Buffer>         commandBuffer;
    uint                            commandBufferSize = 0;

    MeshList                        meshes;
    MaterialList                    materials;
    TextureBatch                    textures;

    ObjectHandleList                objectHandles;
    ObjectList                      objects;
    uint                            firstFreeObject;

    CommandList                     commands;

    HashString                      tagName;
    bool                            bufferDirty = false;

public:

    explicit Batch(const HashString& tag);
    ~Batch() = default;
   
    bool               CanAdd            (ComponentMeshRenderer* object) const;
    uint               Add               (ComponentMeshRenderer* object);
    void               Remove            (uint index);

    void               AddToRender       (uint index);
    void               DoRender          (uint transformsIndex, uint materialsIndex, uint texturesLocation);

    bool               IsEmpty           () const { return objects.empty(); }
    const HashString&  GetTagName        () const { return tagName; }

    void               CreateRenderData  ();
private:

    void ClearRenderData      ();
    void Init                 (ComponentMeshRenderer* object);

    void CreateVertexBuffers  ();
    void CreateDrawIdBuffer   ();
    void CreateMaterialBuffer ();
    void CreateTransformBuffer();
    void CreateCommandBuffer();
};

#endif /* _BATCH_H_ */
