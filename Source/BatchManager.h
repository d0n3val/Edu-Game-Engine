#ifndef _BATCHMANAGER_H_
#define _BATCHMANAGER_H_

#include "RenderList.h"
#include "HashString.h"

#include <vector>
#include <memory>

class GeometryBatch;
class ComponentMeshRenderer;

class BatchManager
{
    typedef std::vector<std::unique_ptr<GeometryBatch> > BatchPool;

    BatchPool batches;

public:
    BatchManager();
    ~BatchManager();

    uint Add(const ComponentMeshRenderer* object, const HashString& tag);
    void Remove(const ComponentMeshRenderer* object);

    void UpdateModel(const NodeList& objects);
    void Render(const NodeList& objects, uint transformIndex, uint materialsIndex, uint instancesIndex, bool keepOrder);

    void FillBatchNames(std::vector<HashString>& names) const;
    void OnMaterialModified(UID materialID);
};

#endif /* _BATCHMANAGER_H_ */


