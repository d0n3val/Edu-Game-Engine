#include "Globals.h"

#include "PerlinProperties.h"
#include "Perlin.h"
#include "OpenGL.h"

#include "imgui/imgui.h"

#define TEXTURE_SIZE 512

PerlinProperties::PerlinProperties()
{
    perlin_fb   = std::make_unique<Framebuffer>(); 

    perlin_text.reset(Texture2D::CreateDefaultRGBA(TEXTURE_SIZE, TEXTURE_SIZE, nullptr, false));

    perlin_fb->AttachColor(perlin_text.get());

    std::unique_ptr<Shader> perlin_vs(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl"));
    std::unique_ptr<Shader> perlin_fs(Shader::CreateFSFromFile("assets/shaders/perlinFS.glsl"));

    if(perlin_vs->Compiled() && perlin_fs->Compiled())
    {
        perlin_prog = std::make_unique<Program>(perlin_vs.get(), perlin_fs.get(), 2, "Perlin program");
    }

    timer.Start();
}

void PerlinProperties::Draw(FractalPerlin3D& info)
{
    // update animation time
	float dt = (float)timer.Read() / 1000.0f;
	timer.Start();

    frame = fmodf(frame+dt, info.strength);

    // generate perlin texture for current frame
    GenerateTexture(info);

    // show info
    if(ImGui::CollapsingHeader("Perlin noise", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Image((ImTextureID) perlin_text->Id(), ImVec2(92, 92), ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128));
        ImGui::SameLine();
        ImGui::PushItemWidth(125);
        ImGui::BeginGroup();
        ImGui::InputFloat("Strength", &info.strength, 0.01f);
        ImGui::InputFloat("Frequency", &info.frequency, 0.01f);
        ImGui::InputInt("Num Octaves", (int*)&info.octaves);
        ImGui::InputFloat("Freq. mult", &info.octave_mult, 0.01f);
        ImGui::InputFloat("Ampl. mult", &info.octave_scale, 0.01f);
        ImGui::EndGroup();
    }
}

void PerlinProperties::GenerateTexture(FractalPerlin3D& info)
{
    perlin_fb->Clear(TEXTURE_SIZE, TEXTURE_SIZE);

    perlin_prog->Use();

    perlin_prog->BindUniform(0, info.strength);
    perlin_prog->BindUniform(1, info.frequency);
    perlin_prog->BindUniform(2, (int)info.octaves);
    perlin_prog->BindUniform(3, info.octave_mult);
    perlin_prog->BindUniform(4, info.octave_scale);
    perlin_prog->BindUniform(5, frame);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    perlin_fb->Unbind();
    perlin_prog->Unuse();
}
