#include "Globals.h"
#include "ModuleRenderer.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ModuleHints.h"
#include "ModuleEditor.h"
#include "GBufferExportPass.h"
#include "DeferredResolvePass.h"
#include "DeferredResolveProxy.h"
#include "DeferredDecalPass.h"
#include "ScreenSpaceAO.h"
#include "ForwardPass.h"
#include "FxaaPass.h"
#include "ShadowmapPass.h"
#include "CascadeShadowPass.h"
#include "FogPass.h"
#include "LinePass.h"
#include "ParticlePass.h"
#include "DepthRangePass.h"

#include "PostprocessShaderLocations.h"

#include "GameObject.h"

#include "ComponentMeshRenderer.h"

#include "ComponentCamera.h"
#include "ComponentAnimation.h"
#include "ComponentParticleSystem.h"
#include "ComponentTrail.h"
#include "ComponentLine.h"
#include "BatchManager.h"
#include "Postprocess.h"
#include "SkyboxRollout.h"
#include "PanelProperties.h"

#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"

#include "IBLData.h"
#include "LightManager.h"
#include "DirLight.h"
#include "SphereLight.h"
#include "QuadLight.h"
#include "TubeLight.h"
#include "LocalIBLLight.h"

#include "Application.h"

#include "OpenGL.h"
#include "DebugDraw.h"

#include "imgui/imgui.h"

#include <string>
#include <functional>
#include <algorithm>

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

ModuleRenderer::ModuleRenderer() : Module("renderer")
{
    batch_manager = std::make_unique<BatchManager>();
    postProcess = std::make_unique<Postprocess>();
    forward = std::make_unique<ForwardPass>();
    exportGBuffer = std::make_unique<GBufferExportPass>();
    deferredResolve = std::make_unique<DeferredResolvePass>();
    deferredProxy = std::make_unique<DeferredResolveProxy>();
    decalPass = std::make_unique<DeferredDecalPass>();
    fxaa = std::make_unique<FxaaPass>();
    shadowmapPass = std::make_unique<ShadowmapPass>();
    cascadeShadowPass = std::make_unique<CascadeShadowPass>();
    fogPass = std::make_unique<FogPass>();
    linePass = std::make_unique<LinePass>();
    particlePass = std::make_unique<ParticlePass>();
    depthRangePass = std::make_unique<DepthRangePass>();
}

bool ModuleRenderer::Init(Config* config /*= nullptr*/)
{
    ssao = std::make_unique<ScreenSpaceAO>();

    LoadDefaultShaders();
    postProcess->Init();

    return true;
}

ModuleRenderer::~ModuleRenderer()
{
}

void ModuleRenderer::Draw(ComponentCamera* camera, ComponentCamera* culling, Framebuffer* frameBuffer, unsigned width, unsigned height)
{
    UpdateCameraUBO(camera);
    App->level->GetLightManager()->UpdateGPUBuffers();

    render_list.UpdateFrom(culling->frustum, App->level->GetRoot()); 

    batch_manager->MarkForUpdate(render_list.GetOpaques());
    batch_manager->MarkForUpdate(render_list.GetTransparents());

    if (App->hints->GetBoolValue(ModuleHints::ENABLE_CASCADE_SHADOW))
    {
        cascadeShadowPass->updateRenderList(culling->frustum);

        for (uint i = 0; i < CascadeShadowPass::CASCADE_COUNT; ++i)
        {
            const RenderList& renderList = cascadeShadowPass->getRenderList(i);

            batch_manager->MarkForUpdate(renderList.GetOpaques());
            batch_manager->MarkForUpdate(renderList.GetTransparents());
        }
    }
    else
    {
        shadowmapPass->updateRenderList(culling->frustum, float2(-1.0f, 1.0f));
        batch_manager->MarkForUpdate(shadowmapPass->getRenderList().GetOpaques());
        batch_manager->MarkForUpdate(shadowmapPass->getRenderList().GetTransparents());
    }


    batch_manager->DoUpdate();


    // General Buffer bindings 
    cameraUBO->BindToPoint(CAMERA_UBO_BINDING);
    App->level->GetLightManager()->Bind();

    RenderDeferred(camera, culling, frameBuffer, width, height);
}

