#define CASCADE_COUNT 3

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec3 vertex_uv0;
layout(location = 5) in vec3 vertex_tangent;

uniform mat4 proj;
uniform mat4 view;

uniform mat4 light_proj[CASCADE_COUNT];
uniform mat4 light_view[CASCADE_COUNT];

struct VertexOut
{
    vec3 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

out VertexOut fragment;
out vec4 shadow_coord[3];

void main()
{
    fragment.position = vertex_position;
    fragment.normal   = vertex_normal;
    fragment.tangent  = vertex_tangent;
    fragment.uv0      = vertex_uv0;

    gl_Position = proj*view*vec4(fragment.position, 1.0);

    for(uint i=0; i<CASCADE_COUNT; ++i)
    {
        shadow_coord[i]    = light_proj[i]*light_view[i]*vec4(fragment.position, 1.0);
        shadow_coord[i]   /= shadow_coord[i].w;
        shadow_coord[i].xy = shadow_coord[i].xy*0.5+0.5;
    }
}

