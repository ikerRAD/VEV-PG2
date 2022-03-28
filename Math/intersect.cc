#include <cmath>
#include "intersect.h"
#include "constants.h"
#include "tools.h"

/* | algo           | difficulty | */
/* |----------------+------------| */
/* | BSPherePlane   |          1 | */
/* | BBoxBBox       |          2 | */
/* | BBoxPlane      |          4 | */

// test if a BSpheres intersects a plane.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect

int BSpherePlaneIntersect(const BSphere *bs, Plane *pl) {
	//Obtenemos los parámetros de la esfera
    Vector3 sp_centro = bs->m_centre;
    float sp_radio = bs->m_radius;
    //Obtenemos los parámetros del plano
    float pl_md = pl->m_d;
    Vector3 pl_normal = pl->m_n;
    //Calculamos la distancia del plano al centro de la esfera
	float nP = pl_normal.dot(sp_centro);
	float dist = nP - pl_md;
    // vemos si el valor de esta distancia es mayor o igual que el radio
	if (fabs(dist) <= sp_radio){
		return IINTERSECT;
	}else if (dist < 0){//No intersecta y la esfera está detrás
		return -IREJECT;
	}else{//No intersecta y la esfera está delante
		return +IREJECT;
	}

}


// test if two BBoxes intersect.
//! Returns :
//    IINTERSECT intersect
//    IREJECT don't intersect

int  BBoxBBoxIntersect(const BBox *bba, const BBox *bbb ) {
    //Obtenemos los parámetros de ambas bounding box
	Vector3 min_a = bba->m_min;
	Vector3 min_b = bbb->m_min;

	Vector3 max_a = bba->m_max;
	Vector3 max_b = bbb->m_max;
    //Aplicamos las distintas reglas para detectar si las boundig boxes chocan
    //Si, colisionan para todas las dimensiones, colisionarán de verdad
	if (!(min_a[0] > max_b[0] || min_b[0] > max_a[0])){			//CORRECCIÓN
		if (!(min_a[1] > max_b[1] || min_b[1] > max_a[1])){		//CORRECCIÓN
			if (!(min_a[2] > max_b[2] || min_b[2] > max_a[2])){	//CORRECCIÓN
				return IINTERSECT;
			}
		}
	}
    //Si ya no hemos hecho return es que no colisionan
	return IREJECT;

}

//test if a BBox and a plane intersect.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect

int  BBoxPlaneIntersect (const BBox *theBBox, Plane *thePlane) {
    //Obtenemos normal del plano y los parámetros de la bounding box
	Vector3 normal = thePlane->m_n;
	Vector3 pmax = theBBox->m_max;
	Vector3 pmin = theBBox->m_min;

	//puntos mas cercano y lejano del plano (a futuro)
	Vector3 p1,p2;
	p1 = Vector3::ZERO;
	p2 = Vector3::ZERO;

	//Depende del signo de la normal, obtendremos valores del máximo o el mínimo de la bounding box
	//Nos da igual saber que punto de los que obtengamos es el cercano y que punto el lejano, solo
	//nos importa saber que uno será el lejano y otro el cercano. Esto se consigue, fijándose en el signo de la normal

	//signo dimensión x
	if(normal[0] > 0){
	    p1[0] = pmin[0];
	    p2[0] = pmax[0];
	}else{
        p1[0] = pmax[0];
        p2[0] = pmin[0];
	}
    //signo dimensión y
    if(normal[1] > 0){
        p1[1] = pmin[1];
        p2[1] = pmax[1];
    }else{
        p1[1] = pmax[1];
        p2[1] = pmin[1];
    }
    //signo dimension z
    if(normal[2] > 0){
        p1[2] = pmin[2];
        p2[2] = pmax[2];
    }else{
        p1[2] = pmax[2];
        p2[2] = pmin[2];
    }

    //vemos a si pertenecen al mismo lado
    int pertenece = thePlane->whichSide(p1) + thePlane->whichSide(p2);
    if (pertenece == 2){//Ambos pertenecen al lado positivo
        return +IREJECT;
    }else if (pertenece == -2){//Ambos pertenecen al lado negativo
        return -IREJECT;
    }else{//O bien uno de los puntos toca el plano  (o los dos) o están en distinto lugar del plano
        // Pertenece == -1, 0, 1
        return IINTERSECT;
    }

}

// Test if two BSpheres intersect.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBSphereIntersect(const BSphere *bsa, const BSphere *bsb ) {

	Vector3 v;
	v = bsa->m_centre - bsb->m_centre;
	float ls = v.dot(v);
	float rs = bsa->m_radius + bsb->m_radius;
	if (ls > (rs * rs)) return IREJECT; // Disjoint
	return IINTERSECT; // Intersect
}


// Test if a BSpheres intersect a BBox.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBBoxIntersect(const BSphere *sphere, const BBox *box) {

	float d;
	float aux;
	float r;

	r = sphere->m_radius;
	d = 0;

	aux = sphere->m_centre[0] - box->m_min[0];
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[0] - box->m_max[0];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}

	aux = (sphere->m_centre[1] - box->m_min[1]);
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[1] - box->m_max[1];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}

	aux = sphere->m_centre[2] - box->m_min[2];
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[2] - box->m_max[2];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}
	if (d > r * r) return IREJECT;
	return IINTERSECT;
}

// Test if a Triangle intersects a ray.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int IntersectTriangleRay(const Vector3 & P0,
						 const Vector3 & P1,
						 const Vector3 & P2,
						 const Line *l,
						 Vector3 & uvw) {
	Vector3 e1(P1 - P0);
	Vector3 e2(P2 - P0);
	Vector3 p(crossVectors(l->m_d, e2));
	float a = e1.dot(p);
	if (fabs(a) < Constants::distance_epsilon) return IREJECT;
	float f = 1.0f / a;
	// s = l->o - P0
	Vector3 s(l->m_O - P0);
	float lu = f * s.dot(p);
	if (lu < 0.0 || lu > 1.0) return IREJECT;
	Vector3 q(crossVectors(s, e1));
	float lv = f * q.dot(l->m_d);
	if (lv < 0.0 || lv > 1.0) return IREJECT;
	uvw[0] = lu;
	uvw[1] = lv;
	uvw[2] = f * e2.dot(q);
	return IINTERSECT;
}

/* IREJECT 1 */
/* IINTERSECT 0 */

const char *intersect_string(int intersect) {

	static const char *iint = "IINTERSECT";
	static const char *prej = "IREJECT";
	static const char *mrej = "-IREJECT";
	static const char *error = "IERROR";

	const char *result = error;

	switch (intersect) {
	case IINTERSECT:
		result = iint;
		break;
	case +IREJECT:
		result = prej;
		break;
	case -IREJECT:
		result = mrej;
		break;
	}
	return result;
}
