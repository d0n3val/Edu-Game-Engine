#include "Globals.h"

#include "BatchManager.h"
#include "Batch.h"
#include "ComponentMeshRenderer.h"

#include "Leaks.h"

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
}

uint BatchManager::Add(ComponentMeshRenderer* object, const HashString& tag)
{
    uint batch_index = 0;

    for (; batch_index < batches.size(); ++batch_index)
    {
        std::unique_ptr<Batch> &batch = batches[batch_index];
        if (batch->GetTagName() == tag && batch->CanAdd(object))
        {
            batch->Add(object);
            break;
        }
    }

    if (batch_index == batches.size())
    {
        batches.push_back(std::make_unique<Batch>(tag));
        batches.back()->Add(object);
    }

    return batch_index;
}

void BatchManager::Remove(ComponentMeshRenderer* object)
{
    batches[object->GetBatchIndex()]->Remove(object);
}

void BatchManager::Render(const ComponentMeshRenderer* object)
{
    batches[object->GetBatchIndex()]->Render(object);
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

void BatchManager::DoRender(uint transformIndex, uint materialsIndex, uint texturesLocation)
{
    // \todo: Sort from front to back ?
    for(std::unique_ptr<Batch>& batch : batches)
    {
        if (!batch->IsEmpty())
        {
            batch->DoRender(transformIndex, materialsIndex, texturesLocation);
        }
    }
}

void BatchManager::FillBatchNames(std::vector<HashString>& names) const
{
    for(const std::unique_ptr<Batch>& batch : batches)
    {
        const HashString& tag_name = batch->GetTagName();
        if(std::find(names.begin(), names.end(), tag_name) == names.end())
        {
            names.push_back(tag_name);
        }
    }
}
