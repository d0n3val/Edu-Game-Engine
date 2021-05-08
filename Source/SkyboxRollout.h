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
    enum ScreenshoType
    {
        Environment,
        DiffuseIBL,
        PrefilteredIBL
    };
    

    void TakeScreenshot(Skybox* skybox, ScreenshoType type);

private:

    std::unique_ptr<Texture2D>   screenshotTex;
    std::unique_ptr<Framebuffer> screenshot_fb;

    std::unique_ptr<Texture2D>   environmentTex;
    std::unique_ptr<Texture2D>   diffuseIBLTex;
    std::unique_ptr<Texture2D>   prefilteredIBLTex;
    std::unique_ptr<Framebuffer> postprocess_fb;

    std::unique_ptr<Postprocess> postProcess;

    float                        azimuthal = 0.0f;
    float                        polar     = 0.0f;
    SelectResourceDlg            selectTexture;
};


#endif /* __SKYBOXROLLOUT_H__ */