void ModuleRenderer::RenderForward(ComponentCamera* camera, Framebuffer* frameBuffer, unsigned width, unsigned height)
{
    frameBuffer->Bind();
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    forward->executeOpaque(render_list, nullptr, width, height);
    forward->executeTransparent(render_list, nullptr, width, height);
    frameBuffer->Unbind();


    // Skybox
    frameBuffer->Bind();
    App->level->GetSkyBox()->DrawEnvironment(camera->GetProjectionMatrix(), camera->GetViewMatrix());
    frameBuffer->Unbind();
}

void ModuleRenderer::RenderDeferred(ComponentCamera* camera, ComponentCamera* culling, Framebuffer* frameBuffer, unsigned width, unsigned height)
{
    // TODO: Update batch transforms and skinning / morphing for 2 cameras, render and shadows
    
    // Deferred
    exportGBuffer->execute(render_list, width, height);

    depthRangePass->execute(exportGBuffer->getDepth(), width, height);

    if (App->hints->GetBoolValue(ModuleHints::ENABLE_CASCADE_SHADOW))
    {
        cascadeShadowPass->execute();
    }
    else
    {
        shadowmapPass->updateRenderList(culling->frustum, depthRangePass->getMinMaxDepth());
        shadowmapPass->execute( 3000, 3000);
    }

    //decalPass->execute(camera, render_list, width, height);
    ssao->execute(width, height);

    deferredResolve->execute(frameBuffer, width, height);
    deferredProxy->execute(frameBuffer, width, height);

    frameBuffer->AttachDepthStencil(exportGBuffer->getDepth(), GL_DEPTH_ATTACHMENT);
    assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);
    

    // Forward Transparent
    frameBuffer->Bind();
    forward->executeTransparent(render_list, nullptr, width, height);
    frameBuffer->Unbind();

    RenderVFX(camera, culling, frameBuffer, width, height);

    DrawAreaLights(camera, frameBuffer);
    

    // Skybox
    frameBuffer->Bind();
    App->level->GetSkyBox()->DrawEnvironment(camera->GetProjectionMatrix(), camera->GetViewMatrix());
    frameBuffer->Unbind();

    fogPass->execute(frameBuffer, width, height);
}

void ModuleRenderer::RenderVFX(ComponentCamera *camera, ComponentCamera *culling, Framebuffer *frameBuffer, unsigned width, unsigned height)
{
    //linePass->execute(camera, render_list, frameBuffer, width, height);
    //particlePass->execute(camera, render_list, frameBuffer, width , height);
}

void ModuleRenderer::DrawForSelection(ComponentCamera* camera)
{
	float4x4 proj   = camera->GetProjectionMatrix();	
	float4x4 view   = camera->GetViewMatrix();

    render_list.UpdateFrom(camera->frustum, App->level->GetRoot());

    SelectionPass(proj, view);
}

void ModuleRenderer::UpdateCameraUBO(ComponentCamera *camera)
{
    struct CameraData
    {
        float4x4 proj     = float4x4::identity;
        float4x4 view     = float4x4::identity;
        float4   view_pos = float4::zero;
    } cameraData;

    if(!cameraUBO)
    {
        cameraUBO.reset(new Buffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(cameraData), nullptr));
    }

    cameraData.proj     = camera->GetProjectionMatrix();  
    cameraData.view     = camera->GetViewMatrix();
    cameraData.view_pos = float4(cameraData.view.RotatePart().Transposed().Transform(-cameraData.view.TranslatePart()), 1.0);

    cameraUBO->InvalidateData();
    cameraUBO->SetData(0, sizeof(CameraData), &cameraData);
}

