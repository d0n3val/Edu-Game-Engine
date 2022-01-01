#include "Globals.h"

#include "BatchManager.h"
#include "Batch.h"
#include "ComponentMeshRenderer.h"

#include "Leaks.h"

#define DEFAULT_MAX_VERTICES 64000
#define DEFAULT_MAX_OBJECTS 32

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
}

void BatchManager::AddToBatch(ComponentMeshRenderer* object, const HashString& tag, uint& batch_index, uint& object_index)
{
    if(object->GetMeshRes()->HasAttrib(ATTRIB_BONES))
	{
		batch_index  = UINT_MAX;
		object_index = UINT_MAX;
    }
    else
    {
        for(batch_index = 0; batch_index < batches.size(); ++batch_index)
        {
            std::unique_ptr<Batch>& batch = batches[batch_index];
            if(batch->GetTagName() == tag && batch->CanAdd(object))
            {
                object_index = batch->Add(object);
                break;
            }
        }

        if(batch_index == batches.size())
        {
            batches.push_back(std::make_unique<Batch>(tag));

            object_index = batches.back()->Add(object);
        }
    }
}

void BatchManager::RemoveFromBatch(uint batch_index, uint object_index)
{
    assert(batch_index < batches.size());

    batches[batch_index]->Remove(object_index);

    if(batches[batch_index]->IsEmpty())
    {
        batches[batch_index].reset(new Batch(batches[batch_index]->GetTagName()));
    }
}

void BatchManager::AddToRender(uint batch_index, uint object_index)
{
    assert(batch_index < batches.size() && !batches[batch_index]->IsEmpty());

    batches[batch_index]->AddToRender(object_index);
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
