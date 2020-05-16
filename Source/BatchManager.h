#ifndef _BATCHMANAGER_H_
#define _BATCHMANAGER_H_

#include "HashString.h"

#include <vector>
#include <memory>

class Batch;
class ComponentMeshRenderer;

class BatchManager
{
    typedef std::vector<std::unique_ptr<Batch> > BatchPool;

    BatchPool batches;

public:
    BatchManager();
    ~BatchManager();

    void AddToBatch(ComponentMeshRenderer* object, const HashString& tag, uint& batch_index, uint& object_index);
    void RemoveFromBatch(uint batch_index, uint object_index);

    void AddToRender(uint batch_index, uint object_index);
    void DoRender();

    void FillBatchNames(std::vector<HashString>& names) const;
};

#endif /* _BATCHMANAGER_H_ */


