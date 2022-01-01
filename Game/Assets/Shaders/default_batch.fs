struct VertexOut
{
    vec3 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

struct TexHandle
{
    int index;
    float slice;
};

struct Material
{
    vec4      diffuseColor;
    vec4      specularColor;
    vec4      emissiveColor;
    vec2      uv_tiling;
    vec2      uv_offset;
    vec2      uv_secondary_tiling;
    vec2      uv_secondary_offset;
    float     smoothness;
    float     normalStrength;
    float     alphaTest;
    uint      mapMask;
    TexHandle diffuseMap;
    uint      padding0;
    uint      padding1;
};

layout(std430) buffer Materials
{
    Material materials[];
};

uniform sampler2DArray textures[64];

flat in int  draw_id;
in VertexOut fragment;
in vec4 shadow_coord[3];
out vec4 color;

layout(location=0) uniform Material material;

void main()
{
    Material mat = materials[draw_id];

    color.rgb = texture(textures[mat.diffuseMap.index], vec3(fragment.uv0.xy, mat.diffuseMap.slice)).rgb;
    color.a = 1.0;
}
