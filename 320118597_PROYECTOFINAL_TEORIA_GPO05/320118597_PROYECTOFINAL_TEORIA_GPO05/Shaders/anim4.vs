#version 330
// sin layout(location)
// usa los atributos por defecto que ya usa tu Model: 0=pos, 2=uv
in vec3 aPos;
in vec2 aTex;       // Coordenadas UV del modelo

uniform mat4 model;         // Matriz del modelo
uniform mat4 view;          // Matriz de vista
uniform mat4 projection;    // Matriz de proyección

out vec2 vUV;           // Salida de coordenadas UV

void main(){
    vUV = aTex;
    gl_Position = projection * view * model * vec4(aPos, 1.0);          // Calculo posición final del vértice
}
