#version 330 core

out vec4 FragColor;

in vec4 ourColor;
in vec2 TexCoord;                         // <--- Entrada desde el Vertex Shader

uniform sampler2D uTexture;               // <--- La textura que viene de C++

void main() {
    // Multiplica la textura por el color base (o usa solo la textura)
    FragColor = texture(uTexture, TexCoord);
}