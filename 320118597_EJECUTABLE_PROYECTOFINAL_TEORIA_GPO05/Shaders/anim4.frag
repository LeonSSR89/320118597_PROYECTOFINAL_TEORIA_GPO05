#version 330
in vec2 vUV;                // Coordenadas UV
out vec4 FragColor;         // Color final

uniform float uTime;         //  Tiempo global usado para animar el parpadeo
uniform vec3  uColor;        //  Color base del foco (rojo, azul, etc.)
uniform float uMinI;         //  Intensidad mínima del brillo
uniform float uMaxI;         //  Intensidad máxima del brillo
uniform float uSpeed;        //  Velocidad de parpadeo
uniform float uSoft;         //  Transición suave del borde cuando es radial
uniform int   uUseRadial;    //  Si es 1 usa forma circular, si es 0 llena todo el mesh


void main(){
//  Pulso senoidal 
    // Genera un valor oscilante entre 0 y 1 para variar la intensidad
    float s = 0.5 + 0.5 * sin(uTime * 6.28318 * uSpeed);
    float intensity = mix(uMinI, uMaxI, s);     // Interpola entre brillo mínimo y máximo

     float mask = 1.0;                        // Valor por defecto (sin máscara)
    if (uUseRadial == 1) {                   // Solo si está activado desde el C++
        vec2  c  = vUV - vec2(0.5);           // Centro del brillo en el medio (0.5,0.5)
        float r  = length(c);                // Calcula distancia desde el centro
        float r0 = 0.38;                     // Radio base del círculo brillante
        mask = 1.0 - smoothstep(r0, r0 + uSoft, r); // Suaviza el borde con uSoft
    }
         //  Color final 
    vec3 col = uColor * intensity * mask;
    FragColor = vec4(col, 1.0);
}
