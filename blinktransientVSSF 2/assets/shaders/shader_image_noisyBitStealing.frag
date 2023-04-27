#extension GL_EXT_gpu_shader4: enable

///////////// Do not touch the code below
uniform sampler2D texture_0; // Texture index 
uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object
//////////////////////////////////////////////////

#define MAX uint(65536)

uniform float alpha;	// transparentcy

// bit-stealing parameters
uniform float GB; // green-to-blue ratio
uniform float RB; // red-to-blue ratio

// noisy-bit parameters
uniform float rndseed1;		// can also be a timestamp
uniform float rndseed2;		// integer in range [0,65535]

////////////////// functions for generating pseudorandom values /////////////
uint hash( uint x )
{
	x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    // x -= (x/MAX) * MAX;
    x %= MAX;
    return x;
}
float Rand( uint x, uint y, uint seed1, uint seed2 ){ return float( ( seed2 + hash( seed1 ^ hash(x) ^ hash(y) ) ) % MAX ) / float(MAX); }
///////////////////////////////////////////////////////////////////////////

///////////////////////// Bit-Stealing and Noisy-Bit //////////////////////
vec4 noisyBitStealing(float grayImg, float rn) {
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
	
	// find the nearest value less than decPart, and the nearest value greater than decPart
	float bestdf_less = 1;
	int bestidx_less = 0;
	float dec_less = 0;
	
	float bestdf_more = 1;
	int bestidx_more = 7;
	float dec_more = 0;
	
	for (int ii = 0; ii<8; ii++) {
		float dec = (LUT[3*ii] * RB + LUT[3*ii+1]*GB + LUT[3*ii+2]) / (RB + GB + 1.0);
		float df = dec - decPart;
		
		if ((df > 0) && (df < bestdf_less)) {
			bestdf_less = df;
			bestidx_less = ii;
			dec_less = dec;
		}
		if ((df < 0) && (-df < bestdf_more)) {
			bestdf_more = -df;
			bestidx_more = ii;
			dec_more = dec;
		}
	}
	 
	// now apply dithering to the remaining portion of the decimal, normalized by the size of the step
	float remDec = (decPart - dec_less) / (dec_more - dec_less);
	
	float bestidx = bestidx_less;
	// if the remaining decimal is 0.3, show then show the low value 70% of the time
	if (rn < remDec) {
		bestidx = bestidx_less;
	} else { // and the high value 30% of the time
		bestidx = bestidx_more;
	}

	vec4 color;
	color.r = (intPart + LUT[3 * bestidx]) / 255.0;
	color.g = (intPart + LUT[3 * bestidx + 1])/255.0;
	color.b = (intPart + LUT[3 * bestidx + 2])/255.0;
	color.w = 1;
	return color;
}
///////////////////////////////////////////////////////////////////////////


// Add your code here
void main(void)
{
	// Get coordinate 
	float x = gl_FragCoord.x - (screenOffsetX + cX);
	float y = gl_FragCoord.y - (screenOffsetY + cY);
	
	// Get color from the image
	vec4 color = texture2D(texture_0, vec2(gl_TexCoord[0]));
	
	// Apply bit stealing
	vec4 colorbs = noisyBitStealing(color.r, Rand( uint(gl_FragCoord.x), uint(gl_FragCoord.y), uint(rndseed1), uint(rndseed2) ) );
	
	// Modify color of object
	gl_FragColor.w = alpha;
	gl_FragColor.r = colorbs.r;
	gl_FragColor.g = colorbs.g;
	gl_FragColor.b = colorbs.b;
}
