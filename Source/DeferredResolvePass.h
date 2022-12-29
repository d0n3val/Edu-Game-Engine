#ifndef _DEFERRED_RESOLVE_H_
#define _DEFERRED_RESOLVE_H_

#include <memory>

class Program;
class Framebuffer;
class VertexArray;

class DeferredResolvePass
{
    std::unique_ptr<Program> program;
    std::unique_ptr<VertexArray> vao;

public:
    DeferredResolvePass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
    void bindShadows();
};

#endif /* _DEFERRED_RESOLVE_H_ */