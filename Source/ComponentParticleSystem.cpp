#include "Globals.h"
#include "ComponentParticleSystem.h"

#include "OpenGL.h"

ComponentParticleSystem::ComponentParticleSystem(GameObject* container) : Component(container, Types::ParticleSystem)
{
}

ComponentParticleSystem::~ComponentParticleSystem()
{
}

void ComponentParticleSystem::OnSave(Config& config) const 
{
	config.AddUID("Material", material);
	config.AddUInt("Min_Num_Quads", vb_min_num_quads);

	config.AddArrayEntry("Billboards");

	for (uint i = 0; i < billboards.size(); ++i)
	{
		Config billboard;
		billboard.AddFloat3("position", billboards[i].GetPosition());
		billboard.AddFloat2("size", billboards[i].GetSize());

        config.AddArrayEntry(billboard);
	}
}

void ComponentParticleSystem::OnLoad(Config* config) 
{
    material = config->GetUID("Material", 0);
	vb_min_num_quads = config->GetUInt("Min_Num_Quads", 32);

    uint count = config->GetArrayCount("Resources");

    billboards.resize(count);

    for(uint i=0; i < count; ++i)
    {
        Config billboard(config->GetArray("Resources", i));

        billboards[i].SetPosition(billboard.GetFloat3("position"));
        billboards[i].SetSize(billboard.GetFloat2("size"));
    }
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
    //UdpateBillboards();
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


