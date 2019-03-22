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
    glGenBuffers(1, &render_buffers.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, render_buffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    static const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(1, &render_buffers.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if(render_buffers.vao == 0)
    {
        glGenVertexArrays(1, &render_buffers.vao);
    }

    glBindVertexArray(render_buffers.vao);

    glBindBuffer(GL_ARRAY_BUFFER, render_buffers.vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)(4*sizeof(float)*3));

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4+sizeof(float)*4, (void*)0);
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4+sizeof(float)*4, (void*)(sizeof(float)*3));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4+sizeof(float)*4, (void*)(2*sizeof(float)*3));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3*4+sizeof(float)*4, (void*)(3*sizeof(float)*3));
    glVertexAttribDivisor(6, 1);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(float)*3*4+sizeof(float)*4, (void*)(4*sizeof(float)*3));
    glVertexAttribDivisor(7, 1);

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

    if(render_buffers.vbo != 0)
    {
        glDeleteBuffers(1, &render_buffers.vbo);
        render_buffers.vbo = 0;
    }

    if(render_buffers.ibo != 0)
    {
        glDeleteBuffers(1, &render_buffers.ibo);
        render_buffers.ibo = 0;
    }

    if(render_buffers.vao != 0)
    {
        glDeleteVertexArrays(1, &render_buffers.vao);
        render_buffers.vao = 0;
    }

}

void ComponentParticleSystem::OnSave(Config& config) const 
{
	config.AddFloat("Duration", init.duration);
	config.AddUInt("Max particles", init.max_particles);
	config.AddBool("Loop", init.loop);
	config.AddFloat("Life", init.life);
	config.AddFloat("Speed", init.speed);
	config.AddFloat("Size", init.size);
	config.AddFloat("Whole speed", init.whole_speed);

    config.AddUInt("Particles per second", emissor.particles_per_second);
    config.AddUInt("Particles per distance", emissor.particles_per_distance);
    config.AddUInt("Shape type", (uint)shape.type);
    if(shape.type == Circle)
    {
        config.AddFloat("Circle radius", shape.radius);
    }
    else if(shape.type == Cone)
    {
        config.AddFloat("Cone radius", shape.radius);
        config.AddFloat("Cone angle", shape.angle);
    }

    config.AddFloat3("Speed init", speed_over_time.init);
    config.AddFloat3("Speed end", speed_over_time.end);

    config.AddFloat("Size init", size_over_time.init);
    config.AddFloat("Size end", size_over_time.end);

    config.AddFloat4("Color init", color_over_time.init);
    config.AddFloat4("Color end", color_over_time.end);

	config.AddUID("Texture", texture_info.texture);
    config.AddInt("Sheet x", texture_info.x_tiles);
    config.AddInt("Sheet y", texture_info.y_tiles);
    config.AddFloat("Sheet speed", texture_info.speed);
}

void ComponentParticleSystem::OnLoad(Config* config) 
{
	init.duration = config->GetFloat("Duration", 0.0f);
	init.max_particles = config->GetUInt("Max particles", 100);
    init.loop = config->GetBool("Loop", false);
    init.life = config->GetFloat("Life", 0.0f);
	init.speed = config->GetFloat("Speed", 0.0f);
	init.size = config->GetFloat("Size", 0.0f);
	init.whole_speed = config->GetFloat("Whole speed", 1.0f);

    emissor.particles_per_second = config->GetUInt("Particles per second", 0);
    emissor.particles_per_distance = config->GetUInt("Particles per distance", 0);
    shape.type = (ShapeType)config->GetUInt("Shape type", (uint)Circle);
    if(shape.type == Circle)
    {
        shape.radius = config->GetFloat("Circle radius", 1.0f);
    }
    else if(shape.type == Cone)
    {
        shape.radius = config->GetFloat("Cone radius", 1.0f);
        shape.angle = config->GetFloat("Cone angle", 0.0f);
    }


    speed_over_time.init = config->GetFloat3("Speed init", float3::zero);
    speed_over_time.end = config->GetFloat3("Speed end", float3::zero);

    size_over_time.init = config->GetFloat("Size init", 1.0f);
    size_over_time.end = config->GetFloat("Size end", 1.0f);

    color_over_time.init = config->GetFloat4("Color init", float4::one);
    color_over_time.end = config->GetFloat4("Color end", float4::one);

    SetTexture(config->GetUID("Texture", 0));

    texture_info.x_tiles = config->GetInt("Sheet x", 1);
    texture_info.y_tiles = config->GetInt("Sheet y", 1);
    texture_info.speed   = config->GetFloat("Sheet speed", 24.0);
}

void ComponentParticleSystem::OnPlay() 
{
    particles.resize(init.max_particles);
    last_used_particle = 0;
    elapsed_emission = 0.0f;
    alive_particles = 0;
}

void ComponentParticleSystem::OnStop() 
{
    texture_info.current = 0.0f;
    alive_particles = 0;
    for(uint i=0; i< particles.size(); ++i)
    {
        particles[i].life = 0.0f;
    }
}

