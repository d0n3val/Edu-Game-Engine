
#version 460

#extension GL_ARB_shading_language_include : require

#include "/shaders/LocationsAndBindings.h"
#include "/shaders/common.glsl"

layout(binding = FXAA_LDR_BINDING) uniform sampler2D ldrOutput;

// Trims the algorithm from processing darks.
//   0.0833 - upper limit (default, the start of visible unfiltered edges)
//   0.0625 - high quality (faster)
//   0.0312 - visible limit (slower)
const float CONTRAST_THRESHOLD = 0.0312;

// The minimum amount of local contrast required to apply algorithm.
//   0.333 - too little (faster)
//   0.250 - low quality
//   0.166 - default
//   0.125 - high quality 
//   0.063 - overkill (slower)
const float LOCAL_THRESHOLD = 0.063;

#define EDGE_STEP_COUNT 12
#define EDGE_STEPS 1, 1.5, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4

const float edgeSteps[EDGE_STEP_COUNT] = { EDGE_STEPS };

layout(location = FXAA_SUBPIXELBLENDING_LOCATION)uniform float subPixelBlending;

in vec2 uv;
out vec4 outColor;

struct LumNeighbours
{
    float center, north, south, east, west;
    float north_east, north_west, south_east, south_west;
    float highest, lowest, contrast;
};

struct EdgeData
{
    bool horizontal;
    vec2 pixelStep;
    float oppositeLuminance;
    float gradient;
};

float ComputePixelBlendFactor(in LumNeighbours luminance) 
{
    float avg = 2.0*(luminance.north+luminance.south, luminance.east, luminance.west);
    avg += (luminance.north_east+luminance.north_west+luminance.south_east+luminance.south_west);
    avg /= 12.0;

    float factor = smoothstep(0, 1, clamp(abs(luminance.center-avg)/luminance.contrast, 0.0, 1.0));

    return factor*factor*subPixelBlending;
}

bool IsHorizontalEdge(in LumNeighbours luminance)
{
    float edgeVert = abs(luminance.north_west-2.0*luminance.north+luminance.north_east)+
                     2.0*abs(luminance.west-2.0*luminance.center+luminance.east)+
                     abs(luminance.south_west-2.0*luminance.south+luminance.south_east);
    
    float edgeHorz = abs(luminance.north_west-2.0*luminance.west+luminance.south_west)+
                     2.0*abs(luminance.north-2.0*luminance.center+luminance.south)+
                     abs(luminance.north_east-2.0*luminance.east+luminance.south_east);

    return edgeHorz >= edgeVert;
}

void ComputeEdge(in LumNeighbours luminance, out EdgeData edge)
{
    edge.horizontal = IsHorizontalEdge(luminance);
    ivec2 textureSize = textureSize(ldrOutput, 0);

    if(edge.horizontal)
    {
        float pGradient = abs(luminance.north-luminance.center);
        float nGradient = abs(luminance.south-luminance.center);
        if(pGradient >= nGradient)
        {
            edge.pixelStep = vec2(0.0, 1.0/textureSize.y);
            edge.oppositeLuminance = luminance.north;
            edge.gradient = pGradient;
        }
        else 
        {
            edge.pixelStep = vec2(0.0, -1.0/textureSize.y);
            edge.oppositeLuminance = luminance.south;
            edge.gradient = nGradient;
        }
    }
    else
    {
        float pGradient = abs(luminance.east-luminance.center);
        float nGradient = abs(luminance.west-luminance.center);
        if(pGradient >= nGradient)
        {
            edge.pixelStep = vec2(1.0/textureSize.x, 0.0);
            edge.oppositeLuminance = luminance.east;
            edge.gradient = pGradient;
        }
        else
        {
            edge.pixelStep = vec2(-1.0/textureSize.x, 0.0);
            edge.oppositeLuminance = luminance.west;
            edge.gradient = nGradient;
        }
    }
}

