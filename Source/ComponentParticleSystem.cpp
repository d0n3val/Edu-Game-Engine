#include "Globals.h"
#include "ParticleSystem.h"


ComponentParticleSystem::ComponentParticleSystem(GameObject* container) : Component(container, Types::ParticleSystem)
{
}

ComponentParticleSystem::~ComponentParticleSystem()
{
}

void ComponentParticleSystem::OnSave(Config& config) const 
{
	config.AddUID("Resource", texture);

}

void ComponentParticleSystem::OnLoad(Config* config) 
{
    texture = config->GetUID("texture", 0);
}

void ComponentParticleSystem::OnPlay() 
{
}

void ComponentParticleSystem::OnStop() 
{
}

void ComponentParticleSystem::OnUpdate(float dt) 
{
    // Update particle positions
}

void ComponentParticleSystem::PreRender()
{
    UpdateBuffers();
    UdpateBillboards();
}

void ComponentParticleSystem::UpdateBuffers()
{
    if(vb_num_quads < billboards.size())
    {
        vb_num_quads = max(billboards.size(), vb_min_num_quads);

        if(vbo != 0)
        {
            glGenBuffers(1, &vbo);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertex_size*vb_num_quads*4, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}


