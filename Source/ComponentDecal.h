#ifndef __COMPONENT_DECAL_H__
#define __COMPONENT_DECAL_H__

#include "Component.h"

class ResourceTexture;

class ComponentDecal : public Component
{
public:
    ComponentDecal(GameObject* go);
    ~ComponentDecal();

	void OnSave (Config& config) const override;
	void OnLoad (Config* config) override;

    static Types GetClassType () { return Decal; }

    UID GetAlbedo() const { return albedo; }
    UID GetNormal() const { return normal; }
    UID GetEmissive() const { return emissive; }

    void SetAlbedo(UID uid);
    void SetNormal(UID uid);
    void SetEmissive(UID uid);

    ResourceTexture* GetAlbedoRes();
    ResourceTexture* GetNormalRes();
    ResourceTexture* GetEmissiveRes();

private:

    UID LoadTexToMemory(UID uid);

private:
    UID albedo;
    UID normal;
    UID emissive;
};


#endif /* __COMPONENT_DECAL_H__ */