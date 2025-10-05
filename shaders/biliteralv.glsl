#version 330 core
out vec2 vUV;
const vec2 POS[3] = vec2[3]( vec2(-1,-1), vec2(3,-1), vec2(-1,3) );
const vec2 UV [3] = vec2[3]( vec2(0,0),   vec2(2,0),   vec2(0,2) );
void main(){
    gl_Position = vec4(POS[gl_VertexID], 0.0, 1.0);
    vUV = UV[gl_VertexID];
}
