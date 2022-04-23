#ifndef _DEFERRED_DECAL_PASS_H_
#define _DEFERRED_DECAL_PASS_H_

#include <memory>

class Program;
class Framebuffer;
class RenderList;
class Buffer;
class VertexArray;
class ComponentCamera;
class Texture2D;

class DeferredDecalPass
{
    std::unique_ptr<Program>     program;
    std::unique_ptr<Buffer>      vbo;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   worldPosTex;
    std::unique_ptr<Texture2D>   objPosTex;

public:
    DeferredDecalPass();

    void execute(ComponentCamera* camera, const RenderList& objects, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
    void generateCube();
    void generateFramebuffer(uint width, uint height);

};

#endif /* _DEFERRED_DECAL_PASS_H_ */
