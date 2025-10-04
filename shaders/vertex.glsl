#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

uniform float uSphereRadius; // world/eye units
uniform float uPointScale; // // = framebufferHeight / (2*tan(fovY/2))

out vec3 vEyeCenter;
out float vRadius;


void main() {
   vec4 eye = uView * uModel * vec4(aPos, 1.0);
   vEyeCenter = eye.xyz;
   float sizePx = uPointScale * (uSphereRadius / -eye.z);
   sizePx = clamp(sizePx, 1.0, 200.0);
   gl_PointSize = sizePx;

   gl_Position = uProj * eye;
}
