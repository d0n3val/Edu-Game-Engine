#define MAX_POINT_LIGHTS 4

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
    float quadric;
};

struct Lights
{
    AmbientLight ambient;
    DirLight     directional;
    PointLight   points[MAX_POINT_LIGHTS];
    uint         num_point;
};

//////////////////// UNIFORMS ////////////////////////

layout(location=0) uniform Material material;
layout(location=20) uniform Lights lights;
uniform vec3 view_pos;


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

float lambert(vec3 light_dir, const vec3 normal)
{
    return max(0.0, dot(normal, -light_dir));
}

float specular_blinn(vec3 light_dir, const vec3 pos, const vec3 normal, const vec3 view_pos, const float shininess)
{
    vec3 view_dir    = normalize(view_pos-pos);
    vec3 half_dir    = normalize(view_dir-light_dir);
    float sp         = max(dot(normal, half_dir), 0.0);

    return pow(sp, shininess); 
}

vec4 blinn(const vec3 pos, const vec3 normal, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    float diffuse  = lambert(lights.directional.dir, normal);
    float specular = specular_blinn(lights.directional.dir, pos, normal, view_pos, specular_color.a);
    vec3 color     = lights.directional.color*(diffuse_color.rgb*(diffuse*material.k_diffuse)+specular_color.rgb*(specular*material.k_specular));

    // \note: num_point best option use constant and black color for lights disabled?
    for(uint i=0; i < lights.num_point; ++i)
    {
        vec3 light_dir = pos-lights.points[i].position;
        float distance = length(light_dir);
        light_dir = light_dir/distance;
        float att = 1.0/(lights.points[i].constant+lights.points[i].linear*distance+lights.points[i].quadric*(distance*distance));

        diffuse  = att*lambert(light_dir, normal);
        specular = att*specular_blinn(light_dir, pos, normal, view_pos, specular_color.a);

        color += lights.points[i].color*(diffuse_color.rgb*(diffuse*material.k_diffuse)+specular_color.rgb*(specular*material.k_specular));
    }

    color += diffuse_color.rgb*(lights.ambient.color*occlusion_color*material.k_ambient);
    color += emissive_color;

    return vec4(color, diffuse_color.a); 
}

void main()
{
    color = blinn(f_in.position, normalize(f_in.normal), f_in.uv0, view_pos, lights, material);
}
