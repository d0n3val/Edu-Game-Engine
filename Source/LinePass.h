#pragma once

#include <memory>

class Program;
class RenderList;
class Framebuffer;
class ComponentCamera;

class LinePass
{
    std::unique_ptr<Program> program;
public:

    LinePass();
    ~LinePass();

    void execute(const ComponentCamera* camera, const RenderList& objects, Framebuffer* frameBuffer, uint width, uint height);
    
private:

    void UseProgram();
    void GenerateProgram();

};