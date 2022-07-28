#version 460
layout (location = 0) out int partID;

#define PART_SKY	0
#define PART_SET	1
#define PART_INC	2

#define FLOAT_PREC 	0.0000005

struct Camera {
	vec3 loc;
	vec3 lookAt;
	vec3 up;
	vec3 right;
	float fov;
};

uniform vec2 resolution;
uniform float time;
uniform float elapsedTime;
uniform float zoom;
uniform float zoomRaw;

uniform Camera camera;

//precision equalf
bool equalf(in float a, in float b) {
	float diff = abs(a - b);
	return diff < FLOAT_PREC || diff < abs(a * FLOAT_PREC) || diff < abs(b * FLOAT_PREC);
}

//ellipsoid centered at the origin with radii ra
//taken from https://iquilezles.org/articles/intersectors/
vec2 eliIntersect( in vec3 ro, in vec3 rd, in vec3 ra )
{
    vec3 ocn = ro/ra;
    vec3 rdn = rd/ra;
    float a = dot( rdn, rdn );
    float b = dot( ocn, rdn );
    float c = dot( ocn, ocn );
    float h = b*b - a*(c-1.0);
    if( h<0.0 ) return vec2(-1.0); //no intersection
    h = sqrt(h);
    return vec2(-b-h,-b+h)/a;
}

//change to be my own
//https://iquilezles.org/articles/distancefractals
float distanceToMandelbrot(in vec2 c)
{
	float c2 = dot(c, c);
	// skip computation inside M1 - https://iquilezles.org/articles/mset1bulb
	if( 256.0*c2*c2 - 96.0*c2 + 32.0*c.x - 3.0 < 0.0 ) return 0.0;
	// skip computation inside M2 - https://iquilezles.org/articles/mset2bulb
	if( 16.0*(c2+2.0*c.x+1.0) - 1.0 < 0.0 ) return 0.0;

    // iterate
    float di =  1.0;
    vec2 z  = vec2(0.0);
    float m2 = 0.0;
    vec2 dz = vec2(0.0);
    for( int i=0; i<300; i++ )
    {
        if( m2>1024.0 ) { 
			di=0.0; 
			break; 
		}

		// Z' -> 2·Z·Z' + 1
        dz = 2.0*vec2(z.x*dz.x-z.y*dz.y, z.x*dz.y + z.y*dz.x) + vec2(1.0,0.0);
			
        // Z -> Z² + c			
        z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
			
        m2 = dot(z,z);
    }

    // distance	
	// d(c) = |Z|·log|Z|/|Z'|
	float d = 0.5*sqrt(dot(z,z)/dot(dz,dz))*log(dot(z,z));
    if( di>0.5 ) 
		d=0.0;
	
    return d;
}

void main() {
	vec3 cd = normalize(camera.lookAt - camera.loc);
	vec3 cx = normalize(camera.right);
	vec3 cy = normalize(camera.up);
	mat4 view = transpose(mat4(cx, 0.0, cy, 0.0, cd, 0.0, 0.0, 0.0, 0.0, 1.0));

	vec2 pv = (2.0 * gl_FragCoord.xy - resolution) / (resolution.y * zoom);
	vec3 ro = camera.loc;
	vec3 rd = normalize((view * vec4(pv, camera.fov, 0.0)).xyz);
	
	vec2 intersection = eliIntersect(ro, rd, vec3(2.0, 1.25, 1.25));
	float txy = !equalf(rd.z, 0.0) ? -ro.z / rd.z : -1.0;
	float tb = intersection.x;
	float dist = distanceToMandelbrot((ro + txy * rd).xy);
	
	int part = PART_SKY;
	bool set = equalf(dist, 0.0);
	bool inc = intersection.x >= 0.0 || intersection.y >= 0.0;
	
	//Remove top half of ellipsoid from consideration
	if (inc) {
		//If the first intersection point is below the xy-plane and in front of the camera, then it's ok.
		//If the second intersection point is below the xy-plane and in front of the camera, then it's ok.
		//If neither, then either both intersection points are above the xy-plane, or the camera is facing
		//in the positive z direction.
		
/* 		float xZ = (ro + intersection.x * rd).z;
		float yZ = (ro + intersection.y * rd).z;
		bool xZL = xZ <= FLOAT_PREC;
		bool yZL = yZ <= FLOAT_PREC;
		bool ixG = intersection.x >= 0.0;
		bool iyG = intersection.y >= 0.0;
		bool xS = (xZL && ixG);
		bool yS = (yZL && iyG);
		bool S = xS || yS;
		bool negZ = S; */
		
		bool negZ = ((ro + intersection.x * rd).z <= FLOAT_PREC && intersection.x >= 0.0) || ((ro + intersection.y * rd).z <= FLOAT_PREC && intersection.y >= 0.0);
		inc = negZ;
	}
	
	part = inc ? PART_INC : part;
	if (txy >= 0.0 && set)
		part = PART_SET;
		
	partID = part;
}