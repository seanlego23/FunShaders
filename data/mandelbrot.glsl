#version 460

uniform vec2 resolution;
uniform vec2 translation;
uniform vec2 scale;
uniform vec2 offset;
uniform float time;
uniform float elapsedTime;
uniform float zoom;

out vec4 FragColor;

vec4 getColor(float it) {
	vec3 col = 0.5 + 0.5 * cos(3.0 + it * 0.15 + vec3(0.0, 0.6, 1.0));
	return vec4(col, 1.0);
}

float mandelbrot(vec2 c) {
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

void main() {
	vec2 loc = gl_FragCoord.xy / resolution * scale;
	loc = loc / zoom + offset + translation;
	
	float halfX = 0.5 / (resolution.x * zoom);
	float halfY = 0.5 / (resolution.y * zoom);
	
	vec4 colors[4];
	colors[0] = getColor(mandelbrot(loc));
	colors[1] = getColor(mandelbrot(loc + vec2(0.0, halfY)));
	colors[2] = getColor(mandelbrot(loc + vec2(halfX, 0.0)));
	colors[3] = getColor(mandelbrot(loc + vec2(halfX, halfY)));
	
	FragColor = (colors[0] + colors[1] + colors[2] + colors[3]) / 4.0;
}