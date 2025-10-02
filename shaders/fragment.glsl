#version 330 core
out vec4 FragColor;

void main() {
    // round point sprite
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    if (dot(p,p) > 1.0) discard;
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
