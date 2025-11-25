#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTexA;   // canal A
uniform sampler2D uTexB;   // canal B
uniform float uTime;

// parámetros sencillos
uniform float uSpeed;      // velocidad del barrido
uniform float uWidth;      // ancho del barrido
uniform float uScan;       // intensidad scanlines
uniform float uDist;       // distorsión UV
uniform float uFlick;      // flicker

void main()
{
    // distorsión muy simple
    vec2 uv = vUV;
    uv.x += sin(uTime * 2.0 + uv.y * 8.0) * uDist;
    uv.y += sin(uTime * 1.2 + uv.x * 6.0) * uDist;

    //  BARRIDO A->B 
    float pos = fract(uTime * uSpeed); // avanza 0→1
    float start = pos - uWidth * 0.5;
    float end   = pos + uWidth * 0.5;

    // máscara suave con smoothstep
    float mask = smoothstep(start, pos, uv.x) * (1.0 - smoothstep(pos, end, uv.x));

    vec3 colA = texture(uTexA, uv).rgb;
    vec3 colB = texture(uTexB, uv).rgb;

    vec3 col = mix(colA, colB, mask);

    // scanlines
    float s = sin(uv.y * 900.0 + uTime * 6.28);
    col *= mix(1.0, 0.5 + 0.5 * s, uScan);

    // flicker sencillo
    float flick = 1.0 + uFlick * sin(uTime * 50.0);
    col *= flick;

    FragColor = vec4(col, 1.0);
}
