#version 420 core

out vec2 v_texcoord;

in vec2 position;
in vec2 texcoord;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	v_texcoord = texcoord;
}
