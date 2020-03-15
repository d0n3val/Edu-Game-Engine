#ifndef __COMPONENT_GRASS_H__
#define __COMPONENT_GRASS_H__

#include "Component.h"
#include "OGL.h"

#include "ResHandle.h"

class ResourceTexture;

class ComponentGrass : public Component
{
public:
    ComponentGrass(GameObject* object);
    ~ComponentGrass() = default;

	virtual void                OnSave          (Config& config) const override;
	virtual void                OnLoad          (Config* config) override;

	void                        SetAlbedo       (UID uid){ albedo = uid; }
    const ResourceTexture*      GetAlbedo       () const {return albedo.GetPtr<ResourceTexture>(); }
   
	void                        SetNormal       (UID uid){ normal = uid; }
    const ResourceTexture*      GetNormal       () const {return normal.GetPtr<ResourceTexture>(); }

    static Types                GetClassType    () { return Grass; }

    void                        Draw            ();

private:

    friend void DrawGrassComponent(ComponentGrass* grass);

    void BindMaterial();

private:

    ResHandle albedo;
    ResHandle normal;

    std::unique_ptr<Buffer>      billboard_vbo;
    std::unique_ptr<Buffer>      billboard_ibo;
    std::unique_ptr<VertexArray> billboard_vao;
};

#endif /* __COMPONENT_GRASS_H__ */