float ComputeEdgeBlendFactor(out LumNeighbours luminance, EdgeData edge)
{
    vec2 uvEdge = uv;
    ivec2 textureSize = textureSize(ldrOutput, 0);
    vec2 edgeStep;

    if(edge.horizontal)
    {
        uvEdge.y += 0.5/textureSize.y;
        edgeStep = vec2(1.0/textureSize.x, 0.0);
    }
    else
    {
        uvEdge.x += 0.5/textureSize.x;
        edgeStep = vec2(0.0, 1.0/textureSize.y);
    }

    float edgeLuminance = (luminance.center+edge.oppositeLuminance)*0.5;
    float gradientThreshold = edge.gradient*0.25;

    vec2 puv = uvEdge;
    vec2 nuv = uvEdge;
    float pLuminanceDelta, nLuminanceDelta;

    bool pAtEnd = false;
    bool nAtEnd = false;

    // Iterate until find the edge end's (right and left or top and bottom)
    for (int i = 0; i < EDGE_STEP_COUNT; ++i) 
    {
        if(!pAtEnd)
        {
            puv += edgeStep*edgeSteps[i];
            pLuminanceDelta = textureLod(ldrOutput, puv, 0.0).g - edgeLuminance;
            pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;
        }

        if(!nAtEnd)
        {
            nuv -= edgeStep*edgeSteps[i];
            nLuminanceDelta = textureLod(ldrOutput, nuv, 0.0).g - edgeLuminance;
            nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;
        }

        if(pAtEnd && nAtEnd)
            break;
	}

    float pDistance, nDistance; 

    // compute the distances to the end of the edges
    if(edge.horizontal)
    {
        pDistance = puv.x-uvEdge.x; 
        nDistance = uvEdge.x-nuv.x;
    }
    else
    {
        pDistance = puv.y-uvEdge.y; 
        nDistance = uvEdge.y-nuv.y;
    }

    float shortest;
    bool deltaSign; 

    // compute the shortest distance
    if(pDistance < nDistance)
    {
        shortest = pDistance;
        deltaSign = pLuminanceDelta >= 0;
    }
    else
    {
        shortest = nDistance;
        deltaSign = nLuminanceDelta >= 0;
    }

    // We are solving only from white to black and not from black to white
    if(deltaSign == (luminance.center-edgeLuminance >= 0))
    {
        return 0;
    }

    return 0.5-shortest/(pDistance+nDistance);
}

void SampleLuminance(out LumNeighbours luminance)
{
    luminance.center = textureLod(ldrOutput, uv, 0.0).g;
    luminance.north = textureLodOffset(ldrOutput, uv, 0.0, ivec2(0, 1)).g;
    luminance.south = textureLodOffset(ldrOutput, uv, 0.0, ivec2(0, -1)).g;
    luminance.east = textureLodOffset(ldrOutput, uv, 0.0, ivec2(1, 0)).g;
    luminance.west = textureLodOffset(ldrOutput, uv, 0.0, ivec2(-1, 0)).g;

    luminance.north_east = textureLodOffset(ldrOutput, uv, 0.0, ivec2(1, 1)).g;
    luminance.north_west = textureLodOffset(ldrOutput, uv, 0.0, ivec2(-1, 1)).g;
    luminance.south_east = textureLodOffset(ldrOutput, uv, 0.0, ivec2(1, -1)).g;
    luminance.south_west = textureLodOffset(ldrOutput, uv, 0.0, ivec2(-1, -1)).g;

    luminance.highest = max(luminance.center, max(luminance.north, max(luminance.south, max(luminance.east, luminance.west))));
    luminance.lowest = min(luminance.center, min(luminance.north, min(luminance.south, min(luminance.east, luminance.west))));

    luminance.contrast = luminance.highest-luminance.lowest;
}

vec3 applyFXAA(in LumNeighbours luminance)
{
    float pixelBlendFactor = ComputePixelBlendFactor(luminance);

    EdgeData edge;
    ComputeEdge(luminance, edge);

    float edgeBlendFactor = ComputeEdgeBlendFactor(luminance, edge);

    return textureLod(ldrOutput, uv+edge.pixelStep*max(pixelBlendFactor, edgeBlendFactor), 0.0).rgb;
}

void main()
{
    LumNeighbours luminance;
    SampleLuminance(luminance);

    if(luminance.contrast < max(LOCAL_THRESHOLD*luminance.highest, CONTRAST_THRESHOLD))
    {
        outColor.rgb = textureLod(ldrOutput, uv, 0.0).rgb;
    }
    else
    {
        outColor.rgb = applyFXAA(luminance);
    }
}