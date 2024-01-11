#include "Globals.h"
#include "ComponentTrail.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleHints.h"
#include "ModuleResources.h"
#include "ResourceTexture.h"

#include "DebugDraw.h"
#include "OpenGL.h"

#include "Leaks.h"

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

    uint max_num_billboards = uint(segments.size()-1)*(config_trail.NumAddVertices+1);
    uint max_num_vertices   = max_num_billboards*4;
    uint max_num_indices    = max_num_billboards*6;
    
    glBindBuffer(GL_ARRAY_BUFFER, render_buffers.vbo);
    if(max_num_vertices > render_buffers.reserved_vertices)
    {
        render_buffers.reserved_vertices = max_num_vertices;
        glBufferData(GL_ARRAY_BUFFER, render_buffers.reserved_vertices*sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    }

    Vertex* vertex_data = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0, max_num_vertices*sizeof(Vertex), GL_MAP_WRITE_BIT);

    uint vertex_idx = 0;
    float total_size = 0.0f;

    std::vector<SegmentInstance> instances;

    float3 prev_pos;

    for(uint i=0; i < segments.size(); ++i)
    {
        GetSegmentInfo(i, instances);

        if (i == 0)
        {
            prev_pos = instances.front().position;
        }

        for(const SegmentInstance& inst : instances)
        {
            total_size += inst.position.Distance(prev_pos);

            Vertex& vertex0 = vertex_data[vertex_idx++];
            Vertex& vertex1 = vertex_data[vertex_idx++];

            vertex0.pos = inst.position-inst.normal*config_trail.width*inst.size;
            vertex1.pos = inst.position+inst.normal*config_trail.width*inst.size;

            // accumulated size
            vertex0.color.x = vertex1.color.x = total_size;

            // accumulated life
            vertex0.color.w = vertex1.color.w = inst.life / config_trail.duration;

            vertex0.uv  = float2(total_size, 1.0f);
            vertex1.uv  = float2(total_size, 0.0f);

            prev_pos = inst.position;
        }
    }

    for (uint i = 0; i < vertex_idx; ++i)
    {

        float3 color;
        color_over_time.gradient.getColorAt(vertex_data[i].color.w, (float*)&color);            

        if (blend_mode == AdditiveBlend)
        {
            // Additive alpha lerp to black
            color = color.Lerp(float3(0.0f, 0.0f, 0.0f), 1.0f-vertex_data[i].color.w);
        }
            
        if (texture_mode == Stretch)
        {
            vertex_data[i].uv.x /= total_size;
        }

        vertex_data[i].color.x = color.x;
        vertex_data[i].color.y = color.y;
        vertex_data[i].color.z = color.z;
    }
       
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
    if(max_num_indices > render_buffers.reserved_indices)
    {
        render_buffers.reserved_indices = max_num_indices;
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned)*render_buffers.reserved_indices, nullptr, GL_DYNAMIC_DRAW);
    }

    unsigned* index_data = (unsigned*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, max_num_indices*sizeof(unsigned), GL_MAP_WRITE_BIT);

    uint index_idx   = 0;
    uint quad_idx    = 0;
    num_billboards   = (vertex_idx / 2 - 1);
    uint num_indices = num_billboards*6;

    // todo: stripes

    for(uint i=0; i< num_billboards; ++i)
    {
        index_data[index_idx++] = 0+quad_idx*2;
        index_data[index_idx++] = 1+quad_idx*2;
        index_data[index_idx++] = 2+quad_idx*2;
        index_data[index_idx++] = 2+quad_idx*2;
        index_data[index_idx++] = 1+quad_idx*2;
        index_data[index_idx++] = 3+quad_idx*2;

        ++quad_idx;
    }

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
            glBindTexture(GL_TEXTURE_2D, uint(tex_res->GetID()));
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

        glPointSize(0.6f);
        glBindVertexArray(render_buffers.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_buffers.ibo);
        glDrawElements(GL_TRIANGLES, num_billboards*6, GL_UNSIGNED_INT, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

}

void ComponentTrail::OnSave(Config& config) const 
{
	config.AddUID("Texture", texture);
	config.AddFloat("Duration", config_trail.duration);
	config.AddFloat("Min vertex distance", config_trail.min_vertex_distance);
	config.AddFloat("width", config_trail.width);
	config.AddFloat("Min Angle To Add Vertices", config_trail.MinAngleToAddVertices);
	config.AddUInt("Num Add Vertices", config_trail.NumAddVertices);
	config.AddBool("Linear", config_trail.linear);

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
    config_trail.MinAngleToAddVertices = config->GetFloat("Min Angle To Add Vertices", pi/24.0f);
    config_trail.NumAddVertices = config->GetUInt("Num Add Vertices", 4);
    config_trail.linear = config->GetBool("Linear", false);

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
    float4x4 transform = GetGameObject()->GetGlobalTransformation();
    segments.push_back(Segment(transform, config_trail.duration));
    segments.push_back(Segment(transform, config_trail.duration));
}

