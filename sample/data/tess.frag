#version 420 core

uniform mat4 matMVP;
uniform vec3 vLight;

uniform vec3 vLightColor = vec3(1,0.9,0.8);

in block
{
	vec3 texCoord;
} In;

layout(location = 0, index = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D coarse_height;
layout(binding = 1) uniform sampler2D diffuseMap;
layout(binding = 2) uniform sampler2D noise_height;
layout(binding = 3) uniform sampler2D detailMap;

void main()
{
	vec3 diffuseColor = texture(diffuseMap, In.texCoord.xy).rgb;
	vec3 detailColor = texture(detailMap, In.texCoord.xy*256).rgb;

	vec3 normal = texture(coarse_height, In.texCoord.xy).rgb;
	normal = normalize(normal* 2.0 - 1.0);	// scale back and normalize from (0,1) to (-1,1)

	vec3 diffuse = clamp( dot( normal, vLight ),0.0,1.0)*vLightColor;
	vec3 finalColor = (0.4 + diffuse)*diffuseColor*detailColor*1.5;

	//FragColor = vec4(normal, 1.0);
	FragColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);

	// draw the boarder of terrain grid
	if(In.texCoord.z==1.0f)
	   FragColor = vec4(1,0,0,1);	   
}
