#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE 
#include "tiny_gltf.h"

#include <memory>
#include <SDL_assert.h>

template <class T>
inline void loadAccessor(std::unique_ptr<T[]> &data, uint &count, const tinygltf::Model &model, int index)
{
    const tinygltf::Accessor &accessor = model.accessors[index];

    count = uint(accessor.count);
    data = std::make_unique<T[]>(count);

    const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
    const uint8_t *bufferData = reinterpret_cast<const uint8_t *>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
    size_t defaultStride = tinygltf::GetComponentSizeInBytes(accessor.componentType) * tinygltf::GetNumComponentsInType(accessor.type);
    SDL_assert(defaultStride == sizeof(T));
    size_t bufferStride = view.byteStride == 0 ? defaultStride : view.byteStride;

    for (uint i = 0; i < count; ++i)
    {
        data[i] = *reinterpret_cast<const T *>(bufferData);
        bufferData += bufferStride;
    }
}

template <>
inline void loadAccessor<unsigned>(std::unique_ptr<unsigned[]>& data, uint& count, const tinygltf::Model& model, int index)
{
    const tinygltf::Accessor& accessor = model.accessors[index];

    count = uint(accessor.count);
    uint numComponents = tinygltf::GetNumComponentsInType(accessor.type);
    uint componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
    data = std::make_unique<unsigned[]>(count* numComponents);

    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const uint8_t* bufferData = reinterpret_cast<const uint8_t*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
    size_t defaultStride = componentSize * numComponents;    
    size_t bufferStride = view.byteStride == 0 ? defaultStride : view.byteStride;

    switch (accessor.componentType)
    {
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
        {
            for (uint i = 0; i < count; ++i)
            {
                for (uint j = 0; j < numComponents; ++j)
                {
                    data[i*numComponents+j] = reinterpret_cast<const unsigned*>(bufferData)[j];
                }
                bufferData += bufferStride;
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
        {
            for (uint i = 0; i < count; ++i)
            {
                for (uint j = 0; j < numComponents; ++j)
                {
                    data[i * numComponents + j] = reinterpret_cast<const short*>(bufferData)[j];
                }

                bufferData += bufferStride;
            }
            break;
        }
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
        {
            for (uint i = 0; i < count; ++i)
            {
                for (uint j = 0; j < numComponents; ++j)
                {
                    data[i * numComponents + j] = reinterpret_cast<const uint8_t*>(bufferData)[j];
                }

                bufferData += bufferStride;
            }
            break;
        }
    }
}

template <class T>
inline void loadAccessor(std::unique_ptr<T[]>& data, uint& count, const tinygltf::Model& model, const std::map<std::string, int>& attributes, const char* accesorName)
{
    const auto& it = attributes.find(accesorName);
    if (it != attributes.end())
    {
        loadAccessor(data, count, model, it->second);
    }
    else
    {
        count = 0;
    }
}


