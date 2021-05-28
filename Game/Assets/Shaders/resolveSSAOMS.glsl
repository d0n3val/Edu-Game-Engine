#version 440

layout(location = 0) out vec4 position;
layout(location = 1) out vec4 normal;

in vec2 uv;

uniform sampler2DMS depths;
uniform sampler2DMS positions;
uniform sampler2DMS normals;

void main()
{
    ivec2 vp = textureSize(depths);
    vp = ivec2(vec2(vp)*uv);

    int minIndex = 0;
    float minDepth = texelFetch(depths, vp, 0).z;

    for(int i=1; i< 4; ++i)
    {
        float depth = texelFetch(depths, vp, i).z;
    
        if(depth < minDepth)
        {
            minIndex = i;
            minDepth = depth;
        }
    }

    position = texelFetch(positions, vp, minIndex);
    normal = texelFetch(normals, vp, minIndex);
}


