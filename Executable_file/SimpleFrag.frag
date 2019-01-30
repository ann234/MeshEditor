#version 330 core

//	interpolated values from the vertex shaders
in vec3 fragmentColor;

//	Output data
out vec3 color;
uniform bool is_wire;	//	for wireframe rendering

void main()
{
	if(is_wire)
	{
		color = vec3(0.0, 0.0, 0.0);
		return;
	}
	// Output color = color specified in the vertex shader,
    // interpolated between all 3 surrounding vertices
	color = fragmentColor;
}