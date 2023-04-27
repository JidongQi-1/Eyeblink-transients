///////////// Do not touch the code below
uniform sampler2D texture_0; // Texture index 
uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object
//////////////////////////////////////////////////

// bit-stealing parameters
uniform float GB; // green-to-blue ratio
uniform float RB; // red-to-blue ratio

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
	// Get color from the image
	vec4 color = texture2D(texture_0, vec2(gl_TexCoord[0]));
	
	// Apply bit stealing
	vec4 colorbs = bitStealing(color.x);
	gl_FragColor.w = colorbs.w;
	gl_FragColor.r = colorbs.r;
	gl_FragColor.g = colorbs.g;
	gl_FragColor.b = colorbs.b;
}
