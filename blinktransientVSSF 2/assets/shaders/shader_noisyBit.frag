//#version 130
#extension GL_EXT_gpu_shader4: enable

///////////// Do not touch the code below /////////
uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object
//////////////////////////////////////////////////

// Add your code here
#define M_PI 3.1415926535897932384626433832795
#define MAX uint(65536)

// noisy bit parameters
uniform float rndseed1;		// can also be a timestamp
uniform float rndseed2;		// integer in range [0,65535]
uniform float gray;	        // gray level to display


////////////////// functions for generating pseudorandom values /////////////
uint hash( uint x )
{
	x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    // x -= (x/MAX) * MAX;
    // x %= MAX;
    return x;
}
float Rand( uint x, uint y, uint seed1, uint seed2 )
{
uint   m = ( seed2 + hash( seed1 ^ hash(x) ^ hash(y) ) ); 
float  f = uintBitsToFloat( m );
return f / float(MAX); 
}
///////////////////////////////////////////////////////////////////////////

///////////////////////// noisy bit method ///////////////////////////////
float noisybit(float scale, float rn) {
	float scale255 = scale;// * 255;
	float scaleint = floor(scale255);
	float scaledec = scale255 - scaleint;
	
	float scaleuse;
	
	// for example, if desired value is 129.3, show 129 in 30% of frames and 130 in 70% of frames
	if (rn > scaledec) {
		scaleuse = scaleint / 255.0;
	} else {
		scaleuse = (scaleint + 1) / 255.0;
	}
	return scaleuse;
}
///////////////////////////////////////////////////////////////////////////

void main(void)
{
	// Get coordinate 
	float x = gl_FragCoord.x;
	float y = gl_FragCoord.y;

	// apply noisy-bit
	float scaleuse = noisybit( gray, Rand( uint(x), uint(y), uint(rndseed1), uint(rndseed2) ) );
	
	// Modify color of object
	gl_FragColor.w = 1;
	gl_FragColor.r = scaleuse;
	gl_FragColor.g = scaleuse;
	gl_FragColor.b = scaleuse;
}