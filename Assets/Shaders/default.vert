#version 330 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 fTexCoord;
out vec3 fNormal;
out vec3 fPos;

void main()
{
    vec4 pos = vec4(aPosition, 1.0) * model;
    gl_Position = pos * view * projection;
    fPos = pos.xyz;
    fTexCoord = aTexCoord;
    fNormal = aNormal;
}