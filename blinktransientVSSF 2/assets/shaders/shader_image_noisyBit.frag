#extension GL_EXT_gpu_shader4: enable

///////////// Do not touch the code below /////////
uniform sampler2D texture_0; // Texture index 
uniform float screenOffsetX;	// Transfrom coordinate system into Eyeris
uniform float screenOffsetY;	// Transfrom coordinate system into Eyeris
uniform float cX;	// Center of object
uniform float cY;	// Center of object
//////////////////////////////////////////////////


// noisy bit parameters
uniform float rndseed1;		// can also be a timestamp
uniform float rndseed2;		// integer in range [0,65535]

// grating parameters
uniform float transparency;
uniform float spatialFreq;
uniform float phase;
uniform float angle;
uniform float amplitude;
uniform float pixelAngle;

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

///////////////////////// noisy bit method ///////////////////////////////
float noisybit(float scale, float rn) {
	float scale255 = scale * 255;
	float scaleint = floor(scale255);
	float scaledec = scale255 - scaleint;
	
	float scaleuse;
	
	// for example, if desired value is 129.3, show 129 in 30% of frames and 130 in 70% of frames
	if (rn < scaledec) {
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
	float x = gl_FragCoord.x - (screenOffsetX + cX);
	float y = gl_FragCoord.y - (screenOffsetY + cY);
	
	x = x * pixelAngle / 60;
	y = y * pixelAngle / 60;
	
	// Get color from the image
	// this method assumes the same value in each color channel
	vec4 color = texture2D(texture_0, vec2(gl_TexCoord[0]));
	
	// Apply bit stealing
	float scaleuse = noisybit(color.r, Rand( uint(gl_FragCoord.x), uint(gl_FragCoord.y), uint(rndseed1), uint(rndseed2) ) );

	// Modify color of object
	gl_FragColor.w = color.w;
	gl_FragColor.r = scaleuse;
	gl_FragColor.g = scaleuse;
	gl_FragColor.b = scaleuse;
}
