#pragma once

#include <memory>

class Program;
class Framebuffer;
class Texture2D;
class VertexArray;

class KawaseBlur
{
    std::unique_ptr<Program>     program;
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<VertexArray> vao;


public:
    KawaseBlur();
    ~KawaseBlur();

    void execute(const Texture2D *input, const Texture2D* output, uint inMip, uint inWidth, uint inHeight, uint outMip, uint outWidth, uint outHeight, uint step);

private:
    void createProgram();

};