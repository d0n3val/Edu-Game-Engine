#version 440

#define PI 3.14159

in vec2 uv;
in vec3 normal;
in vec3 position;

out vec4 outColor;

layout (binding = 0 ) uniform sampler2D albedo_map;
layout (binding = 1 ) uniform sampler2D specular_map;

uniform vec3 light_dir;
uniform vec3 light_color;
uniform vec3 cam_pos;
uniform vec3 ambient_color;

vec3 schlick_fresnel(vec3 F0, float cos_angle)
{
    return F0+(1-F0)*pow(1-cos_angle, 5.0);
}

float sq(float val)
{
    return val*val;
}

float ggx_ndf(float roughness, float NdotH)
{
    float sq_rough = sq(roughness);
    float sq_ndoth = sq(NdotH);
    
    return sq_rough/(PI*sq(sq_ndoth*(sq_rough-1.0)+1.0));
}

float smith_vsf(float NdotL, float NdotV, float roughness)
{
    float GGXV = NdotL * (NdotV * (1.0 - roughness) + roughness);
    float GGXL = NdotV * (NdotL * (1.0 - roughness) + roughness);
    return 0.5 / max(0.00001, (GGXV + GGXL));
}

void main() 
{
   vec3 Cd          = pow(texture(albedo_map, uv).rgb, vec3(2.2));
   vec4 specular    = texture(specular_map, uv);
   vec3 Cs          = pow(specular.rgb, vec3(2.2));
   float smoothness = specular.a;
   float roughness  = sq(1-smoothness);
   
   // vectors
   vec3 L = -normalize(light_dir);  
   vec3 N = normalize(normal);
   vec3 V = normalize(cam_pos-position);
   vec3 H = normalize(L+V);


   float NdotL = max(0.0, dot(N, L));
   float NdotH = max(0.0, dot(N, H));
   float LdotH = max(0.0, dot(H, L));
   float NdotV = max(0.0, dot(N, V));

   // Fresnel   
   vec3 fresnel = schlick_fresnel(Cs, LdotH);
   
   // Visibility term
   float vsf    = smith_vsf(NdotL, NdotV, roughness);
     
   // NDF
   float ndf    = ggx_ndf(roughness, NdotH);
   
   // Rendering equation
   vec3 newCd  = Cd*(1-Cs);
   vec3 Lo     = ambient_color*Cd+(newCd+fresnel*ndf*vsf*0.25)*NdotL*light_color;
   
   // tone mapping + gamma correction
   Lo 		   = pow(Lo/(Lo+1.0), vec3(1/2.2));
      
   outColor    = vec4(Lo, 1.0);      
}