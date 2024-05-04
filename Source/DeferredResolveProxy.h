#pragma once

#include <memory>

class Program;
class Framebuffer;
class Buffer;

class DeferredResolveProxy
{
    std::unique_ptr<Program> sphereProgram;
    std::unique_ptr<Program> coneProgram;
    std::unique_ptr<Program> debugSphereProgram;
    std::unique_ptr<Program> debugConeProgram;
public:

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useSphereProgram();
    bool generateSphereProgram();
    void useConeProgram();
    bool generateConeProgram();
    void useDebugSphere();
    bool generateDebugSphere();
    void useDebugCone();
    bool generateDebugCone();
};
