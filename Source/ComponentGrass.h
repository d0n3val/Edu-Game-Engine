#ifndef __COMPONENT_GRASS_H__
#define __COMPONENT_GRASS_H__

#include "Component.h"

class ResourceTexture;

class ComponentGrass : public Component
{
public:
    ComponentGrass(GameObject* object);
    ~ComponentGrass();

	virtual void                OnSave          (Config& config) const override;
	virtual void                OnLoad          (Config* config) override;

	bool                        SetAlbedo       (UID uid);
    const ResourceTexture*      GetAlbedo       () const;
   
	bool                        SetNormal       (UID uid);
    const ResourceTexture*      GetNormal       () const;

    static Types                GetClassType    () { return Grass; }

private:

    friend void DrawGrassComponent(ComponentGrass* grass);

private:

    UID albedo = 0;
    UID normal = 0;
};

#endif /* __COMPONENT_GRASS_H__ */


