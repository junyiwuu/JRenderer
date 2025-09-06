#version 450
#include "common.sp"  //where camera matrix

layout (set = 2, binding = 0) uniform sampler2D albedoMap;
layout (set = 2, binding = 1) uniform sampler2D roughnessMap;
layout (set = 2, binding = 2) uniform sampler2D metallicMap;
layout (set = 2, binding = 3) uniform sampler2D normalMap;

layout (set = 1, binding = 1) uniform sampler2D samplerBRDFLUT;
layout (set = 1, binding = 2) uniform samplerCube samplerIrradiance;
layout (set = 1, binding = 3) uniform samplerCube prefilteredMap;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec4 outColor;


layout(push_constant) uniform Push{
    mat4 modelMatrix;
    vec3 baseColor;
    float roughness;
    float metallic;

    //flags
    int inputAlbedoPath;
    int inputRoughnessPath;
    int inputMetallicPath;
    int inputNormalPath;
}push;


//from https://github.com/SaschaWillems/Vulkan/blob/master/shaders/glsl/pbrtexture/pbrtexture.frag
#define PI 3.1415926535897932384626433832795
#define ALBEDO pow(texture(albedoMap, inUV).rgb, vec3(2.2))  //convert from srgb to linear

// From http://filmicgames.com/archives/75
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness)
{	
	float maxLod = float(textureQueryLevels(prefilteredMap) - 1.0f);
	float lod = roughness * maxLod;
	// const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	// float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
	vec3 b = textureLod(prefilteredMap, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products	
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness); 
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);		
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
		color += (kD * ALBEDO / PI + spec) * dotNL;
	}

	return color;
}

// vec3 calculateNormal()
// {
// 	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;
// 	tangentNormal = normalize(tangentNormal);
// 	// vec3 tangentNormal = normalize(vec3(0.f,0.f,0.f));

// 	vec3 N = normalize(inNormal);
// 	vec3 T = normalize(inTangent);

// 	if (length(T) < 1e-5) {
//         vec3 up = (abs(N.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
//         T = normalize(cross(up, N));}

// 	T = normalize(T - N * dot(N, T));
// 	vec3 B = normalize(cross(N, T));
// 	mat3 TBN = mat3(T, B, N);
// 	return normalize(TBN * tangentNormal);
// 	// return normalize(inNormal);
// }


// vec3 calculateNormal()
// {
// 	vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;

// 	vec3 N = normalize(inNormal);
// 	vec3 T = normalize(inTangent.xyz);
// 	vec3 B = normalize(cross(N, T));
// 	mat3 TBN = mat3(T, B, N);

// 	if(push.inputNormalPath == 1){
// 		return normalize(TBN * tangentNormal);
// 	}else{
// 		return N;
// 	}
// }
vec3 calculateNormal()
{
    vec3 N = normalize(inNormal);
    if (push.inputNormalPath == 0)
        return N;

    vec3 T = normalize(inTangent);
    T = normalize(T - N * dot(N, T));
    vec3 B = normalize(cross(N, T));
    vec3 tangentNormal = texture(normalMap, inUV).xyz * 2.0 - 1.0;
    return normalize(mat3(T, B, N) * tangentNormal);
}


void main()
{		
	vec3 N = calculateNormal();

	vec3 V = normalize(ubo.camPos - inWorldPos);
	vec3 R = reflect(-V, N); 



	vec3 albedo;
	if(push.inputAlbedoPath == 1){
		// albedo = pow(texture(albedoMap, inUV).rgb, vec3(2.2));
		albedo = texture(albedoMap, inUV).rgb; 
	}else{
		albedo = push.baseColor;
	}

	float metallic;
	if(push.inputMetallicPath == 1){
		metallic = texture(metallicMap, inUV).r;
	}else{
		metallic = push.metallic;
	}

	float roughness;
	if(push.inputRoughnessPath == 1){
		roughness = texture(roughnessMap, inUV).r;
	}else{
		roughness = push.roughness;
	}

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0.0);
    // no light for now
	// for(int i = 0; i < uboParams.lights[i].length(); i++) {
	// 	vec3 L = normalize(uboParams.lights[i].xyz - inWorldPos);
	// 	Lo += specularContribution(L, V, N, F0, metallic, roughness);
	// }   
	
	vec2 brdf = texture(samplerBRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb;	
	vec3 irradiance = texture(samplerIrradiance, N).rgb;

	// Diffuse based on irradiance
	vec3 diffuse = irradiance * albedo;	

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	vec3 specular = reflection * (F0 * brdf.x + brdf.y);


	// Ambient part (i dont use ao for now)
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;	  
	// vec3 ambient = (kD * diffuse + specular) * texture(aoMap, inUV).rrr;
    vec3 ambient = (kD * diffuse + specular);


	vec3 color = ambient + Lo;
	



	// Tone mapping
    //dont have exposure for now
	// color = Uncharted2Tonemap(color * uboParams.exposure);
    color = Uncharted2Tonemap(color);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	// Gamma correction
    //dont have gamma for now
	// color = pow(color, vec3(1.0f / uboParams.gamma));
	color = pow(color, vec3(1.0f / 2.2f)); // Standard sRGB gamma

	outColor = vec4(color, 1.0);
	// outColor = vec4(texture(samplerIrradiance, N).rgb, 1.0);
}


