#include "Globals.h"

#include "BatchManager.h"
#include "GeometryBatch.h"
#include "ComponentMeshRenderer.h"
#include "BatchDrawCommands.h"
#include "GameObject.h"

#include "OpenGL.h"

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
}

uint BatchManager::Add(ComponentMeshRenderer* object, const HashString& tag)
{
    if(!programs.skinningProgram)
    {
        CreateSkinningProgram();
    }

    if(!programs.culling)
    {
        CreateFrustumCullingProgram();
    }

    uint batch_index = 0;

    for (; batch_index < batches.size(); ++batch_index)
    {
        std::unique_ptr<GeometryBatch> &batch = batches[batch_index];
        if (batch->GetTagName() == tag && batch->CanAdd(object))
        {
            batch->Add(object);
            break;
        }
    }

    if (batch_index == batches.size())
    {
        batches.push_back(std::make_unique<GeometryBatch>(tag, batch_index, &programs));
        batches.back()->Add(object);
    }

    return batch_index;
}

void BatchManager::Remove(ComponentMeshRenderer* object)
{
    batches[object->GetBatchIndex()]->Remove(object);
}

void BatchManager::DoFrustumCulling(BatchDrawCommands &drawCommands, const float4 *planes, const float3& cameraPos, bool opaque)
{
    if (drawCommands.getNumBatches() < uint(batches.size()))
    {
        drawCommands.resize(uint(batches.size()));
    }

    for(auto& batch : batches)
    {
        if ((batch->GetRenderMode() == RENDER_OPAQUE) == opaque)
        {
            batch->DoFrustumCulling(drawCommands, planes, cameraPos);
        }
    }
}

void BatchManager::DoRenderCommands(BatchDrawCommands &drawCommands)
{
    for(auto& batch : batches)
    {
        batch->DoRenderCommands(drawCommands);
    }

}

void BatchManager::DoRender(const NodeList &objects, uint flags)
{
    if((flags & BR_FLAG_KEEP_ORDER) != 0)
    {
        uint lastBatch = UINT_MAX;
        for (const TRenderInfo &info : objects)
        {
            uint batch = info.mesh->GetBatchIndex();
            if (batch != UINT_MAX)
            {
                if (batch != lastBatch && lastBatch != UINT_MAX)
                {
                    batches[lastBatch]->DoRender(flags);
                }

                batches[batch]->Render(info.mesh);
                lastBatch = batch;
            }
        }

        if(lastBatch != UINT_MAX)
        {
            batches[lastBatch]->DoRender(flags);
        }
    }
    else
    {
        for (const TRenderInfo &info : objects)
        {
            uint batchIndex = info.mesh->GetBatchIndex();
            if (batchIndex != UINT_MAX)
            {
                batches[batchIndex]->Render(info.mesh);
            }
        }

        for (std::unique_ptr<GeometryBatch> &batch : batches)
        {
            if (batch->HasCommands())
            {
                batch->DoRender(flags);
            }
        }
    }
}


void BatchManager::DoUpdate()
{
    for (std::unique_ptr<GeometryBatch>& batch : batches)
    {
        batch->DoUpdate();
    }

}

void BatchManager::FillBatchNames(std::vector<HashString>& names) const
{
    for(const std::unique_ptr<GeometryBatch>& batch : batches)
    {
        const HashString& tag_name = batch->GetTagName();
        if(std::find(names.begin(), names.end(), tag_name) == names.end())
        {
            names.push_back(tag_name);
        }
    }
}

void BatchManager::OnMaterialModified(UID materialID)
{
    for(const std::unique_ptr<GeometryBatch>& batch : batches)
    {
        batch->OnMaterialModified(materialID);
    }
}

void BatchManager::CreateFrustumCullingProgram()
{
    const char* defines[] = { "#define TRANSPARENTS\n" };

    std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/frustumCulling.glsl");
    std::unique_ptr<Shader> shaderTransparents = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/frustumCulling.glsl", &defines[0], 1);

    bool ok = shader->Compiled();

    if(ok)
    {        
        programs.culling = std::make_unique<Program>(shader.get());
        programs.cullingTransparent = std::make_unique<Program>(shaderTransparents.get());

        ok = programs.culling->Linked() && programs.cullingTransparent->Linked();
    }

    if(!ok)
    {
        programs.culling.release();
        programs.cullingTransparent.release();
    }
}

void BatchManager::CreateSkinningProgram()
{
    const char* defines[] = { "#define NO_TANGENTS\n"};
    std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/skinning.glsl");
    std::unique_ptr<Shader> shaderNoTangents = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/skinning.glsl", &defines[0], 1);

    bool ok = shader->Compiled() && shaderNoTangents->Compiled();

    if(ok)
    {        
        programs.skinningProgram = std::make_unique<Program>(shader.get());
        programs.skinningProgramNoTangents = std::make_unique<Program>(shaderNoTangents.get());

        ok = programs.skinningProgram->Linked() && programs.skinningProgramNoTangents->Linked();
    }

    if(!ok)
    {
        programs.skinningProgram.release();
        programs.skinningProgramNoTangents.release();
    }
}

ComponentMeshRenderer* BatchManager::FindComponentMeshRenderer(uint batchIndex, uint instanceIndex) const
{
    if (batchIndex < batches.size())
    {
        return batches[batchIndex]->FindObject(instanceIndex);
    }

    return nullptr;
}