#version 420 core
layout(binding = 0) uniform sampler2D inTex;

in vec2 v_texcoord;
out vec4 color;

uniform ivec2 scalar;

void main()
{
	float c = texture(inTex, v_texcoord).x; 

#if SOBEL
	float tl = textureOffset(inTex, v_texcoord, ivec2(-1,-1)).x; // top left
	float l  = textureOffset(inTex, v_texcoord, ivec2(-1, 0)).x; // left
	float bl = textureOffset(inTex, v_texcoord, ivec2(-1, 1)).x; // bottom left
	float t  = textureOffset(inTex, v_texcoord, ivec2( 0,-1)).x; // top
	float b  = textureOffset(inTex, v_texcoord, ivec2( 0, 1)).x; // bottom
	float tr = textureOffset(inTex, v_texcoord, ivec2( 1,-1)).x; // top right
	float r  = textureOffset(inTex, v_texcoord, ivec2( 1, 0)).x; // right
	float br = textureOffset(inTex, v_texcoord, ivec2( 1, 1)).x; // bottom right

	// Compute dx using Sobel:
	//           -1 0 1      
	//           -2 0 2     
	//           -1 0 1     
	float dX = tr + 2*r + br -tl - 2*l - bl;

	// Compute dy using Sobel:
	//           -1 -2 -1      
	//            0  0  0     
	//            1  2  1     
	float dY = bl + 2*b + br -tl - 2*t - tr;

	// normalize and scale from (-1,1) to (0,1) for texture output
	vec3 N = normalize(vec3(dX, 2.0/normalStrength, dY))*0.5 + 0.5;
#else
	float sleft		= textureOffset(inTex, v_texcoord, ivec2(-1, 0)).x; 
    float sright	= textureOffset(inTex, v_texcoord, ivec2( 1, 0)).x; 
    float sbottom	= textureOffset(inTex, v_texcoord, ivec2( 0,-1)).x; 
    float stop		= textureOffset(inTex, v_texcoord, ivec2( 0, 1)).x; 

    vec3 va = normalize(vec3(2.0, (sright-sleft)*scalar.y, 0.0)); 
    vec3 vb = normalize(vec3(0.0, (stop-sbottom)*scalar.y, -2.0)); 
    vec3 N = cross(va,vb)*0.5+0.5; 

#endif
	color = vec4(N, c);
}
