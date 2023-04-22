#ifndef __SKYBOXROLLOUT_H__
#define __SKYBOXROLLOUT_H__

#include "SelectResourceDlg.h"
#include <memory>

class Texture2D;
class Framebuffer;
class IBLData;
class Postprocess;

class SkyboxRollout
{
public:
    enum CubemapType
    {
        Environment = 0,
        DiffuseIBL,
        PrefilteredIBL,
        Count
    };

public:

    SkyboxRollout();
    ~SkyboxRollout();

    void DrawProperties(IBLData* skybox);
    CubemapType getSelected() const { return selected;  }
    float getRoughness() const {return roughness;}

private:
    
    void TakeScreenshot(IBLData* skybox, CubemapType type);

private:

    std::unique_ptr<Texture2D>   screenshotTex;
    std::unique_ptr<Texture2D>   postprocessedTex;
    std::unique_ptr<Texture2D>   screenshotDepthTex;
    std::unique_ptr<Framebuffer> screenshot_fb;

    std::unique_ptr<Framebuffer> postprocess_fb;

    std::unique_ptr<Postprocess> postProcess;

    float                        azimuthal = 0.0f;
    float                        polar     = 0.0f;
    float                        roughness = 0.0f;
    CubemapType                  selected = Environment;
    SelectResourceDlg            selectTexture;
};


#endif /* __SKYBOXROLLOUT_H__ */


