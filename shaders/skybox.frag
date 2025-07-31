#version 450

layout(set = 1, binding = 0) uniform samplerCube m_samplerCubemap;

layout(location=0) in vec3 dir;
layout (location = 0) out vec4 outColor;

void main(){
    // Use actual cubemap texture
    outColor = texture(m_samplerCubemap, dir);
}




// oid main(){
//     // Debug: mix texture with color to see if texture is working
//     vec4 texColor = texture(m_samplerCubemap, dir);
    
//     // If texture is black/empty, we'll see green. If texture works, we'll see the texture.
//     outColor = mix(vec4(0.0, 1.0, 0.0, 1.0), texColor, 0.8); // 80% texture, 20% green
// }