#include "Globals.h"

#include "ComponentParticleSystem.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "GameObject.h"
#include "ResourceTexture.h"

#include "OpenGL.h"

#include "mmgr/mmgr.h"

#define TEXTURE_MAP_LOC 0
#define SHEET_LOC 10

ComponentParticleSystem::ComponentParticleSystem(GameObject* container) : Component(container, Types::ParticleSystem)
{
}

ComponentParticleSystem::~ComponentParticleSystem()
{
    if(vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if(ibo != 0)
    {
        glDeleteBuffers(1, &ibo);
        ibo = 0;
    }

}

void ComponentParticleSystem::OnSave(Config& config) const 
{
	config.AddUID("Texture", texture);

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
    SetTexture(config->GetUID("Texture", 0));

    uint count = config->GetArrayCount("Resources");

    billboards.resize(count);

    for(uint i=0; i < count; ++i)
    {
        Config billboard(config->GetArray("Resources", i));

        billboards[i].SetPosition(billboard.GetFloat3("position"));
        billboards[i].SetSize(billboard.GetFloat2("size"));
    }

    if(count == 0) // \hack
    {
        billboards.resize(1);
        billboards[0].SetPosition(float3::zero);
        billboards[0].SetSize(float2(1, 1));
    }
}

void ComponentParticleSystem::OnPlay() 
{
}

void ComponentParticleSystem::OnStop() 
{
    sheet_animation.current = 0.0f;
}

void ComponentParticleSystem::OnUpdate(float dt) 
{
    // Update particle positions
    
    const float speed = 20.0f; // fps; 

    sheet_animation.current = fmodf(sheet_animation.current+speed*dt, float(sheet_animation.x_tiles*sheet_animation.y_tiles));
    //sheet_animation.current = min(sheet_animation.current+speed*dt, float(sheet_animation.x_tiles*sheet_animation.y_tiles-1));
}

void ComponentParticleSystem::UpdateBuffers()
{
    if(vb_num_quads < billboards.size())
    {
        vb_num_quads = billboards.size();

        if(vbo == 0)
        {
            glGenBuffers(1, &vbo);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertex_size*vb_num_quads*4, nullptr, GL_DYNAMIC_DRAW);

        float2* uv = (float2*)glMapBufferRange(GL_ARRAY_BUFFER, vb_num_quads*4*sizeof(float3), vb_num_quads*4*sizeof(float2), GL_MAP_WRITE_BIT);

        for(uint i=0; i< vb_num_quads; ++i)
        {
            uv[i*4+0] = float2(0, 0); 
            uv[i*4+1] = float2(1, 0);
            uv[i*4+2] = float2(1, 1);
            uv[i*4+3] = float2(0, 1);
        }

		glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        if(ibo == 0)
        {
            glGenBuffers(1, &ibo);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, vb_num_quads*6*sizeof(unsigned), nullptr, GL_STATIC_DRAW);
        unsigned* indices = (unsigned*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, vb_num_quads*6, GL_MAP_WRITE_BIT);

        for(uint i=0; i< vb_num_quads; ++i)
        {
            indices[i*6+0] = i*4+0;
            indices[i*6+1] = i*4+1;
            indices[i*6+2] = i*4+2;
            indices[i*6+3] = i*4+0;
            indices[i*6+4] = i*4+2;
            indices[i*6+5] = i*4+3;
        }

		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    if(vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float3), (void*)0);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float2), (void*)(vb_num_quads*4*sizeof(float3)));

        glBindVertexArray(0);
    }
}


void ComponentParticleSystem::UpdateBillboards()
{
    if(!billboards.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        float3* vertices = (float3*)glMapBufferRange(GL_ARRAY_BUFFER, 0, vertex_size*vb_num_quads*4, GL_MAP_WRITE_BIT);

        // todo: sort front to back
        Billboard::GetVertices(&billboards[0], billboards.size(), vertices, App->renderer3D->active_camera);

        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void ComponentParticleSystem::Draw()
{
    UpdateBuffers();
    UpdateBillboards();

    const ResourceTexture* tex_res = static_cast<const ResourceTexture*>(App->resources->Get(texture));
    
    if(tex_res)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_res->GetID());
        glUniform1i(TEXTURE_MAP_LOC, 0);
    }

    float4x4 transform = GetGameObject()->GetGlobalTransformation();
    glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

    glUniform1i(SHEET_LOC, sheet_animation.x_tiles);
    glUniform1i(SHEET_LOC+1, sheet_animation.y_tiles);
    glUniform1f(SHEET_LOC+2, sheet_animation.current);

    glBindVertexArray(vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, vb_num_quads*6, GL_UNSIGNED_INT, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


const ResourceTexture* ComponentParticleSystem::GetTextureRes() const
{
	return static_cast<const ResourceTexture*>(App->resources->Get(texture));
}

ResourceTexture* ComponentParticleSystem::GetTextureRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(texture));
}

void ComponentParticleSystem::SetTexture(UID uid)
{
    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory() == true)
        {
            texture = uid;
        }
    }
}
