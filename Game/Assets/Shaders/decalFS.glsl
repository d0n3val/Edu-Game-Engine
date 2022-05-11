#version 460
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"
#include "/shaders/LocationsAndBindings.h"

in vec4 clipping;

layout(binding = GBUFFER_POSITION_TEX_BINDING) uniform sampler2D positionTex;
layout(binding = DECAL_ALBEDO_TEX_BINDING) uniform sampler2D decalAlbedoTex;
layout(binding = DECAL_NORMAL_TEX_BINDING) uniform sampler2D decalNormalTex;
layout(binding = DECAL_SPECULAR_TEX_BINDING) uniform sampler2D decalSpecularTex;

layout(location = INV_MODEL_LOCATION) uniform mat4 invModel;
layout(location = NORMAL_STRENGTH_LOCATION) uniform float normalStrength;

layout(location = 0) out vec4 albedo;
layout(location = 1) out vec4 normal;
layout(location = 2) out vec4 specular;

void main()
{
    vec2 uv = (clipping.xy/clipping.w)*0.5+0.5;
    vec3 worldPos = texture(positionTex, uv).xyz;
    vec3 objPos   = (invModel*vec4(worldPos, 1.0)).xyz;

    vec3 dxWp = dFdx(worldPos);
    vec3 dyWp = dFdy(worldPos);

    // tangent space using derivatives
    vec3 boxNormal = normalize(vec3(invModel[0].z, invModel[1].z, invModel[2].z));
    vec3 geoNormal = normalize(cross(dxWp, dyWp));

    // Out of decal box
    if(abs(objPos.x) > 0.5 || abs(objPos.y) > 0.5 || abs(objPos.z) > 0.5 || dot(boxNormal, geoNormal) < 0.1)
    {
        discard;
    }

    vec2 texCoords   = objPos.xy+0.5;
    vec4 decalAlbedo = texture(decalAlbedoTex, texCoords);

    if(decalAlbedo .a < 0.5)
    {
        discard;
    }

    vec3 tangent = normalize(dxWp);
    vec3 bitangent   = normalize(dyWp);

    mat3 transform = mat3(tangent, bitangent, geoNormal);

    vec3 texNormal = texture(decalNormalTex, texCoords).xyz*2.0-1.0;
    texNormal.xy *= normalStrength;
    texNormal = normalize(texNormal);

    albedo.rgb = decalAlbedo.rgb;
    normal.xyz = (transform*texNormal)*0.5+0.5;
    specular = texture(decalSpecularTex, texCoords);
}