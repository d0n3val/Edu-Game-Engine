#ifndef __COMPONENT_MATERIAL_H__
#define __COMPONENT_MATERIAL_H__

#include "Component.h"

struct TextureInfo;
class ResourceMaterial;

class ComponentMaterial : public Component
{
public:

	ComponentMaterial (GameObject* container);
	~ComponentMaterial ();

	bool                    SetResource (UID uid);
    const ResourceMaterial* GetResource () const;
    ResourceMaterial*       GetResource ();

    bool                    CastShadows () const { return cast_shadows; }
    bool                    RecvShadows () const { return recv_shadows; }

	void                    OnSave      (Config& config) const override;
	void                    OnLoad      (Config* config) override;

    static Types            GetClassType() { return Material; }

private:

    UID   resource     = 0;
    bool cast_shadows  = true;
    bool recv_shadows  = true;
};

#endif // __COMPONENT_MESH_H__
