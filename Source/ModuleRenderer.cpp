#include "Globals.h"

#include "ModuleRenderer.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModuleHints.h"

#include "GameObject.h"

#include "ComponentGeometry.h"

#include "ResourceMesh.h"
#include "ResourceMaterial.h"

#include "Application.h"

#include "OpenGL.h"

unsigned ModuleRenderer::renderer_count = 0;
unsigned ModuleRenderer::uniforms[UNIFORM_COUNT];

ModuleRenderer::ModuleRenderer() : Module("renderer")
{
    LoadDefaultShaders();
    LoadShadowShaders();

    if(renderer_count++ == 0)
    {
        const unsigned uniform_size[] = { sizeof(float4x4) * 2, sizeof(float4x4) * 2 };

        // Bind uniforms blocks to buffers
        for(unsigned i=0; i < UNIFORM_COUNT; ++i)
        {
            glGenBuffers(1, &uniforms[i]);
            glBindBuffer(GL_UNIFORM_BUFFER, uniforms[i]);
            glBufferData(GL_UNIFORM_BUFFER, uniform_size[i], 0, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            glBindBufferBase(GL_UNIFORM_BUFFER, i, uniforms[i]);
        }
    }

    glGenFramebuffers(1, &color.fbo);
    glGenFramebuffers(1, &shadow.fbo);
}

ModuleRenderer::~ModuleRenderer()
{
    if(--renderer_count == 0)
    {
        glDeleteBuffers(UNIFORM_COUNT, uniforms);
    }

    glDeleteFramebuffers(1, &shadow.fbo);
    glDeleteFramebuffers(1, &color.fbo);
}

void ModuleRenderer::Draw(unsigned width, unsigned height)
{
    // \todo: culling
    CollectNodes();

    UpdateCameraUniform();
    UpdateLightUniform();

    ShadowPass(width, height);
    ColorPass(width, height);
}

void ModuleRenderer::CollectNodes()
{
	draw_nodes.clear();

	CollectNodesRec(App->level->GetRoot());
}

void ModuleRenderer::CollectNodesRec(GameObject* node)
{
	if(node->HasComponent(Component::Geometry))
	{
		draw_nodes.push_back(node);
	}

	for (std::list<GameObject*>::const_iterator it = node->childs.begin(), end = node->childs.end(); it != end; ++it)
	{
		CollectNodesRec(*it);
	}
}

/* \todo:
void ModuleRenderer::DrawSkybox()
{
    Scene::Skybox* skybox = Scene::GetService()->GetSkybox();

    if(skybox->vao != 0)
    {
        App->programs->UseProgram("skybox", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubemap);
        glUniform1i(App->programs->GetUniformLocation("cubemap"), 0);

        glBindVertexArray(skybox->vao);
        glDrawArrays(GL_TRIANGLES, 0, 6*6);
        glBindVertexArray(0);
    }
}
*/

void ModuleRenderer::DrawNodes(void (ModuleRenderer::*drawer)(const float4x4& transform, ResourceMesh* mesh))
{
	for(NodeList::iterator it = draw_nodes.begin(), end = draw_nodes.end(); it != end; ++it)
	{
		GameObject* node = *it;

        ComponentGeometry* geometry = static_cast<ComponentGeometry*>(node->FindFirstComponent(Component::Geometry));

		for (uint i=0, count = geometry->meshes.size(); i < count; ++i)
		{
            (this->*drawer)(node->GetGlobalTransformation(), static_cast<ResourceMesh*>(App->resources->Get(geometry->meshes[i])));
        }

	}
}

void ModuleRenderer::DrawMeshColor(const float4x4& transform, const ResourceMesh* mesh)
{    
	const ResourceMaterial* material = static_cast<const ResourceMaterial*>(App->resources->Get(mesh->mat_id));
    const ComponentLight* light = App->level->GetActiveLight();

    unsigned variation = PIXEL_LIGHTING;

	if(mesh->attribs & ResourceMesh::ATTRIB_BONES)
	{
        variation |= SKINNING;
	}

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_NORMAL_MAPPING) && mesh->attribs & ResourceMesh::ATTRIB_TANGENTS && material->normal_map != 0)
    {
        variation |= NORMAL_MAP;
    }

    if(App->hints->GetBoolValue(ModuleHints::ENABLE_SPECULAR_MAPPING) && material->specular_map != 0)
    {
        variation |= SPECULAR_MAP;
    }

    if(light != nullptr && light->type == ComponentLight::LIGHT_DIRECTIONAL)
    {
        variation |= LIGHT_DIRECTIONAL;
    }

    if(material->recv_shadows)
    {
        variation |= RECEIVE_SHADOWS;
    }

	App->programs->UseProgram("default", variation);

    glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_FALSE, reinterpret_cast<const float*>(&transform));
    glUniform1f(App->programs->GetUniformLocation("shininess"), material->shininess);

    glUniform1f(App->programs->GetUniformLocation("shadow_bias"), hints->GetFloatValue(Hint::SHADOW_BIAS));

    glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadow.tex);
	glUniform1i(App->programs->GetUniformLocation("shadow_map"), 3);

    glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, App->res->Get(material->specular_map)->gpu_id);
	glUniform1i(App->programs->GetUniformLocation("specular_map"), 2);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, App->res->Get(material->normal_map)->gpu_id);
	glUniform1i(App->programs->GetUniformLocation("normal_map"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, App->res->Get(material->albedo_map)->gpu_id);
	glUniform1i(App->programs->GetUniformLocation("diffuse"), 0);

	if((mesh->attribs & Scene::ATTRIB_BONES))
	{
		glUniformMatrix4fv(App->programs->GetUniformLocation("palette"), mesh->num_bones, GL_FALSE, reinterpret_cast<const float*>(mesh->palette));
	}

    glBindVertexArray(mesh->vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void ModuleRenderer::DrawMeshShadow(const float4x4& transform, ComponentMesh* mesh)
{
	Scene* scene = Scene::GetService();

	if (scene->GetMaterial(mesh->material)->cast_shadows)
	{
		unsigned variation = 0;
		if ((mesh->attribs & Scene::ATTRIB_BONES))
		{
			variation |= SKINNING;
		}

		App->programs->UseProgram("shadow", variation);

		if ((variation & SKINNING))
		{
			glUniformMatrix4fv(App->programs->GetUniformLocation("palette"), mesh->num_bones, GL_FALSE, reinterpret_cast<const float*>(mesh->palette));
		}

		glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_FALSE, reinterpret_cast<const float*>(&transform));

		glBindVertexArray(mesh->vao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
		glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, nullptr);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void ModuleRenderer::DebugDrawTangentSpace(float size)
{
	Scene* scene = Scene::GetService();

    for(NodeList::const_iterator it_node = draw_nodes.begin(), node_end = draw_nodes.end(); it_node != node_end; ++it_node)
    {
        const Scene::Node* node = *it_node;

        for(Scene::MeshInstList::const_iterator it_mesh = node->meshes.begin(), end_mesh = node->meshes.end(); it_mesh != end_mesh; ++it_mesh)
        {
            if(it_mesh->visible)
            {
                const Scene::Mesh* mesh = scene->GetMesh(it_mesh->index);

                if((mesh->attribs & Scene::ATTRIB_TANGENTS) != 0 && (mesh->attribs& Scene::ATTRIB_NORMALS))
                {
                    for(unsigned i = 0, count = mesh->num_vertices; i < count; ++i)
                    {
                        float4 position  = mul(node->global, float4(mesh->src_vertices[i], 1.0));
						float4 normal    = mul(node->global, float4(mesh->src_normals[i], 0.0));
						float4 tangent   = mul(node->global, float4(mesh->src_tangents[i], 0.0));
						float4 bitangent = float4(cross(normal.xyz(), tangent.xyz()), 0.0);

						float4x4 tbn(tangent, bitangent, normal, position);
                        
                        dd::axisTriad(tbn, size*0.1f, size, 0);
                    }
                }
            }
        }
    }
}

void ModuleRenderer::ColorPass(unsigned width, unsigned height)
{
    GenerateFBOTexture(color, width, height, true);

    glBindFramebuffer(GL_FRAMEBUFFER, color.fbo);

	glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawNodes(&ModuleRenderer::DrawMeshColor);
    //\todo: DrawSkybox();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    App->programs->UnuseProgram();
}

void ModuleRenderer::ShadowPass(unsigned width, unsigned height)
{
	Hints* hints = Hints::GetService();

    if(hints->GetBoolValue(Hint::ENABLE_SHADOW_FRONT_CULLING))
    {
        glCullFace(GL_FRONT);
    }
    
    float shadow_res = hints->GetFloatValue(Hint::SHADOW_RESOLUTION);

    width = unsigned(width*shadow_res);
    height = unsigned(height*shadow_res);

	glViewport(0, 0, width, height);	

    GenerateFBOTexture(shadow, width, height, false);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);

	glClear(GL_DEPTH_BUFFER_BIT);

    App->programs->UseProgram("shadow", 0);

	DrawNodes(&ModuleRenderer::DrawMeshShadow);

	App->programs->UnuseProgram();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(hints->GetBoolValue(Hint::ENABLE_SHADOW_FRONT_CULLING))
    {
        glCullFace(GL_BACK);
    }
}

void ModuleRenderer::GenerateFBOTexture(R2TInfo& info, unsigned width, unsigned height, bool aColor)
{
    if(width != info.size.x || height != info.size.y)
    {
        if(info.tex != 0)
        {
            glDeleteTextures(1, &info.tex);
        }

        if(width != 0 && height != 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, info.fbo);
            glGenTextures(1, &info.tex);
            glBindTexture(GL_TEXTURE_2D, info.tex);

			if (aColor)
			{
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			}

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            if(aColor)
            {
                glGenRenderbuffers(1, &info.depth);
                glBindRenderbuffer(GL_RENDERBUFFER, info.depth);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, info.depth);            
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, aColor ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, info.tex, 0);

            glDrawBuffer(aColor ? GL_COLOR_ATTACHMENT0 : GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

		info.size.x  = width;
		info.size.y = height;
    }
}

void ModuleRenderer::LoadDefaultShaders()
{
    typedef std::pair<char*, unsigned> PairUniform;

    const PairUniform binding[]   = { { "camera", UNIFORM_CAMERA } , { "light", UNIFORM_LIGHT } };

    const char* macros[]		  = { "#define SKINNING 1 \n", "#define PIXEL_LIGHTING 1 \n" , "#define NORMAL_MAP 1 \n",
                                      "#define SPECULAR_MAP 1 \n", "#define LIGHT_DIRECTIONAL 1 \n", "#define RECEIVE_SHADOWS 1 \n" };

    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);

    App->programs->Load("default", "data/default.vs", "data/default.fs", macros, num_macros, binding, UNIFORM_COUNT);
    App->programs->Load("skybox", "data/skybox.vs", "data/skybox.fs", 0, 0, binding, 1);

}

void ModuleRenderer::LoadShadowShaders()
{
    typedef std::pair<char*, unsigned> PairUniform;

    const char* macros[]          = { "#define SKINNING 1 \n" };
    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);
    const PairUniform binding[]   = { { "camera", UNIFORM_CAMERA } , { "light", UNIFORM_LIGHT } };

	App->programs->Load("shadow", "data/shadow.vs", "data/shadow.fs", macros, num_macros, binding, UNIFORM_COUNT);
}

