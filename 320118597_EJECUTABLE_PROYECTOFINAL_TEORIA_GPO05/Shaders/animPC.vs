#version 330
layout(location = 0) in vec3 aPos;          // Posición del vértice en el modelo
layout(location = 2) in vec2 aUV;           // Coordenadas UV del modelo

uniform mat4 model;                         // Matriz del modelo
uniform mat4 view;                          // Matriz de vista
uniform mat4 projection;                    // Matriz de proyección

out vec2 vUV;                               // Variable de salida que pasa las UV al fragment shader

void main() {
    vUV = aUV;                                                              // Paso las coordenadas UV sin modificar
    gl_Position = projection * view * model * vec4(aPos, 1.0);              // Calculo la posición final del vértice en pantalla
}
