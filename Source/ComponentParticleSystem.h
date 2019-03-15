#ifndef __COMPONENT_PARTICLE_SYSTEM_H__
#define __COMPONENT_PARTICLE_SYSTEM_H__

#include "Component.h"
#include "Billboard.h"
#include <vector>

class ComponentParticleSystem : public Component
{
public:

	ComponentParticleSystem  (GameObject* container);
	~ComponentParticleSystem ();

	void         OnPlay      () override;
	void         OnStop      () override;
	void         OnUpdate    (float dt) override;
    void         PreRender   ();

	void         OnSave      (Config& config) const override;
	void         OnLoad      (Config* config) override;

    static Types GetClassType() { return ParticleSystem; }

private:

    void        UpdateBuffers();
    void        UpdateBillboards();

private:

    uint vao               = 0;
    uint vbo               = 0;
    uint vb_num_quads      = 0;
    uint vb_min_num_quads  = 32;
    const uint vertex_size = sizeof(float3);

    UID material;

    struct TextureSheet
    {
        uint x_tiles = 0;
        uint y_tiles = 0;
    };

    std::vector<Billboard> billboards; // \todo: live/dead pattern
};

#endif /* __COMPONENT_PARTICLE_SYSTEM_H__ */

