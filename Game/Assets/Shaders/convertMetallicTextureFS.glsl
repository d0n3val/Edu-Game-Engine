in vec2 uv;

layout(location = 0) out vec4 diffuse;
layout(location = 1) out vec4 specular;
//layout(location = 2) out vec4 occlusion;


layout(location = 0) uniform sampler2D base_texture;
layout(location = 1) uniform sampler2D metallic_texture;

void main()
{
    vec4 base_color      = texture2D(base_texture, uv);
    vec4 metallic_color  = texture2D(metallic_texture, uv);

    float metallic  = metallic_color.r;

    if(base_color.a < 0.01)
    {
        diffuse   = vec4(0.0);
        specular  = vec4(0.0);
        //occlusion = vec4(0.0);
    }
    else
    {
        diffuse    = vec4(base_color.rgb*(1-metallic), base_color.a);
        specular   = vec4(mix(vec3(0.04), base_color.rgb, metallic), metallic_color.a);
        //occlusion  = vec4(vec3(metallic_color.a), 1.0);
    }

    diffuse.rgb   = pow(diffuse.rgb, vec3(1.0/2.2));
    specular.rgb   = pow(specular.rgb, vec3(1.0/2.2));
    //occlusion.rgb   = pow(occlusion.rgb, vec3(1.0/2.2));
}