void ModuleRenderer::DrawClippingSpace(const float4x4& proj, const float4x4& view) const
{
    // build camera transform from view transform
    float3x3 rot(view.x.xyz(), view.y.xyz(), view.z.xyz());
    rot = transpose(rot);
    float3 pos = -mul(rot, view.w.xyz());
    float4x4 transform(float4(rot.x, 1.0f), float4(rot.y, 1.0f), float4(rot.z, 1.0f), float4(pos, 1.0f));

    dd::axisTriad(transform, 0.1f, 1.0f, 0);

    float3 p[8];
    GetClippingPoints(proj, view, p);

    dd::box(p, dd::colors::LightYellow);
}

void ModuleRenderer::GetClippingPoints(const float4x4& proj, const float4x4& view, float3 points[8]) const
{
    float4x4 clip_to_world = inverse(mul(proj, view));

    // Homogenous points for source cube in clip-space
    float4 v[8] =
    {
        {-1, -1, -1, 1},
        {-1,  1, -1, 1},
        { 1,  1, -1, 1},
        { 1, -1, -1, 1},
        {-1, -1, 1, 1 },
        {-1,  1, 1, 1 },
        { 1,  1, 1, 1 },
        { 1, -1, 1, 1 }
    };

    for (int i = 0; i < 8; i++)
    {
        v[i] = mul(clip_to_world, v[i]);

        v[i] /= v[i].w;
        points[i] = v[i].xyz();
    }

}

