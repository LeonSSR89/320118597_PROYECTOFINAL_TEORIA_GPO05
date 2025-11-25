#version 330
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;   // textura de la pantalla
uniform float uTime;      // tiempo global

void main() {
    vec3 base = texture(uTex, vUV).rgb;

    // brillo que sube y baja
    float intensity = 0.5 + 0.5 * sin(uTime * 2.0); // ajusta velocidad

    FragColor = vec4(base * intensity, 1.0);
}
