#ifndef _DEFERRED_RESOLVE_H_
#define _DEFERRED_RESOLVE_H_

#include <memory>

class Program;
class Framebuffer;

class DeferredResolvePass
{
    std::unique_ptr<Program> program;
public:
    DeferredResolvePass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
};

#endif /* _DEFERRED_RESOLVE_H_ */