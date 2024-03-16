#include "Globals.h"

#include "BatchManager.h"
#include "GeometryBatch.h"
#include "ComponentMeshRenderer.h"
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
    if(!skinningProgram)
    {
        CreateSkinningProgram();
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
        batches.push_back(std::make_unique<GeometryBatch>(tag, batch_index, skinningProgram.get(), skinningProgramNoTangents.get()));
        batches.back()->Add(object);
    }

    return batch_index;
}

void BatchManager::Remove(ComponentMeshRenderer* object)
{
    batches[object->GetBatchIndex()]->Remove(object);
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

void BatchManager::MarkForUpdate(const NodeList& objects)
{
    for(const TRenderInfo& info : objects)
    {
        if(info.mesh && info.mesh->GetBatchIndex() != UINT_MAX)
        {
            batches[info.mesh->GetBatchIndex()]->MarkForUpdate(info.mesh);
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

void BatchManager::CreateSkinningProgram()
{
    const char* defines[] = { "#define NO_TANGENTS\n"};
    std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/skinning.glsl");
    std::unique_ptr<Shader> shaderNoTangents = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/skinning.glsl", &defines[0], 1);

    bool ok = shader->Compiled() && shaderNoTangents->Compiled();

    if(ok)
    {        
        skinningProgram = std::make_unique<Program>(shader.get());
        skinningProgramNoTangents = std::make_unique<Program>(shaderNoTangents.get());

        ok = skinningProgram->Linked() && skinningProgramNoTangents->Linked();
    }

    if(!ok)
    {
        skinningProgram.release();
        skinningProgramNoTangents.release();
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