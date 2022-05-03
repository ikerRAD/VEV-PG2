#version 120

// Bump mapping with many lights.

// all attributes in model space
attribute vec3 v_position;
attribute vec3 v_normal;
attribute vec2 v_texCoord;
attribute vec3 v_TBN_t;
attribute vec3 v_TBN_b;

uniform mat4 modelToCameraMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)

uniform struct light_t {
	vec4 position;    // Camera space
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
} theLights[4];     // MG_MAX_LIGHTS

// All bump computations are performed in tangent space; therefore, we need to
// convert all light (and spot) directions and view directions to tangent space
// and pass them the fragment shader.

varying vec2 f_texCoord;
varying vec3 f_viewDirection;     // tangent space
varying vec3 f_lightDirection[4]; // tangent space
varying vec3 f_spotDirection[4];  // tangent space

void main() {

    vec3 n,t,bn;
    n = (modelToCameraMatrix * vec4(v_normal,0.0)).xyz;
    n = normalize(n);

    t = (modelToCameraMatrix * vec4(v_TBN_t,0.0)).xyz;
    t = normalize(t);

    bn = (modelToCameraMatrix * vec4(v_TBN_b,0.0)).xyz;
    bn = normalize(bn);

    vec3 v_pos = (modelToCameraMatrix * vec4(v_position,1.0)).xyz;

    vec3 tmp;

    tmp.x = dot(v_pos,t);
    tmp.y = dot(v_pos,bn);
    tmp.z = dot(v_pos,n);

    f_viewDirection = tmp;
    vec3 tmp2, tmp3;
    for(int i = 0; i < active_lights_n; i++){
        if(theLights[i].position.w == 0.0){
            tmp2 = -1.0 * theLights[i].position.xyz;
        }else{
            tmp2 = theLights[i].position.xyz - v_pos;

            if (theLights[i].cosCutOff != 0){ //foco, calcilamos sporDir
				tmp3 = theLights[i].spotDir.xyz;

                tmp.x = dot(tmp3,t);
                tmp.y = dot(tmp3,bn);
                tmp.z = dot(tmp3,n);

                f_spotDirection[i] = tmp;
			}
        }
    
        tmp.x = dot(tmp2,t);
        tmp.y = dot(tmp2,bn);
        tmp.z = dot(tmp2,n);

        f_lightDirection[i] = tmp;
    }

    f_texCoord = v_texCoord;
	gl_Position = modelToClipMatrix * vec4(v_position, 1.0);
}
















