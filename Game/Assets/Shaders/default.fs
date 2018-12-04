//////////////////// STRUCTS ////////////////////////

struct Material
{
    sampler2D diffuse_map;
    vec4      diffuse_color;

    sampler2D specular_map;
    vec3      specular_color;
    float     shininess;

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

//////////////////// SUBROUTINES ////////////////////////

subroutine vec4 GetDiffuseColor(const Material mat, const vec2 uv);

//////////////////// UNIFORMS ////////////////////////

uniform Material     material;
uniform AmbientLight ambient;
uniform DirLight     directional;
uniform mat4         view;

layout(location=0) subroutine uniform GetDiffuseColor get_diffuse_color;

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

//////////////////// FUNCTIONS ////////////////////////

layout(index=0)subroutine(GetDiffuseColor) vec4 get_diffuse_from_color(const Material mat, const vec2 uv)
{
    return mat.diffuse_color;
}

layout(index=1)subroutine(GetDiffuseColor) vec4 get_diffuse_from_both(const Material mat, const vec2 uv)
{
    return texture(mat.diffuse_map, uv)*mat.diffuse_color;
}

layout(index=2)subroutine(GetDiffuseColor) vec4 get_diffuse_from_texture(const Material mat, const vec2 uv)
{
    return texture(mat.diffuse_map, uv);
}

vec3 get_specular_color(const Material mat, const vec2 uv)
{
    return max(texture(mat.specular_map, uv).rgb, mat.specular_color);
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


vec4 blinn(const vec3 pos, const vec3 normal, const vec2 uv, const vec3 view_pos, const AmbientLight ambient, const DirLight directional, const Material mat)
{
    float diffuse  = lambert(directional, normal);
    float specular = 0.0;

    if(diffuse > 0.0 && material.k_specular > 0.0 && material.shininess > 1.0)
    {
        specular = specular_blinn(directional, pos, normal, view_pos, material.shininess);
    }
    
    vec4 diffuse_color  = get_diffuse_color(material, uv);
    vec3 specular_color = get_specular_color(material, uv);

    return vec4(diffuse_color.rgb*(ambient.color*material.k_ambient+diffuse*material.k_diffuse)+
                specular_color*specular*material.k_specular,
                diffuse_color.a); 
}

void main()
{
    color = blinn(f_in.position, normalize(f_in.normal), f_in.uv0, transpose(mat3(view))*(-view[3].xyz), ambient, directional, material);
}
