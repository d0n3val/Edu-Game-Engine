#ifndef _FOG_GLSL_
#define _FOG_GLSL_

vec3 applyFog( in vec3  rgb, in float dist, in vec3 rayOrig, in vec3 rayDir) 
{
    float b = 0.2;
    float a = 0.02;
    float fogAmount = (a/b) * exp(-rayOrig.y*b) * (1.0-exp( -dist*rayDir.y*b ))/rayDir.y;
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( rgb, fogColor, fogAmount );
}

#endif /* _FOG_GLSL_ */