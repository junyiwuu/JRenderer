#version 450


layout (set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push{
    vec3 offset;
    vec3 color;
}push;

void main() {
    outColor = texture(texSampler, fragTexCoord) + vec4(push.color, 0.0);
}
