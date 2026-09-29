#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord; // <--- Nuevo atributo (Location 2)

out vec4 ourColor;
out vec2 TexCoord;                        // <--- Salida hacia el Fragment Shader

uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}