// Do not touch the code bellow
uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object

// Add your code here
#define M_PI 3.1415926535897932384626433832795
uniform float transparency;
uniform float spatialFreq;
uniform float phase;
uniform float angle;
uniform float amplitude;
uniform float pixelAngle;

void main(void)
{
	// Get coordinate 
	float x = gl_FragCoord.x - (screenOffsetX + cX);
	float y = gl_FragCoord.y - (screenOffsetY + cY);
	
	x = x * pixelAngle / 60;
	y = y * pixelAngle / 60;
	
	// calculate sinusoidal grating
	float scale = (amplitude*cos(2 * 3.14159 * spatialFreq * 
		(x * cos(angle) + y * sin(angle))) + 1) / 2.0;
	
	// Modify color of object
	gl_FragColor.w = transparency;
	gl_FragColor.x = gl_Color.x * scale;
	gl_FragColor.y = gl_Color.y * scale;
	gl_FragColor.z = gl_Color.z * scale;
}
