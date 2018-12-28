layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;
layout(location = 5) in vec3 vertex_tangent;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

struct VertexOut
{
    vec2 uv0;
    vec3 normal;
    vec3 position;
    mat3 tbn;
};

out VertexOut fragment;

mat3 create_tbn(const vec3 normal, const vec3 tangent)
{
    vec3 ortho_tangent = normalize(tangent-dot(tangent, normal)*normal); // skinning forces this
    vec3 bitangent     = cross(normal, ortho_tangent);
    mat3 tbn           = mat3(tangent, bitangent, normal);

    return tbn;
}

void transform_output()
{
    fragment.position = (model*vec4(vertex_position, 1.0)).xyz;
    fragment.normal   = (model*vec4(vertex_normal, 0.0)).xyz;
    fragment.uv0      = vertex_uv0;
    fragment.tbn      = create_tbn(fragment.normal, (model*vec4(vertex_tangent, 0)).xyz);
}

void main()
{
    transform_output();

    gl_Position = proj*view*vec4(fragment.position, 1.0);
}
