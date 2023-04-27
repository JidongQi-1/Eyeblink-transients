// Do not touch the code bellow
#version 430 core
#extension GL_EXT_gpu_shader4: enable

uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object

// Add your code here
uniform int R=255;
uniform int G;
uniform int B=255;

out vec4 fragmentColor;

void main(void)
{
	// Modify color of object
	//gl_FragColor.w = 1;
	//gl_FragColor.r = R/255.0;
	//gl_FragColor.g = G/255.0;
	//gl_FragColor.b = B/255.0;

	fragmentColor = vec4(R/255.0, 255/255.0, B/255.0, 1.0);
}
