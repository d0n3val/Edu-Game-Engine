#include "Globals.h"
#include "SkyboxRollout.h"

#include "Skybox.h"
#include "PostprocessShaderLocations.h"
#include "Application.h"
#include "ModulePrograms.h"
#include "ModuleRenderer.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ModuleHints.h"
#include "Postprocess.h"
#include "Resource.h"
#include "ResourceTexture.h"

#include "imgui/imgui.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

#include <assert.h>

#define SCREENSHOT_SIZE 192

SkyboxRollout::SkyboxRollout()
{
    screenshot_fb = std::make_unique<Framebuffer>();
    postprocess_fb = std::make_unique<Framebuffer>();

    screenshotTex  = std::move(std::unique_ptr<Texture2D>(new Texture2D(4, SCREENSHOT_SIZE, SCREENSHOT_SIZE, GL_RGBA16F, true)));
    environmentTex = std::move(std::unique_ptr<Texture2D>(Texture2D::CreateDefaultRGBA(SCREENSHOT_SIZE, SCREENSHOT_SIZE)));
    diffuseIBLTex = std::move(std::unique_ptr<Texture2D>(Texture2D::CreateDefaultRGBA(SCREENSHOT_SIZE, SCREENSHOT_SIZE)));
    prefilteredIBLTex = std::move(std::unique_ptr<Texture2D>(Texture2D::CreateDefaultRGBA(SCREENSHOT_SIZE, SCREENSHOT_SIZE)));
    screenshotDepthTex = std::make_unique<Texture2D>(4, SCREENSHOT_SIZE, SCREENSHOT_SIZE, GL_DEPTH24_STENCIL8, true);

    screenshot_fb->AttachColor(screenshotTex.get());
    screenshot_fb->AttachDepthStencil(screenshotDepthTex.get(), GL_DEPTH_ATTACHMENT);

    postProcess = std::make_unique<Postprocess>();
    postProcess->Init();

    assert(screenshot_fb->Check() == GL_FRAMEBUFFER_COMPLETE);
}

SkyboxRollout::~SkyboxRollout()
{
}

void SkyboxRollout::DrawProperties(Skybox* skybox)
{
    if (ImGui::CollapsingHeader("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
    {
        TakeScreenshot(skybox, Environment);
        TakeScreenshot(skybox, DiffuseIBL);
        TakeScreenshot(skybox, PrefilteredIBL);

        ResourceTexture* info = App->resources->GetTexture(skybox->GetCubemap());

        ImGui::BeginGroup();
        ImGui::Text("Texture:");
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

        ImGui::SameLine();
        std::string file;
        App->fs->SplitFilePath(info->GetFile(), nullptr, &file);
        ImGui::Text("%s", file.c_str());
        ImGui::PopStyleColor();

        bool mips = info->HasMips();
        if(ImGui::Checkbox("Mipmaps", &mips))
        {
            info->EnableMips(mips);
        }

        ImGui::SameLine();

        bool linear = !info->GetLinear();
        if(ImGui::Checkbox("sRGB", &linear))
        {
            info->SetLinear(!linear);
        }

        if(ImGui::SmallButton("Select Texture"))
        {
            selectTexture.Open(Resource::texture, "Skybox texture");
        }

        ImGui::EndGroup();

        ImGui::Separator();
        ImGui::Text("Texture");
        ImGui::SameLine(0, 152);
        ImGui::Text("Diffuse IBL");
        ImGui::SameLine(0, 127);
        ImGui::Text("Prefiltered IBL");
        ImGui::Image((ImTextureID)environmentTex->Id(), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 255));
        ImGui::SameLine();
        ImGui::Image((ImTextureID)diffuseIBLTex->Id(), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 255));
        ImGui::SameLine();
        ImGui::Image((ImTextureID)prefilteredIBLTex->Id(), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 255));
        ImGui::SliderFloat("Roughness", &roughness, 0.0, 1.0f);
        ImGui::SliderFloat("Azimuthal", &azimuthal, 0.0f , 2.0f*math::pi);
        ImGui::SliderFloat("Polar", &polar, -0.5f*math::pi, 0.5f*math::pi);
        ImGui::Text("Environment BRDF");
        if (skybox->GetEnvironmentBRDF())
        {
            ImGui::Image((ImTextureID)skybox->GetEnvironmentBRDF()->Id(), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 255));
        }
    }

    selectTexture.Display();

    if(selectTexture.HasSelection())
    {
        skybox->SetCubemap(selectTexture.GetResource());
        selectTexture.ClearSelection();
    }
}

void SkyboxRollout::TakeScreenshot(Skybox* skybox, ScreenshoType type)
{
	Frustum frustum;
	frustum.type  = FrustumType::PerspectiveFrustum;

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
    glViewport(0, 0, SCREENSHOT_SIZE, SCREENSHOT_SIZE);

    switch(type)
    {
        case Environment:
            skybox->Render(proj, view);
            postprocess_fb->ClearAttachments();
            postprocess_fb->AttachColor(environmentTex.get());
            break;
        case DiffuseIBL:
            skybox->DrawDiffuseIBL(proj, view);
            postprocess_fb->ClearAttachments();
            postprocess_fb->AttachColor(diffuseIBLTex.get());
            break;
        case PrefilteredIBL:
            skybox->DrawPrefilteredIBL(proj, view, roughness);
            postprocess_fb->ClearAttachments();
            postprocess_fb->AttachColor(prefilteredIBLTex.get());
            break;
    }

    assert(postprocess_fb->Check() == GL_FRAMEBUFFER_COMPLETE);

    postProcess->Execute(screenshotTex.get(), screenshotDepthTex.get(), postprocess_fb.get(), SCREENSHOT_SIZE, SCREENSHOT_SIZE);
}
