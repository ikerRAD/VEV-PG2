#version 120

attribute vec3 v_position;
attribute vec3 v_normal;
attribute vec2 v_texCoord;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;


varying vec3 f_position;       // camera space
varying vec3 f_viewDirection;  // camera space
varying vec3 f_normal;         // camera space
varying vec2 f_texCoord;      

varying vec3 f_positionw; // world space
varying vec3 f_normalw;   // world space

void main() {

	f_positionw = (modelToWorldMatrix * vec4(v_position, 1.0)).xyz;
	f_normalw = (modelToWorldMatrix * vec4(v_normal, 0.0)).xyz;

	f_position = (modelToCameraMatrix * vec4(v_position, 1.0)).xyz;
	f_normal = (modelToCameraMatrix * vec4(v_normal, 0.0)).xyz;
	f_viewDirection = -1.0 * f_position;

	f_texCoord = v_texCoord;

	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}
