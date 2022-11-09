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
	void GetBoundingBox(AABB& box) const override;

    static Types GetClassType () { return Decal; }

    UID GetAlbedo() const { return albedo; }
    UID GetNormal() const { return normal; }
    UID GetSpecular() const { return specular; }

    float GetNormalStrength() const { return normalStrength; }
    void SetNormalStrength(float strength) { normalStrength = strength; }

    bool IsValid() const { return albedo != 0 && specular != 0 && normal != 0; }
    void SetAlbedo(UID uid);
    void SetNormal(UID uid);
    void SetSpecular(UID uid);

    ResourceTexture* GetAlbedoRes();
    ResourceTexture* GetNormalRes();
    ResourceTexture* GetSpecularRes();

private:

    UID LoadTexToMemory(UID uid);

private:
    UID albedo;
    UID normal;
    UID specular;
    float normalStrength = 1.0f;
};


#endif /* __COMPONENT_DECAL_H__ */