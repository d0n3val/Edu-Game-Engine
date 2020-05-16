#ifndef _BATCH_H_
#define _BATCH_H_

#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "OGL.h"
#include "HashString.h"

#include <memory>
#include <vector>

class ComponentMeshRenderer;

class Batch
{
    typedef std::vector<ComponentMeshRenderer*>  ObjectList;
    typedef std::vector<const ResourceMaterial*> MaterialList;
    typedef std::vector<uint>                    RenderList;

    uint                            max_objects  = 0;
    uint                            max_vertices = 0;
    uint                            num_vertices = 0;

    uint                            num_render_indices = 0;
    uint                            num_render_objects = 0;

    uint                            texture_size[TextureCount][2];

    uint                            vertex_size  = 0;
    uint                            attrib_flags = 0;
    uint                            attrib_count = 0;
    VertexAttrib                    attribs[ATTRIB_COUNT];

    std::unique_ptr<Texture2DArray> textures[TextureCount];
    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         ibo;
    std::unique_ptr<Buffer>         vbo;

    ObjectList                      objects;
    MaterialList                    unique_materials;
    RenderList                      render_objects;
    HashString                      tag_name;

public:

    Batch(const HashString& tag, uint _max_vertices, uint _max_objects);
    ~Batch() = default;
   
    bool               CanAdd       (ComponentMeshRenderer* object) const;
    uint               Add          (ComponentMeshRenderer* object);
    void               Remove       (uint index);

    void               AddToRender  (uint index);
    void               DoRender     ();

    bool               IsEmpty      () const { return objects.empty(); }
    const HashString&  GetTagName   () const { return tag_name; }

private:

    void CreateRenderData   ();
    void Init               (ComponentMeshRenderer* object);

    void CreateBuffers      ();
    void CreateTextureArray ();

};

#endif /* _BATCH_H_ */
