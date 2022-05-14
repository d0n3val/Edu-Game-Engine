#include "Globals.h"

#include "BatchManager.h"
#include "GeometryBatch.h"
#include "ComponentMeshRenderer.h"
#include "GameObject.h"

#include "Leaks.h"

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
}

uint BatchManager::Add(const ComponentMeshRenderer* object, const HashString& tag)
{
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
        batches.push_back(std::make_unique<GeometryBatch>(tag));
        batches.back()->Add(object);
    }

    return batch_index;
}

void BatchManager::Remove(const ComponentMeshRenderer* object)
{
    batches[object->GetBatchIndex()]->Remove(object);
}

void BatchManager::Render(const NodeList &objects, uint flags)
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

void BatchManager::UpdateModel(const NodeList& objects)
{
    for(const TRenderInfo& info : objects)
    {
        if(info.mesh && info.mesh->GetBatchIndex() != UINT_MAX)
        {
            batches[info.mesh->GetBatchIndex()]->UpdateModel(info.mesh);
        }
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
