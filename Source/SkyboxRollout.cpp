#include "Globals.h"
#include "SkyboxRollout.h"

#include "Skybox.h"
#include "PostprocessShaderLocations.h"
#include "Application.h"
#include "ModulePrograms.h"
#include "ModuleRenderer.h"
#include "ModuleHints.h"
#include "Postprocess.h"
#include "Resource.h"

#include "imgui/imgui.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

#include <assert.h>

#define SCREENSHOT_SIZE 256

SkyboxRollout::SkyboxRollout()
{
    screenshot_fb = std::make_unique<Framebuffer>();
    postprocess_fb = std::make_unique<Framebuffer>();

    screenshotTex  = std::move(std::unique_ptr<Texture2D>(new Texture2D(GL_TEXTURE_2D_MULTISAMPLE, 4, SCREENSHOT_SIZE, SCREENSHOT_SIZE, GL_RGBA16F, true)));
    postprocessTex = std::move(std::unique_ptr<Texture2D>(Texture2D::CreateDefaultRGBA(SCREENSHOT_SIZE, SCREENSHOT_SIZE)));

    screenshot_fb->AttachColor(screenshotTex.get());
    postprocess_fb->AttachColor(postprocessTex.get());

    postProcess = std::make_unique<Postprocess>();
    postProcess->Init();

    assert(screenshot_fb->Check() == GL_FRAMEBUFFER_COMPLETE);
    assert(postprocess_fb->Check() == GL_FRAMEBUFFER_COMPLETE);
}

SkyboxRollout::~SkyboxRollout()
{
}

void SkyboxRollout::DrawProperties(Skybox* skybox)
{
    if (ImGui::CollapsingHeader("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
    {
        TakeScreenshot(skybox);
        ImGui::Image((ImTextureID)postprocessTex->Id(), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128));

        ImGui::PushItemWidth(SCREENSHOT_SIZE);
        ImGui::SliderFloat("Azimuthal", &azimuthal, 0.0f , 2.0f*math::pi);
        ImGui::SliderFloat("Polar", &polar, -0.5f*math::pi, 0.5f*math::pi);
        ImGui::PushItemWidth(0.0f);
        if(ImGui::SmallButton("Select Texture"))
        {
            selectTexture.Open(Resource::texture, "Skybox texture");
        }
    }

    selectTexture.Display();

    if(selectTexture.HasSelection())
    {
        skybox->SetCubemap(selectTexture.GetResource());
        selectTexture.ClearSelection();
    }
}

void SkyboxRollout::TakeScreenshot(Skybox* skybox)
{
	Frustum frustum;
	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.pos   = float3::zero;
	frustum.front = -float3::unitZ;
	frustum.up    = float3::unitY;

	frustum.nearPlaneDistance = 0.1f;
	frustum.farPlaneDistance  = 100.0f;
	frustum.verticalFov       = math::pi/4.0f;
	frustum.horizontalFov     = 2.f * atanf(tanf(frustum.verticalFov * 0.5f) * float(SCREENSHOT_SIZE)/float(SCREENSHOT_SIZE));

	float4x4 proj = frustum.ProjectionMatrix();

    math::Quat rotAzimuthal(math::float3(0.0f, 1.0f, 0.0f), azimuthal);
    math::Quat rotPolar(math::float3(1.0f, 0.0f, 0.0f), polar);

    math::float4x4 view(rotAzimuthal*rotPolar);
    
    view.InverseOrthonormal();

    screenshot_fb->Bind();
    screenshot_fb->Clear(SCREENSHOT_SIZE, SCREENSHOT_SIZE);

    skybox->Draw(proj, view);
    postProcess->Execute(screenshotTex.get(), postprocess_fb.get(), SCREENSHOT_SIZE, SCREENSHOT_SIZE);
}
