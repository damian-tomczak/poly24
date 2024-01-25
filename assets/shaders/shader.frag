#version 330

uniform sampler2D texture1;

in vec2 texCoord;
out vec4 outColor;


void main()
{
    outColor = texture(texture1, texCoord);
}
