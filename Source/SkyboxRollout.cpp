#include "Globals.h"
#include "SkyboxRollout.h"

#include "IBLData.h"
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

const char* SelectionName[SkyboxRollout::Count] = { "Environment", "Diffuse", "Prefiltered" };

SkyboxRollout::SkyboxRollout()
{
    screenshot_fb = std::make_unique<Framebuffer>();
    postprocess_fb = std::make_unique<Framebuffer>();

    screenshotTex  = std::move(std::unique_ptr<Texture2D>(Texture2D::CreateDefaultRGBA(SCREENSHOT_SIZE, SCREENSHOT_SIZE)));
    postprocessedTex = std::move(std::unique_ptr<Texture2D>(Texture2D::CreateDefaultRGBA(SCREENSHOT_SIZE, SCREENSHOT_SIZE)));
    screenshotDepthTex = std::make_unique<Texture2D>(4, SCREENSHOT_SIZE, SCREENSHOT_SIZE, GL_DEPTH_COMPONENT, true);

    screenshot_fb->AttachColor(screenshotTex.get());
    screenshot_fb->AttachDepthStencil(screenshotDepthTex.get(), GL_DEPTH_ATTACHMENT);

    postprocess_fb->AttachColor(postprocessedTex.get());

    postProcess = std::make_unique<Postprocess>();
    postProcess->Init();


    assert(screenshot_fb->Check() == GL_FRAMEBUFFER_COMPLETE);
}

SkyboxRollout::~SkyboxRollout()
{
}

void SkyboxRollout::DrawProperties(IBLData* skybox)
{
    if (ImGui::CollapsingHeader("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
    {
        TakeScreenshot(skybox, selected);

        ResourceTexture* info = skybox->GetEnvironmentRes().GetPtr<ResourceTexture>();
        if (info)
        {
            ImGui::BeginGroup();
            ImGui::Text("Texture:");
            ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

            ImGui::SameLine();
            std::string file;
            App->fs->SplitFilePath(info->GetFile(), nullptr, &file);
            ImGui::Text("%s", file.c_str());
            ImGui::PopStyleColor();

            ImGui::SameLine();

            bool linear = info->GetColorSpace() == ColorSpace_linear;
            if (ImGui::Checkbox("sRGB", &linear))
            {
                info->SetColorSpace(linear ? ColorSpace_linear : ColorSpace_gamma);
            }

            if (ImGui::SmallButton("Select Texture"))
            {
                selectTexture.Open(Resource::texture, "Skybox texture", 0);
            }

            ImGui::EndGroup();
        }
        ImGui::Separator();
        ImGui::Combo("Cubemap", (int*)&selected, "Environment\0Diffuse\0Prefiltered\0");
        ImGui::Text(SelectionName[selected]);
        ImGui::Image((ImTextureID)size_t(postprocessedTex->Id()), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 255));
        ImGui::SliderFloat("Roughness", &roughness, 0.0, 1.0f);
        ImGui::SliderFloat("Azimuthal", &azimuthal, 0.0f , 2.0f*math::pi);
        ImGui::SliderFloat("Polar", &polar, -0.5f*math::pi, 0.5f*math::pi);
        ImGui::Text("Environment BRDF");
        if (skybox->GetEnvironmentBRDF() && selected == PrefilteredIBL)
        {
            ImGui::Image((ImTextureID)size_t(skybox->GetEnvironmentBRDF()->Id()), ImVec2(SCREENSHOT_SIZE, SCREENSHOT_SIZE), ImVec2(0, 1), ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 255));
        }
    }

    selectTexture.Display();

    if(selectTexture.HasSelection(0))
    {
        skybox->SetEnvironmentRes(selectTexture.GetResource());
        selectTexture.ClearSelection();
    }
}

void SkyboxRollout::TakeScreenshot(IBLData* skybox, CubemapType type)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "SkyboxRollout::TakeScreenshot");

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    switch(type)
    {
    case Environment:
        skybox->DrawEnvironment(proj, view);
        break;
    case DiffuseIBL:
        skybox->DrawDiffuseIBL(proj, view);
        break;
    case PrefilteredIBL:
        skybox->DrawPrefilteredIBL(proj, view, roughness);
        break;
    }

    assert(postprocess_fb->Check() == GL_FRAMEBUFFER_COMPLETE);

    postProcess->Execute(screenshotTex.get(), screenshotDepthTex.get(), postprocess_fb.get(), SCREENSHOT_SIZE, SCREENSHOT_SIZE);


    glPopDebugGroup();
}
