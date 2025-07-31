#version 450
#include "common.sp"


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;


layout(push_constant) uniform Push{

    mat4 modelMatrix;
}push;

void main() {
    vec4 world_position = push.modelMatrix * vec4(inPosition, 1.0);
    gl_Position =ubo.projection * ubo.view * world_position;
    fragColor = inColor;
    fragTexCoord =  inTexCoord;
}






