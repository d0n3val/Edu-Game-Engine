#pragma once

class SpotShadowMapPass
{
    ShadowmapPass();
    ~ShadowmapPass();

    void updateRenderList();
    void execute();

private:
    void createFramebuffer(uint width, uint height);
    void createProgram();

};