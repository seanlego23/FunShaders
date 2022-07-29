#version 460
layout (location = 0) out vec3 bNormal;
layout (location = 1) out int bMask;

#define PART_SKY	0
#define PART_SET	1
#define PART_INC	2

#define FLOAT_PREC 	0.0000005
#define PI			3.141592654
#define PI_2 		1.570796327
#define SQRT_2 		0.7071067812

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

layout(binding = 0) uniform isampler2D part_tex;

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
float distanceToMandelbrot(in vec2 c) {
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
	float d = 0.5*sqrt(dot(z,z)/dot(dz,dz))*log(sqrt(dot(z,z)));
    if( di>0.5 ) 
		d=0.0;
	
    return d;
}

vec2 distanceToMandelbrot2(in vec4 c) {
	float c12 = dot(c.xy, c.xy);
	float c22 = dot(c.zw, c.zw);
	
	vec2 d = vec2(1.0);
	
	// skip computation inside M1 - https://iquilezles.org/articles/mset1bulb
	if (256.0*c12*c12 - 96.0*c12 + 32.0*c.x - 3.0 < 0.0) 
		d.x = 0.0;
	
	// skip computation inside M2 - https://iquilezles.org/articles/mset2bulb
	if (16.0*(c12+2.0*c.x+1.0) - 1.0 < 0.0) 
		d.x = 0.0;
	
	// skip computation inside M1 - https://iquilezles.org/articles/mset1bulb
	if (256.0*c22*c22 - 96.0*c22 + 32.0*c.z - 3.0 < 0.0) 
		d.y = 0.0;
	
	// skip computation inside M2 - https://iquilezles.org/articles/mset2bulb
	if (16.0*(c22+2.0*c.z+1.0) - 1.0 < 0.0) 
		d.y = 0.0;
		
	if (d.x == 0.0 && d.y == 0.0)
		return d;

    // iterate
    vec2 di = vec2(1.0);
    vec4 z  = vec4(0.0);
    vec2 m2 = vec2(0.0);
    vec4 dz = vec4(0.0);
    for (int i=0; i<300; i++) {
        if (m2.x>1024.0) { 
			di.x = 0.0;
		}
		
		if (m2.y > 1024.0) {
			di.y = 0.0;
		}
		
		if (di.x == 0.0 && di.y == 0.0) {
			break;
		}

		// Z' -> 2·Z·Z' + 1
        dz = 2.0*vec4(z.xz*dz.xz-z.yw*dz.yw, z.xz*dz.yw + z.yz*dz.xw) + vec4(1.0,0.0,1.0,0.0);			
        // Z -> Z² + c			
        z = vec4( z.xz*z.xz - z.yw*z.yw, 2.0*z.xz*z.yw ) + c;
			
        m2 = vec2(dot(z.xy,z.xy), dot(z.zw,z.zw));
    }

    // distance	
	// d(c) = |Z|·log|Z|/|Z'|
	vec2 dotZ = vec2(dot(z.xy,z.xy),dot(z.zw,z.zw));
	vec2 dotDZ = vec2(dot(dz.xy,dz.xy),dot(dz.zw,dz.zw));
	d = 0.5 * sqrt(dotZ / dotDZ) * log(sqrt(dotZ));
    
	if (di.x > 0.5) 
		d.x = 0.0;
		
	if (di.y > 0.5)
		d.y = 0.0;
	
    return d;
}

float map(in vec3 pos) {
	//calculate arc up to xy-plane
	return distanceToMandelbrot(vec2(pos.x, length(pos.yz)));
}

//Find the maximum point on the epsilon circle away from the equipotential curve
//Return angle of that maximum point
float findMaxDiffInDist(in float eps, in float dist, in vec2 pos) {
	float maxDist = dist;
	float thetaLoc = -8.0;
	
	//Search for a point that's close to the maximum point
	for (float theta = 0; theta > 2.0 * (PI - FLOAT_PREC); theta += PI / 36) {
		float newDist = distanceToMandelbrot(pos + eps * vec2(cos(theta), sin(theta)));
		float newDiff = abs(newDist - dist);
		float curDiff = abs(maxDist - dist);
		bool change = newDiff >= curDiff;
		maxDist = change ? newDist : maxDist;
		thetaLoc = change ? theta : thetaLoc;
	}
	
	int dPow = 4;
	float dTheta = PI / 72;
	float lastDiff = abs(maxDist - dist);
	
	//Newton's Method
	for (float theta = thetaLoc + dTheta; dPow < 17; theta += dTheta) {
		float newDist = distanceToMandelbrot(pos + eps * vec2(cos(theta), sin(theta)));
		float newDiff = abs(newDist - dist);
		bool change = newDiff < lastDiff || newDiff == lastDiff;
		dPow = change ? dPow + 1 : dPow;
		dTheta = (change ? -1 : 1) * (PI / (9 * pow(2, dPow)));
		lastDiff = newDiff;
		thetaLoc = theta;
	}
	
	return thetaLoc;
}

