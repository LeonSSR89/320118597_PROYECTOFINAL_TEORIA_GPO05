#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;     // Usa la textura base
uniform vec3  uTint;        // tinte azul
uniform float uAlpha;       

void main() {
    vec3 base  = texture(uTex, vUV).rgb;
    vec3 color = base * uTint;      // azulado
    FragColor = vec4(color, uAlpha);
}
