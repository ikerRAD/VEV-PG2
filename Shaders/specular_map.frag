#version 120

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient; // Scene ambient light

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

uniform sampler2D texture0;
uniform sampler2D specmap;    // specular map

varying vec3 f_position;      // camera space
varying vec3 f_viewDirection; // camera space
varying vec3 f_normal;        // camera space
varying vec2 f_texCoord;


float lambert_f(vec3 n, vec3 l){
	return max(dot(n,l),0.0);
}

float specular_f(vec3 n, vec3 l, vec3 v){// TODO, mejorar aprovechando el lambert d antes
	float res = 0.0;
	vec3 r;

    r = normalize(2.0*dot(n, l)*n - l);

    float RoV = dot(r, v);

    if (RoV > 0.0)
            res = pow(RoV, theMaterial.shininess);
    
	return res;
}

void aporte_dir(in int i,in vec3 l,in vec3 n,in vec3 v,inout vec3 acum1, inout vec3 acum2, in vec4 texel){
	
	float factor1 = lambert_f(n,l);//aporte de lambert
	if (factor1 > 0.0){
		acum1 = acum1 + factor1 * theMaterial.diffuse * theLights[i].diffuse;
		
		float factor2 = specular_f(n,l,v); // specular_f
		if (factor2 > 0.0){
			acum2 = acum2 + * factor1 * factor2 * texel.rgb * theLights[i].specular;
		}
		
	}
	
}

float f_dist(int i, float d){

	float ret = 1.0;

	if(d <= 0){
		if(theLights[i].attenuation.x >= 0){
			ret = 1.0 / theLights[i].attenuation.x;
		}
	}else{

		float a_dividir = theLights[i].attenuation.x + theLights[i].attenuation.y * d + theLights[i].attenuation.z * pow(d, 2);

		if(a_dividir > 0.0){
			ret = 1.0 / a_dividir;
		}
	}
	
	return min(ret, 1.0);

}

void aporte_pos(in int i,in vec3 l,in vec3 n,in vec3 v,inout vec3 acum1, inout vec3 acum2, in vec4 texel){
	
	float dist = length(l);
	vec3 l2 = normalize(l);
	float factor1 = lambert_f(n,l2);//aporte de lambert
	float atenuacion = f_dist(i, dist);

	if (factor1 > 0.0){
		acum1 = acum1 + atenuacion * factor1 * theMaterial.diffuse * theLights[i].diffuse;
		
		float factor2 = specular_f(n,l2,v); // specular_f
		if (factor2 > 0.0){
			acum2 = acum2 + atenuacion * factor1 * factor2 * texel.rgb * theLights[i].specular;
		}
		
	}
	
}

float foco_f(vec3 l, int i){
	return max(dot(-1.0 * l, theLights[i].spotDir), 0);
}

void aporte_foco(in int i,in vec3 l,in vec3 n,in vec3 v,inout vec3 acum1, inout vec3 acum2, in vec4 texel){
	
	float cosOs = foco_f(l, i);
	float spt = 0.0;
	if (cosOs > 0.0){//Evitamos cálculos
		if (cosOs >= theLights[i].cosCutOff){//si Os es menor o igual que Ou
			float factor1 = lambert_f(n,l);//aporte de lambert

			if (factor1 > 0.0){
				spt = pow(cosOs, theLights[i].exponent);
				acum1 = acum1 + spt * factor1 * theMaterial.diffuse * theLights[i].diffuse;
				
				float factor2 = specular_f(n,l,v); // specular_f
				if (factor2 > 0.0){
					acum2 = acum2 + spt * factor1 * factor2 * texel.rgb * theLights[i].specular;
				}
				
			}
		}
	}
	
}


void main() {
	
	vec4 f_color;
	vec4 texel = texture2D(specmap, f_texCoord);
	////////////////

	vec3 L, N, V_P, V;
	vec3 acum_dif, acum_esp;
	
	acum_dif = vec3(0.0);
	acum_esp = vec3(0.0);
	
	N = normalize(f_normal); // En el s.c. camara
	
	V_P = f_position; // En el s.c. camara
	
	V = normalize(f_viewDirection); //vertice-camara
	
	for(int i = 0; i < active_lights_n; i++){
	
		// Cálculo del color
		if (theLights[i].position.w == 0.0){//Es un vector, luz direccional
		
			L = normalize(-1.0 * theLights[i].position.xyz);
			aporte_dir(i, L, N, V, acum_dif, acum_esp, texel);
				
		}else{
		
			L = theLights[i].position.xyz - V_P; //igual para spot y positional


			if (theLights[i].cosCutOff == 0){//posicional

				aporte_pos(i, L, N, V, acum_dif, acum_esp, texel);

			}else{//foco

				L = normalize(L);
				aporte_foco(i, L, N, V, acum_dif, acum_esp, texel);

			} 
		
		}
		
		
	
	}
	
	f_color = vec4(acum_dif + scene_ambient + acum_esp, 1.0);

	//////////////

	vec4 texColor;
	texColor = texture2D(texture0, f_texCoord);
	gl_FragColor = f_color * texColor;

}
