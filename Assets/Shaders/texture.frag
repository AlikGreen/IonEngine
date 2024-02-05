#version 330 core
uniform sampler2D screenTexture;

out vec4 FragColor;

in vec2 fTexCoord;

void main()
{
    FragColor = texture(screenTexture, fTexCoord);
}