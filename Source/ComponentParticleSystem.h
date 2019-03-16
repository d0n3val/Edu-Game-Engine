#ifndef __COMPONENT_PARTICLE_SYSTEM_H__
#define __COMPONENT_PARTICLE_SYSTEM_H__

#include "Component.h"
#include "Billboard.h"
#include <vector>

class ResourceTexture;

class ComponentParticleSystem : public Component
{
public:

	ComponentParticleSystem  (GameObject* container);
	~ComponentParticleSystem ();

	void                    OnPlay          () override;
	void                    OnStop          () override;
	void                    OnUpdate        (float dt) override;
    void                    Draw            ();

	void                    OnSave          (Config& config) const override;
	void                    OnLoad          (Config* config) override;

    static Types            GetClassType    () { return ParticleSystem; }

    const ResourceTexture*  GetTextureRes   () const;
    ResourceTexture*        GetTextureRes   ();

    UID                     GetTexture      () const { return texture; }
    void                    SetTexture      (UID tex);

private:

    void        UpdateBuffers();
    void        UpdateBillboards();

private:

    struct TextureSheet
    {
        uint x_tiles = 6;
        uint y_tiles = 6;
        float current = 0.0f;
    };

    uint vao               = 0;
    uint vbo               = 0;
    uint ibo               = 0;
    uint vb_num_quads      = 0;
    const uint vertex_size = sizeof(float3)+sizeof(float2);

    UID texture = 0;

    TextureSheet sheet_animation;

    std::vector<Billboard> billboards; // \todo: live/dead pattern
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

