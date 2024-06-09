#pragma once

#include <memory>
#include <vector>

class Program;
class Framebuffer;
class Texture2D;
class VertexArray;

class DualKawaseBlur
{
    struct Intermediate
    {
        std::unique_ptr<Framebuffer> fb;
        std::unique_ptr<Texture2D>   texture;
        uint width;
        uint height;

        
        Intermediate() = default;
        Intermediate(Intermediate&& rhs) 
        {
            fb.swap(rhs.fb);
            texture.swap(rhs.texture);
            std::swap(width, rhs.width);
            std::swap(height, rhs.height);
        }

    };

    std::unique_ptr<Program>     downscaleProg;
    std::unique_ptr<Program>     upscaleProg;
    std::unique_ptr<Framebuffer> resultFB;
    std::unique_ptr<Texture2D>   result;
    std::unique_ptr<VertexArray> vao;
    uint                         rWidth = 0;
    uint                         rHeight = 0;
    uint                         rInternal = 0;
    uint                         rFormat = 0;
    uint                         rType = 0;
    std::vector<Intermediate>    intermediates;

public:
    DualKawaseBlur();
    ~DualKawaseBlur();

    void execute(const Texture2D *input, uint internal_format, uint format, uint type, uint width, uint height, uint steps = 1);

    const Texture2D* getResult() const { return result.get(); }

private:
    void createFramebuffers(uint internal_format, uint format, uint type, uint width, uint height, uint steps);
    void createPrograms();

};