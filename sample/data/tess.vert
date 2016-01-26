#version 420 core

uniform ivec2 scalar;
uniform int size;

layout(binding = 0) uniform sampler2D coarse_height;

out block
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
} Out;

void main()
{	
    // create terrain grid position using gl_VertexID. 
	int halfSize = 0;//size/2;
	float unit = 1.0f/(size-1);
	int z = gl_VertexID /size;
	int x = gl_VertexID %size;
	Out.texCoord = vec2(x*unit,1-z*unit);
	
	vec4 NH = texture2D(coarse_height, Out.texCoord);

    Out.position = vec3((x-halfSize)*scalar.x, scalar.y*NH.a, (z-halfSize)*scalar.x);
    Out.normal = normalize(NH.xyz* 2.0 - 1.0);   
}
