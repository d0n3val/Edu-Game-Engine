in vec2 uv;
out vec4 color;

layout(location = 0) uniform sampler2D tex;

void main()
{
    color = texture2D(tex, uv);
}
