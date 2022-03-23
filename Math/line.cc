#include <cassert>
#include <cstdio>
#include <cstdlib>
#include "line.h"
#include "constants.h"
#include "tools.h"

Line::Line() : m_O(Vector3::ZERO), m_d(Vector3::UNIT_Y) {}
Line::Line(const Vector3 & o, const Vector3 & d) : m_O(o), m_d(d) {}
Line::Line(const Line & line) : m_O(line.m_O), m_d(line.m_d) {}

Line & Line::operator=(const Line & line) {
	if (&line != this) {
		m_O = line.m_O;
		m_d = line.m_d;
	}
	return *this;
}

// Set line to pass through two points A and B
//
// Note: Check than A and B are not too close!

void Line::setFromAtoB(const Vector3 & A, const Vector3 & B) {
	//Creamos el vector que va desde A a B
	Vector3 aux = B - A;
	//Si el vector es demasiado pequeño, saltamos un error
	if(aux.length() < Constants::distance_epsilon){
		printf("Ambos puntos están demasiado cerca!\n");
		m_O = Vector3::ZERO;
		m_d = Vector3::UNIT_Y;
	}else{ // Si no, creamos la recta
		m_O = A;
		m_d = aux.normalize();
	}
}

// Give the point corresponding to parameter u

Vector3 Line::at(float u) const {
	Vector3 res;
	//Calculamos el punto correspondiente en la recta
	res = m_O + u*m_d;
	return res;
}

// Calculate the parameter 'u0' of the line point nearest to P
//
// u0 = m_d*(P-m_o) / m_d*m_d , where * == dot product

float Line::paramDistance(const Vector3 & P) const {
	float res = 0.0f;
	float m_d2 = m_d.dot(m_d); //|m_d|^2
	if(m_d2 < Constants::distance_epsilon){//Si |m_d|^2 es un valor enano, devolvemos error
		res = -1;
	}else{ // Sino, calculamos la distancia al punto proyectado
		res = m_d.dot(P-m_O)/m_d2;
	}
	return res;
}

// Calculate the minimum distance 'dist' from line to P
//
// dist = ||P - (m_o + u0*m_d)||
// Where u0 = paramDistance(P)

float Line::distance(const Vector3 & P) const {
	float res = 0.0f;
	//Distancia a u0
	float u0 = paramDistance(P);
	if(u0 < 0){
		res = 0.0; //CORRECIÓN
		printf("Error, división entre 0\n");
	}else{//Calculamos la distancia utilizando u0
		Vector3 aux = P - (m_O + u0*m_d);
		res = aux.length();
	}
	return res;
}

void Line::print() const {
	printf("O:");
	m_O.print();
	printf(" d:");
	m_d.print();
	printf("\n");
}
