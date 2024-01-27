#version 330

uniform sampler2D texture1;

in vec2 texCoord;
out vec4 outColor;


void main()
{
    vec2 adjustedCoord = texCoord;
//    float screenAspectRatio = 1;
//    float textureAspectRatio = 1.227;
//
//    if (screenAspectRatio > textureAspectRatio)
//    {
//        float scale = screenAspectRatio / textureAspectRatio;
//        adjustedCoord.y = (texCoord.y - 0.5) * scale + 0.5;
//    }
//    else
//    {
//        float scale = textureAspectRatio / screenAspectRatio;
//        adjustedCoord.x = (texCoord.x - 0.5) * scale + 0.5;
//    }

    outColor = texture(texture1, adjustedCoord);
}
