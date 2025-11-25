#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

uniform float driftSpeed;   // 0.07f
uniform float rangeX;       // 3.5f
uniform float baseZ;        // 0.0f
uniform float pathWidth;    // 1.2f
uniform float bobAmp;       // 0.05f
uniform float bobFreq;      // 1.8f

void main()
{
    // movimiento automático en el propio VS (sin transformaciones CPU)
    float t = fract(time * driftSpeed);
    float xOffset = mix(-rangeX, rangeX, t);
    float zOffset = baseZ + sin(t * 6.2831853) * pathWidth;
    float yOffset = bobAmp * sin(time * bobFreq);

    vec3 displaced = aPos + vec3(xOffset, yOffset, zOffset);

    gl_Position = projection * view * model * vec4(displaced, 1.0);
}
