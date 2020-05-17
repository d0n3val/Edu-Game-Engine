struct VertexOut
{
    vec3 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

struct Material
{
    sampler2DArray diffuse_map;
};

in VertexOut fragment;
in vec4 shadow_coord[3];
out vec4 color;

layout(location=0) uniform Material material;

void main()
{
    color = texture(material.diffuse_map, fragment.uv0);
}
