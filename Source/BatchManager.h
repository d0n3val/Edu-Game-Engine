#ifndef _BATCHMANAGER_H_
#define _BATCHMANAGER_H_

#include "RenderList.h"
#include "HashString.h"

#include <vector>
#include <memory>

class GeometryBatch;
class ComponentMeshRenderer;

enum BATCH_RENDER_FLAGS
{
    BR_FLAG_KEEP_ORDER = 1 << 0,
};

class BatchManager
{
    typedef std::vector<std::unique_ptr<GeometryBatch> > BatchPool;

    BatchPool batches;

public:
    BatchManager();
    ~BatchManager();

    uint Add(const ComponentMeshRenderer* object, const HashString& tag);
    void Remove(const ComponentMeshRenderer* object);

    void MarkForUpdate(const NodeList& objects);
    void DoUpdate();
    void DoRender(const NodeList& objects, uint flags);

    void FillBatchNames(std::vector<HashString>& names) const;
    void OnMaterialModified(UID materialID);
};

#endif /* _BATCHMANAGER_H_ */


