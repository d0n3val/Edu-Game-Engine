#define MAX_POINT_LIGHTS 4
#define MAX_SPOT_LIGHTS 4

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
    vec3 attenuation;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    vec3 color;
    float inner;
    float outter;
    vec3 attenuation;
};

struct Lights
{
    AmbientLight ambient;
    DirLight     directional;
    PointLight   points[MAX_POINT_LIGHTS];
    uint         num_point;
    SpotLight    spots[MAX_SPOT_LIGHTS];
    uint         num_spot;
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

vec3 directional_blinn(const vec3 pos, const vec3 normal, const vec3 view_pos, const DirLight light, const Material mat, 
                       const vec3 diffuse_color, const vec3 specular_color, float shininess)
{
    float diffuse  = lambert(lights.directional.dir, normal);
    float specular = specular_blinn(lights.directional.dir, pos, normal, view_pos, shininess);
    return lights.directional.color*(diffuse_color*(diffuse*material.k_diffuse)+specular_color*(specular*material.k_specular));
}

float attenuation(vec3 att, float distance)
{
    return 1.0/(att[0]+att[1]*distance+att[2]*distance*distance);
}

void point_blinn(const vec3 pos, const vec3 normal, const vec3 view_pos, const vec3 light_pos,
                 const vec3 light_color, const vec3 attenuation, const float shininess, 
                 out vec3 diffuse_color, out vec3 specular_color)
{
    vec3 light_dir = pos-light_pos;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;
    float att      = attenuation(attenuation, distance);

    float diffuse  = att*lambert(light_dir, normal);
    float specular = att*specular_blinn(light_dir, pos, normal, view_pos, shininess);

    diffuse_color += light_color*diffuse; 
    specular_color += light_color*specular;
}

float inside_cone(const vec3 light_dir, const vec3 cone_dir, const float inner, const float outer)
{
    float cos_a    = dot(light_dir, cone_dir);
    float len      = max(0.0001, inner-outer);

    return min(1.0, max(0.0, (cos_a-outer)/len));
}

vec3 spot_blinn(const vec3 pos, const vec3 normal, const vec3 view_pos, const SpotLight light, const Material mat,
                const vec3 diffuse_color, const vec3 specular_color, float shininess)
{
    vec3 light_dir = pos-light.position;
    float distance = length(light_dir);
    light_dir      = light_dir/distance;

    float cone     = inside_cone(light_dir, light.direction, light.inner, light.outter);
    float att      = attenuation(light.attenuation, distance);

    float diffuse  = lambert(light_dir, normal);
    float specular = specular_blinn(light_dir, pos, normal, view_pos, shininess);

    return (cone*att)*light.color*(diffuse_color*(diffuse*material.k_diffuse)+specular_color*(specular*material.k_specular));
}

void compute_point_lights(const vec3 pos, const vec3 normal, const vec3 view_pos, const Lights lights, 
                          const float shininess, out vec3 diffuse_color, out vec3 specular_color)
{
    for(uint i=0; i < lights.num_spot; ++i)
    {
        point_blinn(pos, normal, view_pos, lights.points[i].position, lights.points[i].color, 
                    lights.points[i].attenuation, shininess, diffuse_color, specular_color);
    }
}

vec3 compute_spot_lights(const vec3 pos, const vec3 normal, const vec3 view_pos, const Lights lights, const Material mat,
                         const vec3 diffuse_color, const vec3 specular_color, float shininess)
{
    vec3 color = vec3(0.0);

    for(uint i=0; i < lights.num_spot; ++i)
    {
        color += spot_blinn(pos, normal, view_pos, lights.spots[i], mat, diffuse_color, specular_color, shininess);
    }

    return color;
}

vec4 blinn(const vec3 pos, const vec3 normal, const vec2 uv, const vec3 view_pos, const Lights lights, const Material mat)
{
    vec4 diffuse_color   = get_diffuse_color(material, uv);
    vec4 specular_color  = get_specular_color(material, uv);
    vec3 occlusion_color = get_occlusion_color(material, uv);
    vec3 emissive_color  = get_emissive_color(material, uv);

    vec3 color = directional_blinn(pos, normal, view_pos, lights.directional, mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);

    vec3 dcolor = vec3(0.0);
    vec3 scolor = vec3(0.0);

    compute_point_lights(pos, normal, view_pos, lights, specular_color.a, dcolor, scolor);

    color += dcolor*diffuse_color.rgb*mat.k_diffuse+scolor*specular_color.rgb*mat.k_specular;

    color += compute_spot_lights(pos, normal, view_pos, lights, mat, diffuse_color.rgb, specular_color.rgb, specular_color.a);

    color += diffuse_color.rgb*(lights.ambient.color*occlusion_color*material.k_ambient);
    color += emissive_color;

    return vec4(color, diffuse_color.a); 
}

void main()
{
    color = blinn(f_in.position, normalize(f_in.normal), f_in.uv0, view_pos, lights, material);
}