void ComponentParticleSystem::OnUpdate(float dt) 
{
    dt = dt*init.whole_speed;
    alive_particles = 0;

    for(uint i=0; i< particles.size(); ++i)
    {
		Particle& particle = particles[i];
        if((particle.life = max(particle.life-dt, 0.0f)) > 0.0f)
        {
            float lambda = particle.life/init.life;
            float3 speed = particle.speed+speed_over_time.Interpolate(lambda*lambda);
            particles[i].transform.SetTranslatePart(particles[i].transform.TranslatePart()+speed*dt);
            particles[i].size = init.size*size_over_time.Interpolate(lambda*lambda);
            particles[i].color = color_over_time.Interpolate(lambda*lambda);

            ++alive_particles;
        }
    }

	if (emissor.particles_per_second > 0)
	{
		elapsed_emission += dt;
		float emission_period = 1.0f / float(emissor.particles_per_second);
		while (alive_particles < init.max_particles && (elapsed_emission-emission_period) > 0.0f)
		{
			AddNewParticle();
			elapsed_emission -= emission_period;
		}
	}

    texture_info.current = fmodf(texture_info.current+texture_info.speed*dt, float(texture_info.x_tiles*texture_info.y_tiles));
}

void ComponentParticleSystem::UpdateInstanceBuffer()
{
    unsigned stride = sizeof(float3)*4+sizeof(float4);

    if(num_instances < alive_particles)
    {
        num_instances = alive_particles;
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, stride*num_instances, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ComponentCamera* camera = App->renderer3D->active_camera;

    glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
    char* instance_data = (char*)glMapBufferRange(GL_ARRAY_BUFFER, 0, stride*num_instances, GL_MAP_WRITE_BIT);
	
    uint index = 0;
    for(uint i=0; i< particles.size(); ++i)
    {
        if(particles[i].life > 0.0f)
        {
            float3 camera_pos = camera->GetViewMatrix().RotatePart().Transposed().Transform(-camera->GetViewMatrix().TranslatePart());

            float3 translation = particles[i].transform.TranslatePart();
            float3 z_axis = camera_pos-translation; z_axis.Normalize();
            float3 y_axis = float3::unitY;
            float3 x_axis = y_axis.Cross(z_axis); x_axis.Normalize();

            x_axis *= particles[i].size;
            y_axis *= particles[i].size;
            z_axis *= particles[i].size;

            particles[i].transform.SetCol3(0, x_axis);
            particles[i].transform.SetCol3(1, y_axis);
            particles[i].transform.SetCol3(2, z_axis);

            float3* matrix = (float3*)(instance_data+(index*stride));

			matrix[0] = x_axis;
			matrix[1] = y_axis;
			matrix[2] = z_axis;
			matrix[3] = translation;

            float4* color = (float4*)(instance_data+(index*stride+sizeof(float3)*4));
            *color = particles[i].color;

            ++index;
        }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ComponentParticleSystem::Draw()
{
    if(alive_particles > 0)
    {
        UpdateInstanceBuffer();

        const ResourceTexture* tex_res = static_cast<const ResourceTexture*>(App->resources->Get(texture_info.texture));

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

        glUniform1i(SHEET_LOC, texture_info.x_tiles);
        glUniform1i(SHEET_LOC+1, texture_info.y_tiles);
        glUniform1f(SHEET_LOC+2, texture_info.current);

        glBindVertexArray(render_buffers.vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, alive_particles); 
        //glDrawElementsInstanced(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, nullptr, alive_particles); 
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
	return static_cast<const ResourceTexture*>(App->resources->Get(texture_info.texture));
}

ResourceTexture* ComponentParticleSystem::GetTextureRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(texture_info.texture));
}

void ComponentParticleSystem::SetTexture(UID uid)
{
    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory() == true)
        {
            texture_info.texture = uid;
        }
    }
}

void ComponentParticleSystem::AddNewParticle()
{
    bool found = false;
    for(uint i=last_used_particle; !found && i< particles.size(); ++i)
    {
        if((found = particles[i].life == 0.0f) == true)
        {
            last_used_particle = i;
        }
    }

    for(uint i=0; !found && i< last_used_particle; ++i)
    {
        if((found = particles[i].life == 0.0f) == true)
        {
            last_used_particle = i;
        }
    }

    if(found)
    {
        if(shape.type == Circle)
        {
            float angle = App->random->Float01Incl()*2*PI;
            float len = App->random->Float01Incl()*shape.radius;
            float3 direction = float3(cos(angle), 0.0f, sin(angle));
            particles[last_used_particle].transform.SetTranslatePart(direction*len);
            particles[last_used_particle].speed = direction*init.speed;
        }
        else if(shape.type == Cone)
        {
            float angle = App->random->Float01Incl()*2*PI;
            float len = App->random->Float01Incl()*shape.radius;
			float3 direction = float3(cos(angle), 0.0f, sin(angle));
			float3 position  = direction*len;
            float3 speed_dir = float3(0.0f, 1.0f, 0.0f);

            if(shape.angle > 0.0001f)
            {
				if (shape.radius > 0.0001f)
				{
					float height = shape.radius / tan(shape.angle);
					speed_dir = position - float3(0.0f, -height, 0.0f);
					speed_dir.Normalize();
				}
				else
				{
					float height = 1.0f / tan(shape.angle);
					speed_dir = direction + float3(0.0f, height, 0.0f);
					speed_dir.Normalize();
				}
            }

            particles[last_used_particle].transform.SetTranslatePart(position);
            particles[last_used_particle].speed = speed_dir*init.speed;
        }

        particles[last_used_particle].size  = init.size;
        particles[last_used_particle].life  = init.life;
        particles[last_used_particle].color = float4::one;
        ++alive_particles;
    }
}

float ComponentParticleSystem::RandomValue::GetValue() const
{
    return init +(end-init)*App->random->Float01Incl();
}