void ModuleRenderer::UpdateCameraUniform() const
{
	Camera* camera			   = Scene::GetService()->GetActiveCamera();
    float4x4 view              = camera->GetViewMatrix();
    const float4x4& projection = camera->GetProjMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER, uniforms[UNIFORM_CAMERA]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float4x4), reinterpret_cast<const void*>(&projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float4x4), sizeof(float4x4), reinterpret_cast<const void*>(&view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ModuleRenderer::UpdateLightUniform() const
{
    Scene* scene        = Scene::GetService();	
    Hints* hints        = Hints::GetService();	
    Scene::Light* light = scene->GetActiveLight();

    float4x4 proj = identity;
    float4x4 view = identity;

	if (light != nullptr)
	{
		if (light->type == Scene::LIGHT_DIRECTIONAL)
		{
			Scene::AABB aabb;

			float4 light_rotation = LookRotation(light->direction, light->up);

			CalcLightSpaceBBox(light_rotation, aabb);

			// Setting light params

			float2 center((aabb.min.x + aabb.max.x)*0.5f, (aabb.min.y + aabb.max.y)*0.5f);
			float2 size((aabb.max.x - aabb.min.x)*0.5f, (aabb.max.y - aabb.min.y)*0.5f);

			float3 light_pos = qrot(light_rotation, float3(center.x, center.y, aabb.max.z + 0.1f));
			float nearP = 0.1f;
			float farP = (aabb.max.z - aabb.min.z)*2.0f;

			Camera light;
			light.SetRotation(light_rotation);
			light.SetPosition(light_pos);
			light.SetOrtho(-size.x, size.x, -size.y, size.y, nearP, farP);

			proj = light.GetProjMatrix();
			view = light.GetViewMatrix();
		}
		else if (light->type == Scene::LIGHT_POINT)
		{
			view.w = float4(-light->position, 1.0);
		}
	}

    glBindBuffer(GL_UNIFORM_BUFFER, uniforms[UNIFORM_LIGHT]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float4x4), reinterpret_cast<const void*>(&proj));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float4x4), sizeof(aiMatrix4x4), reinterpret_cast<const void*>(&view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    if(hints->GetBoolValue(Hint::SHOW_SHADOW_CLIPPING))
    {
        DrawClippingSpace(proj, view);
    }
}

