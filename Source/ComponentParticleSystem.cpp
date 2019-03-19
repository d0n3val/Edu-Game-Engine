#include "Globals.h"

#include "ComponentParticleSystem.h"
#include "ComponentCamera.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "GameObject.h"
#include "ResourceTexture.h"

#include "OpenGL.h"

#include <algorithm>

#include "mmgr/mmgr.h"

#define TEXTURE_MAP_LOC 0
#define SHEET_LOC 10

ComponentParticleSystem::ComponentParticleSystem(GameObject* container) : Component(container, Types::ParticleSystem)
{
    static const float vertices[] = { -0.5f, -0.5f, 0.0f , 0.5f, -0.5f, 0.0f , 0.5f, 0.5f, 0.0f , -0.5f, 0.5f, 0.0f, 
                                        0.0f,  0.0f,        1.0f,  0.0f,        1.0f, 1.0f,        0.0f, 1.0f };
    glGenBuffers(1, &instance_vbo);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    static const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if(vao == 0)
    {
        glGenVertexArrays(1, &vao);
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)(4*sizeof(float)*3));

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4, (void*)0);
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4, (void*)(sizeof(float)*3));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4, (void*)(2*sizeof(float)*3));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4, (void*)(3*sizeof(float)*3));
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ComponentParticleSystem::~ComponentParticleSystem()
{
    if(instance_vbo != 0)
    {
        glDeleteBuffers(1, &instance_vbo);
        instance_vbo = 0;
    }

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

    if(vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

}

void ComponentParticleSystem::OnSave(Config& config) const 
{
	config.AddUID("Texture", texture);

	config.AddArray("Particles");

	for (uint i = 0; i < particles.size(); ++i)
	{
		Config particle;
		particle.AddFloat3("position", particles[i].transform.TranslatePart());
		particle.AddFloat("size", particles[i].size);

        config.AddArrayEntry(particle);
	}

    config.AddInt("Sheet x", sheet_animation.x_tiles);
    config.AddInt("Sheet y", sheet_animation.y_tiles);
    config.AddFloat("Sheet speed", sheet_animation.speed);
}

void ComponentParticleSystem::OnLoad(Config* config) 
{
    SetTexture(config->GetUID("Texture", 0));

    uint count = config->GetArrayCount("Particles");

    particles.resize(count);

    for(uint i=0; i < count; ++i)
    {
        Config particle(config->GetArray("Particles", i));

        particles[i].transform.SetTranslatePart(particle.GetFloat3("position"));
        particles[i].size = particle.GetFloat("size");
    }

    sheet_animation.x_tiles = config->GetInt("Sheet x", 1);
    sheet_animation.y_tiles = config->GetInt("Sheet y", 1);
    sheet_animation.speed   = config->GetFloat("Sheet speed", 24.0);
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
    sheet_animation.current = fmodf(sheet_animation.current+sheet_animation.speed*dt, float(sheet_animation.x_tiles*sheet_animation.y_tiles));
}

void ComponentParticleSystem::UpdateInstanceBuffer()
{
    if(num_instances < particles.size())
    {
        num_instances = particles.size();
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*3*num_instances, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void ComponentParticleSystem::UpdateParticles()
{
    ComponentCamera* camera = App->renderer3D->active_camera;

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    float3* matrices = (float3*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(float3)*4*num_instances, GL_MAP_WRITE_BIT);

    for(uint i=0; i< particles.size(); ++i)
    {
        float3 camera_pos = camera->GetViewMatrix().RotatePart().Transposed().Transform(-camera->GetViewMatrix().TranslatePart());

        float3 translation = particles[i].transform.TranslatePart();
        float3 z_axis = camera_pos-translation; z_axis.Normalize();
        float3 y_axis = float3::unitY;
        float3 x_axis = y_axis.Cross(z_axis); x_axis.Normalize();

        particles[i].transform.SetCol3(0, x_axis);
        particles[i].transform.SetCol3(1, y_axis);
        particles[i].transform.SetCol3(2, z_axis);

		matrices[i * 4 + 0] = x_axis;
		matrices[i * 4 + 1] = y_axis;
		matrices[i * 4 + 2] = z_axis;
		matrices[i * 4 + 3] = translation;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ComponentParticleSystem::Draw()
{
    UpdateInstanceBuffer();
    UpdateParticles(); 

    if(!particles.empty())
    {
        const ResourceTexture* tex_res = static_cast<const ResourceTexture*>(App->resources->Get(texture));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(GL_FALSE);

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
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, num_instances); 
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
    }
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

void ComponentParticleSystem::AddParticle()
{
    particles.push_back(TParticle());
    particles.back().transform.SetTranslatePart(float3(App->random->Float01Incl()*0.5f-0.25f, 0.5f, App->random->Float01Incl()*0.5f-0.25f));
    particles.back().size = 1.0f;
}
