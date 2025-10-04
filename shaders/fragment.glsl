#version 330 core
in vec3  vEyeCenter;
in float vRadius;
uniform mat4 uProj;

layout (location = 0) out vec4  oColor;     // shown on screen
layout (location = 1) out float oEyeDepth;  // R32F: positive distance from eye

void main() {
    // Round sprite
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(p, p);
    if (r2 > 1.0) discard;

    // Sphere normal in eye space (camera looks down -Z)
    float nz = -sqrt(max(0.0, 1.0 - r2));
    vec3  N  = vec3(p, nz);

    // Per-fragment sphere surface position in eye space
    vec3 posEye = vEyeCenter + N * vRadius;

    // Window-space depth for the real sphere surface
    vec4 clip = uProj * vec4(posEye, 1.0);
    float ndcZ = clip.z / clip.w;               // [-1,1]
    gl_FragDepth = ndcZ * 0.5 + 0.5;            // [0,1]

    // Outputs
    oColor    = vec4(1.0);          // debug white for now
    oEyeDepth = -posEye.z;          // positive distance in eye space
}
