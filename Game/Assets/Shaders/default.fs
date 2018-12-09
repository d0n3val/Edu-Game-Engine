#define MAX_POINT_LIGHTS 8

//////////////////// STRUCTS ////////////////////////

struct Material
{
    sampler2D diffuse_map;
    vec4      diffuse_color;

    sampler2D specular_map;
    vec3      specular_color;
    float     shininess;

    sampler2D occlusion_map;

    sampler2D emissive_map;
    vec3      emissive_color;

    float     k_ambient;
    float     k_diffuse;
    float     k_specular;
};

struct AmbientLight
{
    vec3 color;
};

struct DirLight
{
    vec3 dir;
    vec3 color;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadartic;
};

struct Lights
{
    AmbientLight ambient;
    DirLight     directional;
    PointLight   point;
    uint         num_point;
};

//////////////////// UNIFORMS ////////////////////////

uniform Material     material;
uniform Lights       lights;
uniform mat4         view;


//////////////////// INPUTS ////////////////////////

in VertexOut
{

    vec2 uv0;
    vec3 normal;
    vec3 position;

} f_in;

//////////////////// OUTPUT ////////////////////////

out vec4 color;

//////////////////// FUNCTIONS ////////////////////////

vec4 get_diffuse_color(const Material mat, const vec2 uv)
{
    return texture(mat.diffuse_map, uv)*mat.diffuse_color;
}

vec4 get_specular_color(const Material mat, const vec2 uv)
{
    vec4 color = texture(mat.specular_map, uv);
    return vec4(color.rgb*mat.specular_color, max(color.a*mat.shininess*128.0f, 8.0f));
}

vec3 get_occlusion_color(const Material mat, const vec2 uv)
{
    return texture(mat.occlusion_map, uv).rgb;
}

vec3 get_emissive_color(const Material mat, const vec2 uv)
{
    return texture(mat.emissive_map, uv).rgb*mat.emissive_color;
}

float lambert(const DirLight light, const vec3 normal)
{
    return max(0.0, dot(normal, -light.dir));
}

float specular_blinn(const DirLight light, const vec3 pos, const vec3 normal, const vec3 view_pos, const float shininess)
{
    vec3 view_dir    = normalize(view_pos-pos);
    vec3 half_dir    = normalize(view_dir-light.dir);
    float sp         = max(dot(normal, half_dir), 0.0);

    return pow(sp, shininess); 
}


vec4 blinn(const vec3 pos, const vec3 normal, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    float diffuse  = lambert(lights.directional, normal);
    float specular = specular_blinn(lights.directional, pos, normal, view_pos, specular_color.a);

    return vec4(emissive_color+diffuse_color.rgb*(lights.ambient.color*occlusion_color*material.k_ambient)+
                lights.directional.color*(diffuse_color.rgb*diffuse*material.k_diffuse+
                                   specular_color.rgb*specular*material.k_specular),
                diffuse_color.a); 
}

void main()
{
    color = blinn(f_in.position, normalize(f_in.normal), f_in.uv0, transpose(mat3(view))*(-view[3].xyz), lights, material);
}
