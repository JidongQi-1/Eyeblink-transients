///////////// Do not touch the code below
uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object
//////////////////////////////////////////////////

// bit-stealing parameters
uniform float GB; // green-to-blue ratio
uniform float RB; // red-to-blue ratio

// grating parameters
uniform float transparency;
uniform float spatialFreq;
uniform float phase;
uniform float angle;
uniform float amplitude;
uniform float pixelAngle;

/////////////////////////////////// Bit Stealing /////////////////////////////
vec4 bitStealing(float grayImg) {
	// look up table for intermediate luminance levels
	int LUT[24] = {
		//R  G  B
		0, 0, 0,
		0, 0, 1,
		1, 0, 0,
		1, 0, 1,
		0, 1, 0,
		0, 1, 1,
		1, 1, 0,
		1, 1, 1,
	};
	
	// get the integer and decimal parts of desired luminance level
	grayImg = grayImg * 255.0;
	float intPart = floor(grayImg);
	float decPart = grayImg - intPart;
	
	// determine nearest value in LUT to decimal part
	float bestdf = 1;
	int bestidx = 0;
	for (int ii = 0; ii<8; ii++) {
		float dec = (LUT[3*ii] * RB + LUT[3*ii+1]*GB + LUT[3*ii+2]) / (RB + GB + 1.0);
		float df = abs(dec - decPart);
		if (df < bestdf) {
			bestdf = df;
			bestidx = ii;
		}
	}

	// set value for each color channel
	vec4 color;
	color.r = (intPart + LUT[3 * bestidx]) / 255.0;
	color.g = (intPart + LUT[3 * bestidx + 1])/255.0;
	color.b = (intPart + LUT[3 * bestidx + 2])/255.0;
	color.w = 1;
	return color;
}
/////////////////////////////////////////////////////////////////////////////


// Add your code here
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
	
	// Apply bit stealing
	vec4 colorbs = bitStealing(scale);
	
	// Modify color of object
	gl_FragColor.w = transparency;
	gl_FragColor.r = colorbs.r;
	gl_FragColor.g = colorbs.g;
	gl_FragColor.b = colorbs.b;
}
