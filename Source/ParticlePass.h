#pragma once

#include <memory>

class Program;
class ComponentCamera;
class RenderList;
class Framebuffer;

class ParticlePass
{
    std::unique_ptr<Program> program;
public:

    ParticlePass();
    ~ParticlePass();

    void execute(const ComponentCamera* camera, const RenderList& objects, Framebuffer* frameBuffer, uint width, uint height);

private:

    void UseProgram();
    void GenerateProgram();
};