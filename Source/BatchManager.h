#ifndef _BATCHMANAGER_H_
#define _BATCHMANAGER_H_

#include "RenderList.h"
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

    uint Add(ComponentMeshRenderer* object, const HashString& tag);
    void Remove(ComponentMeshRenderer* object);

    void Render(const ComponentMeshRenderer* object);

    void UpdateModel(const NodeList& objects);
    void DoRender(uint transformIndex, uint materialsIndex, uint texturesLocation);

    void FillBatchNames(std::vector<HashString>& names) const;
};

#endif /* _BATCHMANAGER_H_ */


