#version 450
#include "common.sp"


layout(location=0) out vec3 dir;


//hardcode skybox
const vec3 pos[8] = vec3[8](
	vec3(-1.0,-1.0, 1.0),
	vec3( 1.0,-1.0, 1.0),
	vec3( 1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),

	vec3(-1.0,-1.0,-1.0),
	vec3( 1.0,-1.0,-1.0),
	vec3( 1.0, 1.0,-1.0),
	vec3(-1.0, 1.0,-1.0)
);

const int indices[36] = int[36](
	0, 1, 2, 2, 3, 0,	// front
	1, 5, 6, 6, 2, 1,	// right 
	7, 6, 5, 5, 4, 7,	// back
	4, 0, 3, 3, 7, 4,	// left
	4, 5, 1, 1, 0, 4,	// bottom
	3, 2, 6, 6, 7, 3	// top
);


// skybox needs to remove the translation from view matrix
void main(){
    int index = indices[gl_VertexIndex];
    vec4 clipPos = ubo.projection * mat4(mat3(ubo.view)) * vec4(pos[index], 1.0);
    
    // Force skybox to maximum depth (z = w) so it appears behind everything
    gl_Position = clipPos.xyww;
    
    dir = pos[index].xyz;
}