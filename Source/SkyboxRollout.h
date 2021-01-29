#ifndef __SKYBOXROLLOUT_H__
#define __SKYBOXROLLOUT_H__

#include <memory>

class Texture2D;
class Framebuffer;
class Skybox;

class SkyboxRollout
{
public:

    SkyboxRollout();
    ~SkyboxRollout();

    void DrawProperties(Skybox* skybox);

private:

    void TakeScreenshot(Skybox* skybox);

private:
    std::unique_ptr<Texture2D>   screenshot;
    std::unique_ptr<Framebuffer> screenshot_fb;

    std::unique_ptr<Texture2D>   postprocess;
    std::unique_ptr<Framebuffer> postprocess_fb;

    float                        azimuthal = 0.0f;
    float                        polar     = 0.0f;
};

#endif /* __SKYBOXROLLOUT_H__ */


