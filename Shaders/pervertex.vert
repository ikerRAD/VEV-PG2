#version 120

uniform mat4 modelToCameraMatrix; //Modelview
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient;  // rgb

uniform struct light_t {
	vec4 position;    // Camera space
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
} theLights[4];     // MG_MAX_LIGHTS

uniform struct material_t {
	vec3  diffuse;
	vec3  specular;
	float alpha;
	float shininess;
} theMaterial;

attribute vec3 v_position; // Model space
attribute vec3 v_normal;   // Model space
attribute vec2 v_texCoord;

//Variables que hay que rellenar salida vertex a fragment
varying vec4 f_color;
varying vec2 f_texCoord;


float lambert_f(vec3 n, vec3 l){
	return max(dot(n,l),0.0);
}

float specular_f(float lambert,vec3 n, vec3 l, vec3 v){// TODO, mejorar aprovechando el lambert d antes
	float res = 0.0;
	float rf;
	vec3 r;
	if(lambert > 0.0){
		rf = 2.0 * lambert;
		r = r * n;
		r = r - l;
		
		res = max(0.0,dot(r, v));	
	}

	return res;
}

void aporte_dir(in int i,in vec3 l,in vec3 n,in vec3 v,inout vec3 acum1, inout vec3 acum2){
	
	float factor1 = lambert_f(n,l);//aporte de lambert
	if (factor1 > 0.0){
		acum1 = acum1 + factor1 * theMaterial.diffuse * theLights[i].diffuse;
		
		float factor2 = specular_f(factor1,n,l,v); // specular_f
		if (factor2 > 0.0){
			factor2 = factor1 * pow(factor2, shininess);
			acum2 = acum2 + factor2 * theMaterial.specular * theLights[i].specular;
		}
		
	}
	
}


void main() {
	vec3 L, N, V_P, V;
	vec3 acum_dif, acum_esp;
	
	acum_dif = vec3(0.0);
	acum_esp = vec3(0.0);
	
	vec4 N4_eye = modelToCameraMatrix * vec4(v_normal,0.0);
	N = normalize(N4_eye.xyz); // En el s.c. camara
	
	vec4 V4_eye = modelToCameraMatrix * vec4(v_position,1.0);
	V_P = V4_eye.xyz; // En el s.c. camara
	
	V = normalize(-1.0 * V_P); //vertice-camara
	
	for(int i = 0; i < active_lights_n; i++){
	
		// Cálculo del color
		if (theLights[i].position.w == 0.0){//Es un vector, luz direccional
		
			L = normalize(-1.0 * theLights[i].position.xyz);
				
		}else{
		
			L = normalize(theLights[i].position.xyz - V_P); //igual para spot y positional
		
		}
		
		aporte_dir(i, L, N, V, acum_dif, acum_esp);
	
	}
	
	f_color = vec4(acum_dif + scene_ambient + acum_esp, 1.0);
	
	//coordenadas de textura que se pasan del vertex-shadder al fragment-shadder
	f_texCoord = v_texCoord;
	
	gl_Position = modelToClipMatrix * vec4(v_position, 1.0); // obligatoria en todo vertex shadder. Ultima línea
}
