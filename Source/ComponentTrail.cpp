#include "Globals.h"
#include "ComponentTrail.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleHints.h"
#include "ModuleResources.h"
#include "ResourceTexture.h"

#include "DebugDraw.h"
#include "OpenGL.h"

#define MIN_VERTICES 64
#define MIN_INDICES 256
#define TEXTURE_MAP_LOC 0

ComponentTrail::ComponentTrail(GameObject* go) : Component(go, Types::Trail)
{
    render_buffers.reserved_vertices =  MIN_VERTICES;
    render_buffers.reserved_indices =  MIN_INDICES;

    unsigned vertex_size = sizeof(Vertex);

    glGenBuffers(1, &render_buffers.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, render_buffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, render_buffers.reserved_vertices*vertex_size, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &render_buffers.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*render_buffers.reserved_indices, nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if(render_buffers.vao == 0)
    {
        glGenVertexArrays(1, &render_buffers.vao);
    }

    glBindVertexArray(render_buffers.vao);

    glBindBuffer(GL_ARRAY_BUFFER, render_buffers.vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertex_size, (void*)(sizeof(float)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)(sizeof(float)*3+sizeof(float)*4));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ComponentTrail::~ComponentTrail()
{
    if(render_buffers.vbo != 0)
    {
        glDeleteBuffers(1, &render_buffers.vbo);
        render_buffers.vbo = 0;
    }

    if(render_buffers.ibo != 0)
    {
        glDeleteBuffers(1, &render_buffers.ibo);
        render_buffers.ibo = 0;
    }

    if(render_buffers.vao != 0)
    {
        glDeleteVertexArrays(1, &render_buffers.vao);
        render_buffers.vao = 0;
    }
}

void ComponentTrail::UpdateBuffers()
{
    assert(!segments.empty());
    uint num_vertices = segments.size()*4;
    uint num_indices = segments.size()*6;

    glBindBuffer(GL_ARRAY_BUFFER, render_buffers.vbo);
    if(num_vertices > render_buffers.reserved_vertices)
    {
        render_buffers.reserved_vertices = num_vertices;
        glBufferData(GL_ARRAY_BUFFER, render_buffers.reserved_vertices*sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    }

    Vertex* vertex_data = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, num_vertices*sizeof(Vertex), GL_MAP_WRITE_BIT);

    uint vertex_idx = 0;
    float3 prev0, prev1;
    float total_size = 0.0f;

    std::deque<Segment>::iterator prev = segments.begin();
    for(std::deque<Segment>::iterator it = segments.begin(); it != segments.end(); ++it)
    {
        if(it != prev)
        {
            float3 prev_pos = prev->transform.TranslatePart();
            float3 pos      = it->transform.TranslatePart();
            float3 front    = (pos-prev_pos); front.Normalize();
            float3 up       = it->transform.Col3(1);
            float3 right    = front.Cross(up); right.Normalize();
            float prev_size = size_over_time.Interpolate(1.0f-max(0.0f, prev->life_time)/config_trail.duration);
            float size      = size_over_time.Interpolate(1.0f-max(0.0f, it->life_time)/config_trail.duration);

            Vertex& vertex0 = vertex_data[vertex_idx++];
            Vertex& vertex1 = vertex_data[vertex_idx++];
            Vertex& vertex2 = vertex_data[vertex_idx++];
            Vertex& vertex3 = vertex_data[vertex_idx++];

            if(prev != segments.begin())
            {
                vertex0.pos = prev0;
                vertex1.pos = prev1;
            }
            else
            {
                if(prev->life_time <=0)
                {
                    float dif_time = (it->life_time-prev->life_time);
                    float3 dif_pos  = pos-prev_pos;
                    prev_pos = prev_pos+(dif_pos)*(-prev->life_time)/dif_time;
                }

                vertex0.pos = prev_pos-right*config_trail.width*prev_size;
                vertex1.pos = prev_pos+right*config_trail.width*prev_size;
            }

            prev0 = vertex2.pos = pos-right*config_trail.width*size;
            prev1 = vertex3.pos = pos+right*config_trail.width*size;

            color_over_time.gradient.getColorAt(1.0f-max(0.0f, prev->life_time)/config_trail.duration, (float*)&vertex0.color);
            color_over_time.gradient.getColorAt(1.0f-max(0.0f, it->life_time)/config_trail.duration, (float*)&vertex2.color);

            vertex1.color  = vertex0.color;
            vertex3.color  = vertex2.color;

            if(texture_mode == Stretch)
            {
                vertex0.uv  = float2(total_size, 1.0f);
                vertex1.uv  = float2(total_size, 0.0f);

                total_size += prev_pos.Distance(pos);

                vertex2.uv  = float2(total_size, 1.0f);
                vertex3.uv  = float2(total_size, 0.0f);
            }
            else
            {
                vertex0.uv  = float2(0.0f, 1.0f);
                vertex1.uv  = float2(0.0f, 0.0f);
                vertex2.uv  = float2(1.0f, 1.0f);
                vertex3.uv  = float2(1.0f, 0.0f);
            }
        }

        prev = it;
    }

    float3 prev_pos = prev->transform.TranslatePart();
    float4x4 trans  = GetGameObject()->GetGlobalTransformation();
    float3 pos      = trans.TranslatePart();
    float3 front    = (pos-prev_pos); front.Normalize();
    float3 up       = trans.Col3(1);
    float3 right    = front.Cross(up); right.Normalize();
    float prev_size = size_over_time.Interpolate(1.0f-max(0.0f, prev->life_time)/config_trail.duration);
    float size      = size_over_time.Interpolate(0.0f);

    Vertex& vertex0 = vertex_data[vertex_idx++];
    Vertex& vertex1 = vertex_data[vertex_idx++];
    Vertex& vertex2 = vertex_data[vertex_idx++];
    Vertex& vertex3 = vertex_data[vertex_idx++];

    if(prev != segments.begin())
    {
        vertex0.pos = prev0;
        vertex1.pos = prev1;
    }
    else
    {
        vertex0.pos = prev_pos-right*config_trail.width*prev_size;
        vertex1.pos = prev_pos+right*config_trail.width*prev_size;
    }

    prev0 = vertex2.pos = pos-right*config_trail.width*size;
    prev1 = vertex3.pos = pos+right*config_trail.width*size;

    color_over_time.gradient.getColorAt(1.0f-max(0.0f, prev->life_time)/config_trail.duration, (float*)&vertex0.color);
    color_over_time.gradient.getColorAt(0.0f, (float*)&vertex2.color);

    vertex1.color  = vertex0.color;
    vertex3.color  = vertex2.color;

    if(texture_mode == Stretch)
    {
        vertex0.uv  = float2(total_size, 1.0f);
        vertex1.uv  = float2(total_size, 0.0f);

        total_size += prev_pos.Distance(pos);

        vertex2.uv  = float2(total_size, 1.0f);
        vertex3.uv  = float2(total_size, 0.0f);

        for (uint i = 0; i < num_vertices; ++i)
        {
            vertex_data[i].uv.x /= total_size;
        }
    }
    else
    {
        vertex0.uv  = float2(0.0f, 1.0f);
        vertex1.uv  = float2(0.0f, 0.0f);
        vertex2.uv  = float2(1.0f, 1.0f);
        vertex3.uv  = float2(1.0f, 0.0f);
    }
       
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
    if(num_indices > render_buffers.reserved_indices)
    {
        render_buffers.reserved_indices = num_indices;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*render_buffers.reserved_indices, nullptr, GL_DYNAMIC_DRAW);
    }

    unsigned* index_data = (unsigned*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, num_indices*sizeof(unsigned), GL_MAP_WRITE_BIT);

    uint index_idx = 0;
    uint quad_idx = 0;

    prev = segments.begin();
    for(std::deque<Segment>::iterator it = segments.begin(); it != segments.end(); ++it)
    {
        if(it != prev)
        {
            index_data[index_idx++] = 0+quad_idx*4;
            index_data[index_idx++] = 1+quad_idx*4;
            index_data[index_idx++] = 2+quad_idx*4;
            index_data[index_idx++] = 2+quad_idx*4;
            index_data[index_idx++] = 1+quad_idx*4;
            index_data[index_idx++] = 3+quad_idx*4;

            ++quad_idx;
        }

        prev = it;
    }

    index_data[index_idx++] = 0+quad_idx*4;
    index_data[index_idx++] = 1+quad_idx*4;
    index_data[index_idx++] = 2+quad_idx*4;
    index_data[index_idx++] = 2+quad_idx*4;
    index_data[index_idx++] = 1+quad_idx*4;
    index_data[index_idx++] = 3+quad_idx*4;

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ComponentTrail::Draw()
{
    if(segments.size() > 1)
    {
        UpdateBuffers();

        const ResourceTexture* tex_res = static_cast<const ResourceTexture*>(App->resources->Get(texture));

        if(tex_res)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_res->GetID());
            glUniform1i(TEXTURE_MAP_LOC, 0);
        }

        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);

        glEnable(GL_BLEND);
        if(blend_mode == AdditiveBlend)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        }
        else
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        glBindVertexArray(render_buffers.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
        glDrawElements(GL_TRIANGLES, segments.size()*6, GL_UNSIGNED_INT, nullptr); 

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

#if 0
    std::deque<Segment>::iterator prev = segments.begin();
    for(std::deque<Segment>::iterator it = segments.begin(); it != segments.end(); ++it)
    {
        if(it != prev)
        {
            float3 prev_pos = prev->transform.TranslatePart();
            float3 pos = it->transform.TranslatePart();
            float3 front = (pos-prev_pos); front.Normalize();
            float3 up = it->transform.Col3(1);
            float3 right = front.Cross(up); right.Normalize();

            dd::line(prev_pos, pos, dd::colors::Blue, 0, false);

            dd::line(pos-right*config_trail.width, pos+right*config_trail.width,dd::colors::Blue, 0, false);
        }

        prev = it;
    }
#endif
}

