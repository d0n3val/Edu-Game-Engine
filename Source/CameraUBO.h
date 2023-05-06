#pragma once

#include <memory>

class ComponentCamera;
class Buffer;

class CameraUBO
{
    std::unique_ptr<Buffer> ubo;
public:
    CameraUBO();
    ~CameraUBO();

    void Update(ComponentCamera* camera);
    void Bind();
};