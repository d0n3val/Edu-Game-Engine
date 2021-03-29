#ifndef __SKYBOXROLLOUT_H__
#define __SKYBOXROLLOUT_H__

#include "SelectResourceDlg.h"
#include <memory>

class Texture2D;
class Framebuffer;
class Skybox;
class Postprocess;

class SkyboxRollout
{
public:

    SkyboxRollout();
    ~SkyboxRollout();

    void DrawProperties(Skybox* skybox);

private:

    void TakeScreenshot(Skybox* skybox);

private:
    std::unique_ptr<Texture2D>   screenshotTex;
    std::unique_ptr<Framebuffer> screenshot_fb;

    std::unique_ptr<Texture2D>   postprocessTex;
    std::unique_ptr<Framebuffer> postprocess_fb;

    std::unique_ptr<Postprocess> postProcess;

    float                        azimuthal = 0.0f;
    float                        polar     = 0.0f;
    SelectResourceDlg            selectTexture;
};


#endif /* __SKYBOXROLLOUT_H__ */