void ComponentTrail::OnSave(Config& config) const 
{
	config.AddUID("Texture", texture);
	config.AddFloat("Duration", config_trail.duration);
	config.AddFloat("Min vertex distance", config_trail.min_vertex_distance);
	config.AddFloat("width", config_trail.width);

    config.AddArray("Color over time");

    const std::list<ImGradientMark*>& marks = color_over_time.gradient.getMarks();

    for(std::list<ImGradientMark*>::const_iterator it = marks.begin(), end = marks.end(); it != end; ++it)
    {
        Config mark;
        mark.AddBool("alpha", (*it)->alpha);
        if((*it)->alpha)
        {
            mark.AddFloat("color", (*it)->color[0]);
        }
        else
        {
            mark.AddFloat4("color", float4(((*it)->color)));
        }

        mark.AddFloat("position", (*it)->position);
        config.AddArrayEntry(mark);
    }

    config.AddFloat("Size init", size_over_time.init);
    config.AddFloat("Size end", size_over_time.end);
    config.AddFloat4("Size bezier", size_over_time.bezier);

    config.AddInt("Blend mode", (int)blend_mode);
    config.AddInt("Texture mode", (int)texture_mode);
}

void ComponentTrail::OnLoad(Config* config) 
{
    SetTexture(config->GetUID("Texture", 0));
	config_trail.duration = config->GetFloat("Duration", 0.5f);
	config_trail.min_vertex_distance = config->GetFloat("Min vertex distance", 0.1f);
	config_trail.width = config->GetFloat("width", 0.25f);

    color_over_time.gradient.clearMarks();

    uint count = config->GetArrayCount("Color over time");
    for(uint i=0; i< count; ++i)
    {
        Config mark = config->GetArray("Color over time", i);
        
        bool alpha = mark.GetBool("alpha", false);
        float position = mark.GetFloat("position", 0.0f); 
        if(alpha)
        {
            float color = mark.GetFloat("color", 1.0f); 
            color_over_time.gradient.addAlphaMark(position, color);
        }
        else
        {
            float4 color = mark.GetFloat4("color", float4::one); 
            color_over_time.gradient.addMark(position, ImColor(color.x, color.y, color.z, 1.0f));
        }
    }

    if(color_over_time.gradient.getMarks().empty())
    {
        color_over_time.gradient.addMark(0.0f, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
    }

    size_over_time.init = config->GetFloat("Size init", 1.0f);
    size_over_time.end = config->GetFloat("Size end", 1.0f);
    size_over_time.bezier = config->GetFloat4("Size bezier", float4(0.0f, 1.0f, 0.0f, 1.0f));

    blend_mode = (RenderBlendMode)config->GetInt("Blend mode", (int)AdditiveBlend);
    texture_mode = (TextureMode)config->GetInt("Texture mode", (int)Stretch);
}

void ComponentTrail::OnPlay()
{
    Segment segment;
    segment.transform = GetGameObject()->GetGlobalTransformation();
    segment.life_time = config_trail.duration;
	segments.push_back(segment);
}

void ComponentTrail::OnStop() 
{
    segments.clear();
}

void ComponentTrail::OnUpdate(float dt) 
{
    std::deque<Segment>::iterator prev = segments.begin();
    for(std::deque<Segment>::iterator it = segments.begin(); it != segments.end(); ++it)
    {
        it->life_time -= dt;

        if(it->life_time <= 0.0f)
        {
            if(prev != it && prev->life_time <= 0)
            {
                segments.erase(prev);
            }
        }

		prev = it;
    }

    if(prev != segments.end() && prev->life_time <= 0)
    {
        segments.erase(prev);
    }

    float4x4 transform = GetGameObject()->GetGlobalTransformation();
    float3 pos         = transform.TranslatePart();

    if(segments.empty() || segments.back().transform.TranslatePart().Distance(pos) > config_trail.min_vertex_distance)
    {
        segments.push_back(Segment(transform, config_trail.duration));
    }
}

void ComponentTrail::SetTexture(UID uid)
{
    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::texture)
    {
        if(res->LoadToMemory() == true)
        {
            texture = uid;
        }
    }
}

const ResourceTexture* ComponentTrail::GetTextureRes() const
{
	return static_cast<const ResourceTexture*>(App->resources->Get(texture));
}

ResourceTexture* ComponentTrail::GetTextureRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(texture));
}

