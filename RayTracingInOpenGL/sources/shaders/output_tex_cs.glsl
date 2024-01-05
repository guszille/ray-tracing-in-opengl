#version 460 core

layout (local_size_x = 10, local_size_y = 10, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D uImageOutput;
layout (location = 0) uniform float uTime;

void main()
{
	ivec2 texelCoords = ivec2(gl_GlobalInvocationID.xy);
	float imageWidth = 1000;
	float speed = 100;

	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
	value.x = mod(float(texelCoords.x) + uTime * speed, imageWidth) / (gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	value.y = float(texelCoords.y) / (gl_NumWorkGroups.y * gl_WorkGroupSize.y);

	imageStore(uImageOutput, texelCoords, value);
}
