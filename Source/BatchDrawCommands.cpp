#include "Globals.h"

#include "BatchDrawCommands.h"
#include "GeometryBatch.h"

#include "OGL.h"
#include "OpenGL.h"

#include <SDL_assert.h>

BatchDrawCommands::BatchDrawCommands()
{

}

BatchDrawCommands::~BatchDrawCommands()
{

}

void BatchDrawCommands::resizeBatch(uint index, uint maxNumCommands)
{
    SDL_assert(index < batches.size());

    BatchData& data = batches[index];

    if (!data.drawCommands || data.maxNumCommands < maxNumCommands)
    {
        data.maxNumCommands = maxNumCommands;
        data.drawCommands = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawCommand) * data.maxNumCommands, nullptr, true);
    }

    if (!data.drawCount)
    {
        data.drawCount = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT, sizeof(DrawCommand) * data.maxNumCommands, nullptr, true);
    }
}

void BatchDrawCommands::bindToPoints(uint index, uint commandPoint, uint countPoint) const
{
    SDL_assert(index < batches.size());
    const BatchData& data = batches[index];

    *reinterpret_cast<int *>(data.drawCount->Map(GL_WRITE_ONLY)) = 0;
    data.drawCount->Unmap();

    data.drawCommands->BindToPoint(commandPoint);
    data.drawCount->BindToPoint(countPoint);
}

void BatchDrawCommands::bindIndirectBuffers(uint index) const 
{
    SDL_assert(index < batches.size());
    const BatchData& data = batches[index];

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, data.drawCommands->Id());
    glBindBuffer(GL_PARAMETER_BUFFER, data.drawCount->Id());
}