void ComponentTrail::OnStop() 
{
    segments.clear();
}

void ComponentTrail::OnUpdate(float dt) 
{
    // update life time

    for(Segment& segment : segments)
    {
        segment.life_time = std::max(segment.life_time-dt, 0.0f);
    }

    // erase segments not needed for Catmull-Rom
    for(uint i=0; i+2 < segments.size();)
    {
        if (segments[i].life_time <= 0.0f && segments[i+1].life_time <= 0.0f && segments[i+2].life_time <= 0.0f)
        {
            segments.erase(segments.begin()+i);
        }
        else
        {
            ++i;
        }
    }

    // put current head
    segments.back().transform = GetGameObject()->GetGlobalTransformation();

    float vertex_distance = segments[segments.size()-2].transform.TranslatePart().Distance(segments.back().transform.TranslatePart());

    if(vertex_distance > config_trail.min_vertex_distance)
    {
        // add another head
        segments.push_back(Segment(segments.back().transform, config_trail.duration));
    }
}

void ComponentTrail::OnDebugDraw(bool selected) const
{
    float prev_life = 0.0f;
    float prev_size = 0.0f;
    
    std::vector<SegmentInstance> instances;

    float3 prev_pos;

    for(uint i=0; i < segments.size(); ++i)
    {
        GetSegmentInfo(i, instances);

        if (!instances.empty())
        {
            if (i == 0)
            {
                prev_pos = instances.front().position;
            }

            const float3& position = instances.front().position;
            const float3& normal = instances.front().normal;
            dd::line(prev_pos, position, dd::colors::Blue, 0, false);
            float size_multiplier = size_over_time.Interpolate(1.0f - std::max(0.0f, instances.front().life) / config_trail.duration);
            dd::line(position - normal * config_trail.width * size_multiplier, position + normal * config_trail.width * size_multiplier, dd::colors::Blue, 0, false);
            prev_pos = position;
        }

        float3 color = dd::colors::Blue;

        for(uint i=1; i< instances.size(); ++i)
        {
            dd::point(instances[i].position, dd::colors::Red, 5.0f);
        }
    }

}

void ComponentTrail::GetSegmentInfo(uint index, std::vector<SegmentInstance>& instances) const
{

    Quat q0(segments[index].transform);
    Quat q1;

    bool add_vertices = segments.size() >= 2 && index +1 < segments.size();

    if(add_vertices)
    {
        q1 = Quat(segments[index+1].transform);

        if(q0.Dot(q1) < 0.0f)
        {
            q1 = q1.Neg();
        }

        float angle = q0.AngleBetween(q1);

        add_vertices = abs(angle) > config_trail.MinAngleToAddVertices;
    }

    float life0 = segments[index].life_time;
    float size0 = size_over_time.Interpolate(1.0f-std::max(0.0f, segments[index].life_time)/config_trail.duration);

    instances.clear();

    if(add_vertices)
    {
        float life1 = segments[index+1].life_time;
        float size1 = size_over_time.Interpolate(1.0f-std::max(0.0f, segments[index+1].life_time)/config_trail.duration); 

        // CatmullRom
        CubicSegment3 curve;
        CatmullRomFrom(index, curve);

        uint num_points = config_trail.NumAddVertices + 1;
        instances.resize(num_points);

        for(uint i=0; i<num_points; ++i)
        {
            float lambda          = float(i) / float(num_points-1);
            float3 point          = ApplyCurveSegment(curve, lambda);

            instances[i].position = point;

            Quat qi               = q0.Lerp(q1, lambda).Normalized();

            instances[i].normal   = qi * float3::unitZ; 
            instances[i].life     = life0*(1.0f-lambda)+life1*lambda;
            instances[i].size     = size0 * (1.0f - lambda) + size1 * lambda;
        }
    }    
    else
    {
        instances.resize(1);

        instances[0].position = segments[index].transform.TranslatePart();
        instances[0].normal   = segments[index].transform.Col3(2);
        instances[0].life     = life0;
        instances[0].size     = size0;
    }
}

void ComponentTrail::CatmullRomFrom(uint index, CubicSegment3& curve) const
{
    // positions
    float3 p0, p3;
    float3 p1 = segments[index].transform.TranslatePart();
    float3 p2 = segments[index+1].transform.TranslatePart(); 

    Quat q1(segments[index].transform);
    Quat q2(segments[index].transform);
    Quat qdiff = q1.Inverted() * q2;

    if (index > 0)
    {
        p0 = segments[index-1].transform.TranslatePart();
    }
    else
    {
        p0 = p1 -  qdiff.Inverted()*(p2 - p1);
    }

    if (index+2 < segments.size())
    {
        p3 = segments[index+2].transform.TranslatePart();
    }
    else
    {
        p3 = p2 + qdiff*(p2 - p1);
    }

    CentCatmullRom(p0, p1, p2, p3, curve, 1.0f, 0.0f);
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

