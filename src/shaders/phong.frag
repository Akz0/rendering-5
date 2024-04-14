#version 330 core

in vec2 TexCord;
in vec3 Normal;
in vec3 CurrentPosition;

uniform sampler2D baseMap;
uniform vec2 resolution;
uniform vec3 Color;

//Camera
uniform vec3 CameraPosition;

//Light Uniforms
uniform vec3 LightColor;
uniform vec3 LightPosition;
uniform float LightPower;

uniform float AmbientPower;
uniform vec3 AmbientColor;

uniform float DiffusePower;
uniform vec3 DiffuseColor;

uniform float SpecularPower;
uniform vec3 SpecularColor;

uniform vec3 MeshColor;
uniform float Roughness;

uniform float PointLightRadius;
out vec4 FragColor;


vec4 PointLight(float radius){
	vec3 LightVector = LightPosition - CurrentPosition;
	float distance = length(LightVector);

	float a = 1.1f;
	float b = 0.0f;

	float intensity = 1.0 / (a * distance * distance + b * distance + 1.0);

	vec3 normal = normalize(Normal);
	vec3 LightDirection = normalize(LightPosition - CurrentPosition);
	float diffuse = max(dot(normal, LightDirection), 0.0f);

	diffuse = max(diffuse,0.0);
	
	vec3 ViewDirection = normalize(CameraPosition - CurrentPosition);
	vec3 ReflectionDirection = reflect(-LightDirection, normal);

	vec3 halfway = normalize(ViewDirection + LightDirection);

	float SpecularAmount = pow(max(dot(normal,halfway),0.0f),16);
	float specular = SpecularAmount * SpecularPower;
 
	return (((diffuse * DiffusePower + vec4(AmbientPower) + vec4(intensity)) + specular) * vec4(LightColor, 1.0));
}

void main(){
    FragColor = vec4(Color,1.0f) * PointLight(PointLightRadius) * LightPower;
}