//Find the normal of a point on the border of the mandelbrot set
//using the intersection points of a circle of radius epsilon
//and the equipotential curve defined by the Douady-Hubbard potential
//at a specific distance from the mandelbrot set
vec2 findNormal(in float dist, in vec2 pos) {
	float eps = 1.0 / 368.0;
	
	float maxTheta = findMaxDiffInDist(eps, dist, pos);
	vec2 thetas = vec2(maxTheta, maxTheta - 2.0 * PI);
	vec2 dTheta = vec2(-PI / 36, PI / 36);
	
	float maxDist = distanceToMandelbrot(pos + eps * vec2(cos(maxTheta), sin(maxTheta)));
	vec2 last = vec2(maxDist);
	const vec2 dist2 = vec2(dist);
	
	//36 = 9 * (2^2), 16 - 2 = 14
	//theoretical max first time around is 36
	//thus 36 + (1 / 0.5) * 14 = 64
	for (int i = 0; i < 64; i++) {
		thetas += dTheta;
		vec2 cosT = cos(thetas);
		vec2 sinT = sin(thetas);
		
		//Get distance for new positions around epsilon circle
		//vec2 newDist = distanceToMandelbrot2(vec4(pos, pos) + eps * vec4(cosT.x, sinT.x, cosT.y, sinT.y));
		float newDist1 = distanceToMandelbrot(pos + eps * vec2(cosT.x, sinT.x));
		float newDist2 = distanceToMandelbrot(pos + eps * vec2(cosT.y, sinT.y));
		vec2 newDist = vec2(newDist1, newDist2);
		
		//Compare distance to previous distances
		vec2 newDiff = abs(newDist - dist2);
		vec2 lastDiff = abs(last - dist2);
		bvec2 change = greaterThan(newDiff, lastDiff);
		
		//If the new distances are greater than the previous distances,
		//then we have passed the equipotential curve and we must change
		//direction and divide our dTheta by two
		vec2 dirTheta = vec2(change.x ? -0.5 : 1.0, change.y ? -0.5 : 1.0);
		dTheta *= dirTheta;
	}
	
	//thetas now has the angles of the equipotential curve on the epsilon circle
	vec2 cosT = cos(thetas);
	vec2 sinT = sin(thetas);
	vec4 equiPos = vec4(pos + eps * vec2(cosT.x, sinT.x), pos + eps * vec2(cosT.y, sinT.y));
	
	vec2 dir = normalize(equiPos.xy - equiPos.zw);
	vec2 norm = (maxDist < dist ? 1.0 : -1.0) * vec2(dir.y, -dir.x);
	
	return norm;
}

//Cast a ray into the scene to see what it hits
float raycast(in vec3 ro, in vec3 rd, in vec3 rdx, in vec3 rdy) {
	vec2 intersections = eliIntersect(ro, rd, vec3(2.0, 1.25, 1.25));
	float t = ro.z >= 0.0 ? -ro.z / rd.z : intersections.x;
	
	for (int i = 0; i < 128; i++) {
		vec3 pos = ro + t * rd;
		vec3 posx = ro + t * rdx;
		vec3 posy = ro + t * rdy;
		float h = map(ro + t * rd);
		float dx = length(pos - posx);
		float dy = length(pos - posy);
		
		//Less than or equal to half of pixel error
		if (h <= 0.5 * min(dx, dy))
			break;
		
		t += 0.75 * h;
		
		if (t > intersections.y) {
			t = -1.0;
			break;
		}
	}
	return t;
}

void main() {
	bNormal = vec3(0.0);
	bMask = 0;

	int partID = texture(part_tex, gl_FragCoord.xy / resolution).x;
	
	vec3 up = vec3(0.0, 0.0, 1.0);
	vec3 cd = normalize(camera.lookAt - camera.loc);
	vec3 cx = normalize(camera.right);
	vec3 cy = normalize(camera.up);
	mat4 view = mat4(cx, 0.0, cy, 0.0, cd, 0.0, 0.0, 0.0, 0.0, 1.0);
	
	vec2 p = (2.0 * gl_FragCoord.xy - resolution) / (resolution.y * zoom); //view coordinate of pixel
	vec2 px = (2.0 * (gl_FragCoord.xy + vec2(1.0, 0.0)) - resolution) / (resolution.y * zoom);
	vec2 py = (2.0 * (gl_FragCoord.xy + vec2(0.0, 1.0)) - resolution) / (resolution.y * zoom);
	
	vec3 ro = camera.loc;
	vec3 rd = normalize((view * vec4(p, camera.fov, 0.0)).xyz);
	vec3 rdx = normalize((view * vec4(px, camera.fov, 0.0)).xyz);
	vec3 rdy = normalize((view * vec4(py, camera.fov, 0.0)).xyz);
	
	if (partID != PART_INC) {
		bNormal = partID == PART_SKY ? -rd : vec3(0.0, 0.0, 1.0);
		return;
	}
	
	float t = raycast(ro, rd, rdx, rdy);
	
	if (t < 0.0) {
		bNormal = -rd;
		return;
	}
	bMask = 1;
	
	vec3 pos = ro + t * rd;
	vec2 posXY = vec2(pos.x, sign(pos.y) * length(pos.yz));
	float dist = distanceToMandelbrot(posXY);
	vec3 normal = vec3(findNormal(dist, posXY), 0.0);	
	float cosA = dot(vec2(sign(pos.y), 0.0), normalize(pos.yz));
	float sinA = length(cross(vec3(sign(pos.y), 0.0, 0.0), vec3(normalize(pos.yz), 0.0)));
	mat2 rot = mat2(cosA, sinA, -sinA, cosA);
	bNormal = vec3(normal.x, rot * normal.yz);
}