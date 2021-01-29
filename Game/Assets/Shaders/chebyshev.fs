#version 440 

out vec4 color;
in vec2 uv;

uniform sampler2D image;

void main()
{
    float depth = texture(image, uv).r-0.001;
    color = vec4(depth, depth*depth, 0.0, 1.0);
}


