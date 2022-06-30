#version 460

struct Camera {
	vec3 loc;
	vec3 lookAt;
	vec3 up;
	vec3 right;
	float fov;
};

struct DirLight {
	vec3 dir;
	vec3 amb;
	vec3 diff;
	vec3 spec;
};

#define FLOAT_PREC 0.0000005
#define PI_2 1.570796327
#define SQRT_2 0.7071067812

uniform vec2 resolution;
uniform float time;
uniform float elapsedTime;
uniform float zoom;
uniform float zoomRaw;

uniform Camera camera;

out vec4 FragColor;

const vec3 sky = vec3(0.4);
const vec3 black = vec3(0.0);
const vec3 bowl = vec3(0.5, 0.0, 0.0);

const vec3 up = vec3(0.0, 0.0, 1.0);
const DirLight light1 = DirLight(vec3(-SQRT_2, 0.0, SQRT_2), vec3(0.2), vec3(0.75), vec3(1.0));

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

vec3 getLightColor(in vec3 norm) {
	vec3 ambient = 0.2 * bowl;
	float diff = max(dot(-light1.dir, norm), 0.0);
	vec3 diffuse = diff * bowl;
	vec3 viewDir = normalize(camera.lookAt - camera.loc);
	vec3 halfwayDir = normalize(light1.dir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
	vec3 specular = vec3(1.0) * spec;
	return ambient + diffuse + specular;
}

vec3 getColor(in float it) {
	vec3 col = 0.5 + 0.5 * cos(3.0 + it * 0.15 + vec3(0.0, 0.6, 1.0));
	return it == 0 ? black : col;
}

float mandelbrot(in vec2 c) {
	vec2 z = vec2(0.0);
	float i;
	for (i = 0; i < 512.0;) {
		z = vec2(z.x * z.x - z.y * z.y, z.x * z.y * 2.0) + c;
		float mag2 = dot(z, z);
		if (mag2 > 256.0 * 256.0)
			break;
		i += 1.0;
	}
	
	if (i > 511.0)
		return 0.0;
		
	return i - log2(log2(dot(z,z)));
}

vec3 getNormal(in vec3 pos) {
	const float eps = 0.00001;
	const vec2 k = vec2(1, -1);
	vec2 n = normalize( k.xy * distanceToMandelbrot(vec2((pos + k.xyy * eps).x, length(pos + k.xyy * eps))) +
						k.yy * distanceToMandelbrot(vec2((pos + k.yyx * eps).x, length(pos + k.yyx * eps))) +
						k.yx * distanceToMandelbrot(vec2((pos + k.yxy * eps).x, length(pos + k.yxy * eps))) +
						k.xx * distanceToMandelbrot(vec2((pos + k.xxx * eps).x, length(pos + k.xxx * eps))));
	float cos_theta = dot(vec3(0.0, 1.0, 0.0), normalize(vec3(0.0, pos.y, pos.z)));
	float sin_theta = -1.0 * sqrt((1 - cos_theta * cos_theta)); //Underneath the xy-plane
	mat3 rot = mat3(1.0, 0.0, 0.0, 0.0, cos_theta, sin_theta, 0.0, -sin_theta, cos_theta);
	return rot * vec3(pos.x, n);
}

float map(in vec3 pos) {
	if (equalf(pos.z, 0.0)) {
		return distanceToMandelbrot(pos.xy);
	} else {
		//calculate arc up to xy-plane
		return distanceToMandelbrot(vec2(pos.x, length(pos.xy)));
	}
}

void raycast(in vec3 ro, in vec3 rd, in vec3 rdx, in vec3 rdy, out float txy, out float tb) {
	vec2 intersections = eliIntersect(ro, rd, vec3(2.0, 1.25, 1.25));
	
	txy = -ro.z / rd.z;
	if (intersections.x < 0.0 && intersections.y < 0.0) {
		tb = -1.0;
	} else {
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
			
			t += (PI_2 - dot(rd, up)) * h; //Scale by angle with the xy-plane
			
			if (t > intersections.y) {
				t = -1.0;
				break;
			}
		}
		tb = t;
	}
}

vec3 render(vec2 p, mat4 view) {
	vec2 pixel_loc = (2.0 * p - resolution) / (resolution.y * zoom); //view coordinate of pixel
	vec2 pixel_loc_x = (2.0 * (p + vec2(1.0, 0.0)) - resolution) / (resolution.y * zoom);
	vec2 pixel_loc_y = (2.0 * (p + vec2(0.0, 1.0)) - resolution) / (resolution.y * zoom);
	
	vec3 ray_origin = camera.loc;
	vec3 ray_dir = normalize((view * vec4(pixel_loc, camera.fov, 0.0)).xyz);
	vec3 rdx = normalize((view * vec4(pixel_loc_x, camera.fov, 0.0)).xyz);
	vec3 rdy = normalize((view * vec4(pixel_loc_y, camera.fov, 0.0)).xyz);
	
	float plane_intersect, bowl_intersect;
	raycast(ray_origin, ray_dir, rdx, rdy, plane_intersect, bowl_intersect);
	
	//No intersection
	if (plane_intersect < 0.0 && bowl_intersect < 0.0)
		return sky;
	
	vec3 col = sky;
	
	vec3 pli = ray_origin + plane_intersect * ray_dir; //plane intersection point
	float plane_dist = distanceToMandelbrot(pli.xy);
	vec3 plane_col = getColor(mandelbrot(pli.xy));
	
	vec3 bowl_col = black;
	if (bowl_intersect >= 0.0) {
		vec3 bi = ray_origin + bowl_intersect * ray_dir; //bowl intersection point
		vec3 norm = getNormal(bi);
		bowl_col = getLightColor(norm);
	}
	
	if (equalf(plane_dist, 0.0) && plane_intersect <= bowl_intersect && plane_intersect >= 0.0) {
		col = plane_col;
	} else if (bowl_intersect >= 0.0) {
		col = bowl_col;
	} else if (plane_intersect >= 0.0) {
		col = mix(col, plane_col, 0.5);
	}
	
	return col;
}

void main() {	
	vec3 up = vec3(0.0, 0.0, 1.0);
	vec3 cd = normalize(camera.lookAt - camera.loc);
	vec3 cx = normalize(camera.right);
	vec3 cy = normalize(camera.up);
	mat4 view = mat4(cx, 0.0, cy, 0.0, cd, 0.0, 0.0, 0.0, 0.0, 1.0);
	
	vec3 col1 = render(gl_FragCoord.xy, view);
	vec3 col2 = render(gl_FragCoord.xy + vec2(0.5, 0.0), view);
	vec3 col3 = render(gl_FragCoord.xy + vec2(0.0, 0.5), view);
	vec3 col4 = render(gl_FragCoord.xy + vec2(0.5, 0.5), view);
	
	FragColor = vec4((col1 + col2 + col3 + col4) / 4.0, 1.0);
}