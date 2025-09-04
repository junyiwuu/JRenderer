#version 450
#include "common.sp"  //where camera matrix


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;


layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 outBitangent;


layout(push_constant) uniform Push{
    mat4 modelMatrix;
}push;



void main() {
    vec4 world_position = push.modelMatrix * vec4(inPosition, 1.0);

    outWorldPos = world_position.xyz;
    outNormal = mat3(transpose(inverse(push.modelMatrix))) * inNormal;
    outTangent = mat3(transpose(inverse(push.modelMatrix))) * inTangent;
    outTangent = mat3(transpose(inverse(push.modelMatrix))) * inTangent;
    outUV =  inUV;


    gl_Position =ubo.projection * ubo.view * world_position;

}