void ModuleRenderer::CalcLightSpaceBBox(const float4& light_rotation, Scene::AABB& aabb) const
{
	Scene* scene = Scene::GetService();

    float3x3 light_mat_rot = qmat(qconj(light_rotation));
    float4x4 light_mat(float4(light_mat_rot.x, 0.0f), float4(light_mat_rot.y, 0.0f), 
                             float4(light_mat_rot.z, 0.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));

	for(NodeList::const_iterator it = draw_nodes.begin(), end = draw_nodes.end(); it != end; ++it)
	{
		Scene::Node* node = *it;

		for (Scene::MeshInstList::const_iterator it_mesh = node->meshes.begin(), end_mesh = node->meshes.end(); it_mesh != end_mesh; ++it_mesh)
		{
            if(it_mesh->visible)
            {
				assert(it_mesh->index < scene->GetNumMeshes());

				const Scene::Mesh* mesh = scene->GetMesh(it_mesh->index);
                const Scene::Material* material = scene->GetMaterial(mesh->material);

                if(material->cast_shadows)
                {
                    scene->ComputeBBoxMesh(mesh, mul(light_mat, node->global), aabb);
                }
            }
        }
	}

    // \todo: 
    // En una situación real faltaria meter objetos que, estando fuera del frustum estan dentro de los dos planos laterales de volumen ortogonal en
    // dirección a la luz. Estos objetos, a pesar de no estar en el frustum, prodrían arrojar sombras sobre otros que si lo están.
}
