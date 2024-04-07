#pragma once

#include <memory>

class Program;
class Framebuffer;
class Texture2D;
class VertexArray;

class DualKawaseBlur
{
    std::unique_ptr<Program>     downscaleProg;
    std::unique_ptr<Program>     upscaleProg;
    std::unique_ptr<Framebuffer> intermediateFB;
    std::unique_ptr<Texture2D>   intermediate;
    std::unique_ptr<Framebuffer> resultFB;
    std::unique_ptr<Texture2D>   result;
    std::unique_ptr<VertexArray> vao;
    uint                         rWidth = 0;
    uint                         rHeight = 0;
    uint                         rInternal = 0;
    uint                         rFormat = 0;
    uint                         rType = 0;
    uint                         rSteps = 0;

public:
    DualKawaseBlur();
    ~DualKawaseBlur();

    void execute(const Texture2D *input, uint internal_format, uint format, uint type, uint width, uint height);

    const Texture2D* getResult() const { return result.get(); }

private:
    void createFramebuffers(uint internal_format, uint format, uint type, uint width, uint height);
    void createPrograms();

};