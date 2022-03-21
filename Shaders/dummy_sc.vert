#version 120

uniform mat4 modelToCameraMatrix; // M
uniform mat4 cameraToClipMatrix;  // P

attribute vec3 v_position;

uniform float sc; /// Mi variable tiempo

varying vec4 f_color;

void main() {

	float factor;
	if(sc < 0.5)
		factor = 2.0;
	else
		factor = -2.0;
		
	float s = sin(sc);
	float c = cos(sc);
	float m = 1 -(s+c)/2

	f_color = vec4(s, c,m,1); ///RGBTransparencia
	//P * M (proyection * modelview)
	gl_Position = cameraToClipMatrix * modelToCameraMatrix * vec4(v_position,1);
	vec4(v_position + vec3(0.0, factor, 0.0), 1);
}
