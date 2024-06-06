#pragma once

#include <memory>
#include <vector>

class GeometryBatch;
class Buffer;

class BatchDrawCommands
{
    struct BatchData
    {
        std::unique_ptr<Buffer>  drawCommands;
        std::unique_ptr<Buffer>  drawCount;
        uint                     maxNumCommands = 0;

        BatchData() = default;
        BatchData(BatchData&& rhs)
        {
            drawCommands.swap(rhs.drawCommands);
            drawCount.swap(rhs.drawCount);
            maxNumCommands = rhs.maxNumCommands;
        }

        BatchData& operator=(BatchData&& rhs)
        {
            drawCommands.swap(rhs.drawCommands);
            drawCount.swap(rhs.drawCount);
            maxNumCommands = rhs.maxNumCommands;
        }
    };

    // TODO: Batch timestamps

    std::vector<BatchData> batches;

public:

    BatchDrawCommands();
    ~BatchDrawCommands();


    uint getNumBatches() const {return uint(batches.size());}
    void resize(uint batchCount) { batches.resize(batchCount);}

    void resizeBatch(uint index, uint maxNumCommads);
    void bindToPoints(uint index, uint commandPoint, uint countPoint);
    void bindIndirectBuffers(uint index);
    uint getMaxCommands(uint index) const {return batches[index].maxNumCommands;}
};
