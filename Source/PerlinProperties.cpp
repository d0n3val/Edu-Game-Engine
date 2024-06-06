#include "Globals.h"

#include "PerlinProperties.h"
#include "Noise.h"
#include "OpenGL.h"

#include "imgui/imgui.h"

#include "Leaks.h"

#define TEXTURE_SIZE 512

PerlinProperties::PerlinProperties()
{
    perlin_fb   = std::make_unique<Framebuffer>(); 

    perlin_text.reset(Texture2D::CreateDefaultRGBA(TEXTURE_SIZE, TEXTURE_SIZE, nullptr, false));
    perlin_text2.reset(Texture2D::CreateDefaultRGBA(TEXTURE_SIZE, TEXTURE_SIZE, nullptr, false));
    perlin_text2->SetMinMaxFiler(GL_NEAREST, GL_NEAREST);

    perlin_fb->AttachColor(perlin_text.get());

    timer.Start();
}

void PerlinProperties::Draw(FractalNoiseCfg& info)
{
    // update animation time
	float dt = (float)timer.Read() / 1000.0f;
	timer.Start();

    if(info.duration > 0.0f)
    {
        frame = fmodf(frame+dt, info.duration);
    }
    else
    {
        frame = 0.0f;
    }

    // generate perlin texture for current frame
    GenerateTexture(info);

    // show info
    if(ImGui::CollapsingHeader("Perlin noise", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Image((ImTextureID) size_t(perlin_text->Id()), ImVec2(256, 256), ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128));
        ImGui::PushID("Perlin");
        ImGui::BeginGroup();
        ImGui::InputFloat("Duration", &info.duration, 0.01f);
        ImGui::InputFloat("Strength", &info.strength, 0.01f);
        ImGui::InputFloat("Frequency", &info.frequency, 0.01f);
        ImGui::InputInt("Num Octaves", (int*)&info.octaves);
        ImGui::InputFloat("Freq. mult", &info.octave_mult, 0.01f);
        ImGui::InputFloat("Ampl. mult", &info.octave_scale, 0.01f);
        ImGui::EndGroup();
        ImGui::PopID();
    }
}

void PerlinProperties::GenerateTexture(FractalNoiseCfg& info)
{
#if 0
    std::vector<unsigned char> tex_data(TEXTURE_SIZE * TEXTURE_SIZE * 4);
    for (uint j = 0; j < TEXTURE_SIZE; ++j)
    {
        for (uint i = 0; i < TEXTURE_SIZE; ++i)
        {
            float3 v(float(i) / float(TEXTURE_SIZE), float(j) / float(TEXTURE_SIZE), frame);
            float f = Clamp(FractalNoise(info, v) * 0.5f + 0.5f, 0.0f, 1.0f);
            tex_data[(j * TEXTURE_SIZE + i) * 4 + 0] = (unsigned char)(f * 255);
            tex_data[(j * TEXTURE_SIZE + i) * 4 + 1] = (unsigned char)(f * 255);
            tex_data[(j * TEXTURE_SIZE + i) * 4 + 2] = (unsigned char)(f * 255);
            tex_data[(j * TEXTURE_SIZE + i) * 4 + 3] = 255;
        }
    }

    perlin_text2->SetDefaultRGBAData(TEXTURE_SIZE, TEXTURE_SIZE, tex_data.data());
#endif

    //perlin_fb->Clear(TEXTURE_SIZE, TEXTURE_SIZE);
    perlin_fb->Bind();
    glViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);

    if (!perlin_prog)
    {
        std::unique_ptr<Shader> perlin_vs(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl"));
        std::unique_ptr<Shader> perlin_fs(Shader::CreateFSFromFile("assets/shaders/perlinFS.glsl"));

        if (perlin_vs->Compiled() && perlin_fs->Compiled())
        {
            perlin_prog = std::make_unique<Program>(perlin_vs.get(), perlin_fs.get(), "Perlin program");
        }
    }

    perlin_prog->Use();

    perlin_prog->BindUniform(0, info.duration);
    perlin_prog->BindUniform(1, info.strength);
    perlin_prog->BindUniform(2, info.frequency);
    perlin_prog->BindUniform(3, (int)info.octaves);
    perlin_prog->BindUniform(4, info.octave_mult);
    perlin_prog->BindUniform(5, info.octave_scale);
    perlin_prog->BindUniform(6, frame);

#if 0
    perlin_prog->BindTexture(7, 0, perlin_text2.get());
#endif

    glDrawArrays(GL_TRIANGLES, 0, 3);

    perlin_fb->Unbind();
    perlin_prog->Unuse();

}