void ModuleRenderer::SelectionPass(const float4x4& proj, const float4x4& view)
{
    // Set camera uniforms shared for all
    App->programs->UseProgram("selection", 0);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));

    //DrawNodes(render_list.GetOpaques(), &ModuleRenderer::DrawSelection);
    //DrawNodes(render_list.GetTransparents(), &ModuleRenderer::DrawSelection);
}

void ModuleRenderer::LoadDefaultShaders()
{
    const char* default_macros[]	= { "#define SHADOWS_ENABLED 1 \n" , "#define SHOW_CASCADES 1 \n", "#define ENABLE_SOFT 1 \n"}; 
    const unsigned num_default_macros  = sizeof(default_macros)/sizeof(const char*);

    App->programs->Load("default", "Assets/Shaders/defaultVS.glsl", "Assets/Shaders/defaultFS.glsl", default_macros, num_default_macros);
    App->programs->Load("default_batch", "Assets/Shaders/default_batch.vs", "Assets/Shaders/default_batch.fs", default_macros, num_default_macros);

    const char* macros[]		  = { "#define BLOOM 1 \n", "#define GAMMA 1\n" }; 
    const unsigned num_macros     = sizeof(macros)/sizeof(const char*);

    App->programs->Load("postprocess", "Assets/Shaders/postprocess.vs", "Assets/Shaders/postprocess.fs", macros, num_macros);

    const char* gaussian_macros[]       = { "#define HORIZONTAL 1 \n" }; 
    const unsigned num_gaussian_macros  = sizeof(gaussian_macros)/sizeof(const char*);

    App->programs->Load("gaussian", "Assets/Shaders/postprocess.vs", "Assets/Shaders/gaussian.fs", gaussian_macros, num_gaussian_macros);
    App->programs->Load("chebyshev", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/chebyshev.fs", nullptr, 0, nullptr);

    const char* bloom_macros[]       = { "#define MSAA 1 \n" }; 
    const unsigned num_bloom_macros  = sizeof(bloom_macros)/sizeof(const char*);

    App->programs->Load("bloom", "Assets/Shaders/postprocess.vs", "Assets/Shaders/bloom.fs", bloom_macros, num_bloom_macros);

    const char* show_uv_macros[]       = { "#define TEXCOORD1 1 \n" }; 
    const unsigned num_uv_macros  = sizeof(show_uv_macros)/sizeof(const char*);
    App->programs->Load("show_uvs", "Assets/Shaders/show_uvs.vs", "Assets/Shaders/show_uvs.fs", show_uv_macros, num_uv_macros);
    App->programs->Load("selection", "Assets/Shaders/selection.vs", "Assets/Shaders/selection.fs", nullptr, 0);
    App->programs->Load("showtexture", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/fullscreenTextureFS.glsl", nullptr, 0);
    App->programs->Load("convert_texture", "Assets/Shaders/fullscreenVS.glsl", "Assets/Shaders/convertMetallicTextureFS.glsl", nullptr, 0);
}

void ModuleRenderer::DrawDebug()
{
    shadowmapPass->debugDraw();
    //DebugDrawOBB();
}

void ModuleRenderer::DebugDrawOBB(const NodeList& objects)
{
    for(const TRenderInfo& render_info : objects)
    {
        AABB local_bounding = render_info.go->GetLocalBBox();

        if(local_bounding.IsFinite())
        {
            float4x4 transform = render_info.go->GetGlobalTransformation();

            OBB global_bounding = local_bounding.Transform(transform);

            math::float3 corners[8];
            global_bounding.GetCornerPoints(corners);
            std::swap(corners[2], corners[5]);
            std::swap(corners[3], corners[4]);
            std::swap(corners[4], corners[5]);
            std::swap(corners[6], corners[7]);

            dd::box(corners, dd::colors::Sienna); 

        }
    }

}

void ModuleRenderer::DebugDrawOBB()
{
    DebugDrawOBB(render_list.GetOpaques());
    DebugDrawOBB(render_list.GetTransparents());
}

void ModuleRenderer::DebugDrawAnimation()
{
    DebugDrawAnimation(App->level->GetRoot());
}

void ModuleRenderer::DebugDrawAnimation(const GameObject* go)
{
    const ComponentAnimation* animation = go->FindFirstComponent<ComponentAnimation>();
    if(animation && animation->GetDebugDraw())
    {
        DebugDrawHierarchy(go);
    }
    else
    {
        for(std::list<GameObject*>::const_iterator it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
        {
            DebugDrawAnimation((*it));
        }
    }
}

void ModuleRenderer::DebugDrawHierarchy(const GameObject* go)
{
    const float4x4& transform = go->GetGlobalTransformation();

    if(go->GetParent())
    {
        const float4x4& parent_transform = go->GetParent()->GetGlobalTransformation();

        dd::line(parent_transform.TranslatePart(), transform.TranslatePart(), dd::colors::Blue, 0, false);
		//dd::axisTriad(transform, 1.3f,  13.f, 0, false);
    }

    for(std::list<GameObject*>::const_iterator it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
    {
        DebugDrawHierarchy((*it));
    }
}

void ModuleRenderer::DebugDrawTangentSpace()
{
	/*
    for(NodeList::iterator it = opaque_nodes.begin(), end = opaque_nodes.end(); it != end; ++it)
    {
        if(it->material && it->material->GetDDTangent())
        {
            const ResourceMesh* mesh = it->mesh->GetResource();

            if(mesh && (mesh->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0 && (mesh->attribs& ResourceMesh::ATTRIB_NORMALS))
            {
                DebugDrawTangentSpace(mesh, it->transform);
            }
        }
    }

    for(NodeList::iterator it = transparent_nodes.begin(), end = transparent_nodes.end(); it != end; ++it)
    {
        if(it->material && it->material->GetDDTangent())
        {
            const ResourceMesh* mesh = it->mesh->GetResource();

            if(mesh && (mesh->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0 && (mesh->attribs& ResourceMesh::ATTRIB_NORMALS))
            {
                DebugDrawTangentSpace(mesh, it->transform);
            }
        }
    }
	*/
}

void ModuleRenderer::DebugDrawTangentSpace(const ResourceMesh* mesh, const float4x4& transform)
{
    float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

    for(unsigned i = 0, count = mesh->num_vertices; i < count; ++i)
    {
        float3 position  = transform.TransformPos(mesh->src_vertices[i]);
        float3 normal    = transform.TransformDir(mesh->src_normals[i]);
        float3 tangent   = transform.TransformDir(mesh->src_tangents[i]);
        float3 bitangent = normal.Cross(tangent);

        float4x4 tbn(float4(tangent, 0.0f), float4(bitangent, 0.0f), float4(normal, 0.0f), float4(position, 1.0f));

        dd::axisTriad(tbn, metric_proportion*0.1f*0.1f, metric_proportion*0.1f, 0);
    }
}

float4x4 ModuleRenderer::SetFrustum(float left, float right, float bottom, float top, float _near, float _far)
{
    float rl = right-left;
    float tb = top-bottom;
    float fn = _far-_near;

    float4x4 frustum;
    frustum.SetCol(0, 2.0f*_near/rl,             0.0f,                 0.0f,  0.0f);
    frustum.SetCol(1, 0.0f,              2.0f*_near/tb,                0.0f,  0.0f);
    frustum.SetCol(2, (right+left)/rl, (top+bottom)/tb,    -(_far+_near)/fn, -1.0f);
    frustum.SetCol(3, 0.0f,                       0.0f, -2.0f*_far*_near/fn, 0.0f);

    return frustum;
}

float4x4 ModuleRenderer::SetOrtho(float left, float right, float bottom, float top, float _near, float _far)
{
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = -2.0f / (_far - _near);

	float tx = -(right + left) / (right - left);
	float ty = - (top + bottom)/(top - bottom);
	float tz = - (_far + _near)/(_far - _near);

    float4x4 projection;
    projection.SetRow(0, math::float4(a, 0.0f, 0.0f, 0.0f));
    projection.SetRow(1, math::float4(0.0f , b, 0.0f, 0.0f));
    projection.SetRow(2, math::float4(0.0f , 0.0f, c, 0.0f));
    projection.SetRow(3, math::float4(tx, ty, tz, 1.0f));

    return projection;
}

void ModuleRenderer::DrawAreaLights(ComponentCamera* camera, Framebuffer* frameBuffer)
{
    if(!CreateAreaLightProgram()) return ;
    if(!CreateProbeProgram()) return ;

    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Area Lights");
    frameBuffer->Bind();

    float4x4 view = camera->GetOpenGLViewMatrix();
    float4x4 proj = camera->GetOpenGLProjectionMatrix();

    const ResourceMesh *sphere = App->resources->GetDefaultSphere();
    const ResourceMesh *plane = App->resources->GetDefaultPlane();
    const ResourceMesh *cylinder = App->resources->GetDefaultCylinder();

    primitiveProgram->Use();
    primitiveProgram->BindUniformFromName("view", view, false);
    primitiveProgram->BindUniformFromName("proj", proj, false);

    LightManager* lightManager = App->level->GetLightManager();

    for (uint i = 0, count = lightManager->GetNumSphereLights(); i < count; ++i)
    {
        const SphereLight* light = lightManager->GetSphereLight(i);

        if (light->GetEnabled())
        {
            float4x4 model = float4x4::UniformScale(light->GetRadius());
            model.SetTranslatePart(light->GetPosition());

            primitiveProgram->BindUniformFromName("model", model, true);
            primitiveProgram->BindUniformFromName("color", float4(light->GetColor() * light->GetIntensity(), 1.0));

            glBindVertexArray(sphere->GetVAO());
            glDrawElements(GL_TRIANGLES, sphere->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }

    for (uint i = 0, count = lightManager->GetNumQuadLights(); i < count; ++i)
    {
        const QuadLight* light = lightManager->GetQuadLight(i);

        if (light->GetEnabled())
        {
            float4x4 model = float4x4::identity;
            model.SetCol3(0, light->GetRight() * light->GetSize().x);
            model.SetCol3(1, light->GetUp() * light->GetSize().y);
            model.SetCol3(2, light->GetRight().Cross(light->GetUp()));
            model.SetTranslatePart(light->GetPosition());

            primitiveProgram->BindUniformFromName("model", model, true);
            primitiveProgram->BindUniformFromName("color", float4(light->GetColor() * light->GetIntensity(), 1.0));

            glBindVertexArray(plane->GetVAO());
            glDrawElements(GL_TRIANGLES, plane->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);

        }
    }

    glDisable(GL_CULL_FACE);
    for (uint i = 0, count = lightManager->GetNumTubeLights(); i < count; ++i)
    {
        const TubeLight* light = lightManager->GetTubeLight(i);

        if (light->GetEnabled())
        {
            float4x4 model = float4x4::identity;
            model.SetRotatePart(light->GetRotation());
            model.SetTranslatePart(light->GetPosition());
            model.ScaleCol(0, light->GetRadius());
            model.ScaleCol(2, light->GetRadius());
            // \todo: compute model from two points and radius

            primitiveProgram->BindUniformFromName("model", model, true);
            primitiveProgram->BindUniformFromName("color", float4(light->GetColor() * light->GetIntensity(), 1.0));

            glBindVertexArray(cylinder->GetVAO());
            glDrawElements(GL_TRIANGLES, cylinder->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);

        }
    }
    glEnable(GL_CULL_FACE);

    Program* program = nullptr;
    switch (App->editor->props->getSkyboxEditor()->getSelected())
    {
    case SkyboxRollout::Environment:
        program = probeProgram.get();
        break;
    case SkyboxRollout::DiffuseIBL:
        program = probeProgram.get();
        break;
    case SkyboxRollout::PrefilteredIBL:
        program = probeLodProgram.get();
        break;
    }

    program->Use();
    program->BindUniformFromName("view", view, false);
    program->BindUniformFromName("proj", proj, false);

    for (uint i = 0, count = lightManager->GetNumLocalIBLLights(); i < count; ++i)
    {
        const LocalIBLLight *light = lightManager->GetLocalIBLLight(i);

        if (light->GetEnabled())
        {
            const IBLData &ibl = light->getIBLData();

            const TextureCube *texture = nullptr;
            switch (App->editor->props->getSkyboxEditor()->getSelected())
            {
            case SkyboxRollout::Environment:
                texture = ibl.GetEnvironment();
                break;
            case SkyboxRollout::DiffuseIBL:
                texture = ibl.GetDiffuseIBL();
                break;
            case SkyboxRollout::PrefilteredIBL:
                texture = ibl.GetPrefilterdIBL();
                program->BindUniformFromName("roughness", App->editor->props->getSkyboxEditor()->getRoughness());
                program->BindUniformFromName("prefilteredLevels", int(ibl.GetPrefilterdLevels()));
                program->BindTextureFromName("environmentBRDF", 1, ibl.GetEnvironmentBRDF());
                break;
            }

            // \todo: render with environment brdf

            if (texture)
            {
                float4x4 model = float4x4::UniformScale(0.1f);
                model.SetTranslatePart(light->GetPosition());

                program->BindUniformFromName("model", model, true);
                program->BindTextureFromName("cubemap", 0, texture);

                glBindVertexArray(sphere->GetVAO());
                glDrawElements(GL_TRIANGLES, sphere->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
                glBindVertexArray(0);
            }
        }
    }

    glPopDebugGroup();
}

bool ModuleRenderer::CreateProbeProgram()
{
	if(probeProgram && probeLodProgram) return true;

	std::unique_ptr<Shader> vertex, fragment, fragmentLod;

    vertex.reset(Shader::CreateVSFromFile("assets/shaders/probeVS.glsl", 0, 0));

    bool ok = vertex->Compiled();

	if (ok)
	{
		fragment.reset(Shader::CreateFSFromFile("assets/shaders/probeFS.glsl", 0, 0));

        const char *skyboxLodMacros[] = {"#define PREFILTERED\n"};

		fragmentLod.reset(Shader::CreateFSFromFile("assets/shaders/probeFS.glsl", &skyboxLodMacros[0], 1));

        ok = fragment->Compiled();
	}

	if (ok)
	{
		probeProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "ProbeProgram");
		probeLodProgram = std::make_unique<Program>(vertex.get(), fragmentLod.get(), "ProbeLodProgram");

		ok = probeProgram->Linked() && probeLodProgram->Linked();
	}

    if(!ok)
    {
        probeProgram.release();
        probeLodProgram.release();
    }

    return ok;
}

bool ModuleRenderer::CreateAreaLightProgram()
{
	if(primitiveProgram) return true;

	std::unique_ptr<Shader> vertex, fragment;

    vertex.reset(Shader::CreateVSFromFile("assets/shaders/rigid.glsl", 0, 0));

    bool ok = vertex->Compiled();

	if (ok)
	{
		fragment.reset(Shader::CreateFSFromFile("assets/shaders/color.fs", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		primitiveProgram = std::make_unique<Program>(vertex.get(), fragment.get(), "AreaLightProgram");

		ok = primitiveProgram->Linked();
	}

    if(!ok)
    {
        primitiveProgram.release();
    }

    return ok;
}