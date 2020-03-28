#ifndef __COMPONENT_GRASS_H__
#define __COMPONENT_GRASS_H__

#include "Component.h"

#include "OGL.h"
#include "ResHandle.h"

class ResourceMesh;
class ResourceMaterial;

class ComponentGrass : public Component
{
public:
    ComponentGrass(GameObject* object);
    ~ComponentGrass() = default;

	virtual void            OnSave          (Config& config) const override;
	virtual void            OnLoad          (Config* config) override;

	void                    SetMesh         (UID uid){ mesh = uid; }
    void                    SetMaterial     (UID uid) { material = uid; }

    const ResourceMesh*     GetMesh() const;
    ResourceMesh*           GetMesh();
    const ResourceMaterial* GetMaterial() const;
    ResourceMaterial*       GetMaterial();

   
    static Types  GetClassType    () { return Grass; }

    void          Draw            ();

private:

    ResHandle mesh;
    ResHandle material;

    std::unique_ptr<Buffer>      billboard_vbo;
    std::unique_ptr<Buffer>      billboard_ibo;
    std::unique_ptr<VertexArray> billboard_vao;
};

#endif /* __COMPONENT_GRASS_H__ */


