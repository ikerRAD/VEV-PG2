#include <cstdio>
#include <cassert>
#include "node.h"
#include "nodeManager.h"
#include "intersect.h"
#include "bboxGL.h"
#include "renderState.h"

using std::string;
using std::list;
using std::map;

// Recipe 1: iterate through children:
//
//    for(auto it = m_children.begin(), end = m_children.end();
//        it != end; ++it) {
//        auto theChild = *it;
//        theChild->print(); // or any other thing
//    }

Node::Node(const string &name) :
	m_name(name),
	m_parent(0),
	m_gObject(0),
	m_light(0),
	m_shader(0),
	m_placement(new Trfm3D),
	m_placementWC(new Trfm3D),
	m_containerWC(new BBox),
	m_checkCollision(true),
	m_isCulled(false),
	m_drawBBox(false) {}

Node::~Node() {
	delete m_placement;
	delete m_placementWC;
	delete m_containerWC;
}

static string name_clone(const string & base) {

	static char MG_SC_BUFF[2048];
	Node *aux;

	int i;
	for(i = 1; i < 1000; i++) {
		sprintf(MG_SC_BUFF, "%s#%d", base.c_str(), i);
		string newname(MG_SC_BUFF);
		aux = NodeManager::instance()->find(newname);
		if(!aux) return newname;
	}
	fprintf(stderr, "[E] Node: too many clones of %s\n.", base.c_str());
	exit(1);
	return string();
}

Node* Node::cloneParent(Node *theParent) {

	string newName = name_clone(m_name);
	Node *newNode = NodeManager::instance()->create(newName);
	newNode->m_gObject = m_gObject;
	newNode->m_light = m_light;
	newNode->m_shader = m_shader;
	newNode->m_placement->clone(m_placement);
	newNode->m_parent = theParent;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		newNode->m_children.push_back(theChild->cloneParent(this));
	}
	return newNode;
}


Node *Node::clone() {
	return cloneParent(0);
}

///////////////////////////////////
// transformations

void Node::attachGobject(GObject *gobj ) {
	if (!gobj) {
		fprintf(stderr, "[E] attachGobject: no gObject for node %s\n", m_name.c_str());
		exit(1);
	}
	if (m_children.size()) {
		fprintf(stderr, "EW] Node::attachGobject: can not attach a gObject to node (%s), which already has children.\n", m_name.c_str());
		exit(1);
	}
	m_gObject = gobj;
	propagateBBRoot();
}

GObject *Node::detachGobject() {
	GObject *res = m_gObject;
	m_gObject = 0;
	return res;
}

GObject *Node::getGobject() {
	return m_gObject;
}

void Node::attachLight(Light *theLight) {
	if (!theLight) {
		fprintf(stderr, "[E] attachLight: no light for node %s\n", m_name.c_str());
		exit(1);
	}
	m_light = theLight;
}

Light * Node::detachLight() {
	Light *res = m_light;
	m_light = 0;
	return res;
}

void Node::attachShader(ShaderProgram *theShader) {
	if (!theShader) {
		fprintf(stderr, "[E] attachShader: empty shader for node %s\n", m_name.c_str());
		exit(1);
	}
	m_shader = theShader;
}

ShaderProgram *Node::detachShader() {
	ShaderProgram *theShader = m_shader;
	m_shader = 0;
	return theShader;
}

ShaderProgram *Node::getShader() {
	return m_shader;
}

void Node::setDrawBBox(bool b) { m_drawBBox = b; }
bool Node::getDrawBBox() const { return m_drawBBox; }

bool Node::isCulled() const { return m_isCulled; }

void Node::setCheckCollision(bool b) {
	m_checkCollision = b;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		theChild->setCheckCollision(b);
	}
}

bool Node::getCheckCollision() const { return m_checkCollision; }

///////////////////////////////////
// transformations

void Node::initTrfm() {
	Trfm3D I;
	m_placement->swap(I);
	// Update Geometric state
	updateGS();
}

void Node::setTrfm(const Trfm3D * M) {
	if (!M) {
		fprintf(stderr, "[E] setTrfm: no trfm for node %s\n", m_name.c_str());
		exit(1);
	}
	m_placement->clone(M);
	// Update Geometric state
	updateGS();
}

void Node::addTrfm(const Trfm3D * M) {
	if (!M) {
		fprintf(stderr, "[E] addTrfm: no trfm for node %s\n", m_name.c_str());
		exit(1);
	}
	m_placement->add(M);
	// Update Geometric state
	updateGS();
}

void Node::translate(const Vector3 & P) {
	static Trfm3D localT;
	localT.setTrans(P);
	addTrfm(&localT);
};

void Node::rotateX(float angle ) {
	static Trfm3D localT;
	localT.setRotX(angle);
	addTrfm(&localT);
};

void Node::rotateY(float angle ) {
	static Trfm3D localT;
	localT.setRotY(angle);
	addTrfm(&localT);
};

void Node::rotateZ(float angle ) {
	static Trfm3D localT;
	localT.setRotZ(angle);
	addTrfm(&localT);
};

void Node::scale(float factor ) {
	static Trfm3D localT;
	localT.setScale(factor);
	addTrfm(&localT);
};

///////////////////////////////////
// tree operations

Node *Node::parent() {
	if (!m_parent) return this;
	return m_parent;
}

/**
 * nextSibling: Get next sibling of node. Return first sibling if last child.
 *
 */

Node *Node::nextSibling() {
	Node *p = m_parent;
	if(!p) return this;
	list<Node *>::iterator end = p->m_children.end();
	list<Node *>::iterator it = p->m_children.begin();
	while(it != end && *it != this) ++it;
	assert(it != end);
	++it;
	if (it == end) return *(p->m_children.begin());
	return *it;
}

/**
 * firstChild: Get first child of node. Return itself if leaf node.
 *
 * @param theNode A pointer to the node
 */

Node *Node::firstChild() {
	if (!m_children.size()) return this;
	return *(m_children.begin());
}

// Cycle through children of a Node. Return children[ i % lenght(children) ]

Node * Node::cycleChild(size_t idx) {

	size_t m = idx % m_children.size();
	size_t i = 0;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		if (i == m) return *it;
		i++;
	}
	return 0;
}

//
//
// Add a child to node
// Print a warning (and do nothing) if node already has an gObject.

void Node::addChild(Node *theChild) {

	if (theChild == 0) return;
	if (m_gObject) {
		// node has a gObject, so print warning
		printf("Impossible to insert a child to a terminal node");
	} else {
		// node does not have gObject, so attach child
		theChild->m_parent = this;
		m_children.push_back(theChild);
		updateGS();

	}
}

void Node::detach() {

	Node *theParent;
	theParent = m_parent;
	if (theParent == 0) return; // already detached (or root node)
	m_parent = 0;
	theParent->m_children.remove(this);
	// Update bounding box of parent
	theParent->propagateBBRoot();
}

//
//
// given a node:
// - update the BBox of the node (updateBB)
// - Propagate BBox to parent until root is reached
//
// - Preconditions:
//    - the BBox of thisNode's children are up-to-date.
//    - placementWC of node and parents are up-to-date

void Node::propagateBBRoot() {
	updateBB();
	//Si tiene padre, se extiende hacia arriba
	if(m_parent){
		m_parent->propagateBBRoot();
	}
}

// auxiliary function
//
// given a node, update its BBox to World Coordinates so that it includes:
//  - the BBox of the geometricObject it contains (if any)
//  - the BBox-es of the children
//
// Note: Bounding box is always in world coordinates
//
// Precontitions:
//
//     * m_placementWC is up-to-date
//     * m_containerWC of children are up-to-date
//
// These functions can be useful
//
// Member functions of BBox class (from Math/bbox.h):
//
//    void init(): Set BBox to be the void (empty) BBox
//    void clone(const BBox * source): Copy from source bounding box
//    void transform(const Trfm3D * T): Transform BBox by applying transformation T
//    void include(BBox *B): Change BBox so that it also includes BBox B
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

void Node::updateBB () {
	
	if(m_gObject){//Nodo hoja
		m_containerWC->clone(m_gObject->getContainer());//Por si el BB está transformado
		m_containerWC->transform(m_placementWC);//Transformamos
	}else{//Nodo no hoja
		m_containerWC->init();//Reinicia su BB
		for(auto it = m_children.begin(), end = m_children.end();
			it != end; ++it) {
			Node *theChild = *it;
			//Cada nodo hoja se le une haciendo una 'super'-BB
			m_containerWC->include(theChild->m_containerWC);
		}
	}
}

// Update WC (world coordinates matrix) of a node and
// its bounding box recursively updating all children.
//
// given a node:
//  - update the world transformation (m_placementWC) using the parent's WC transformation
//  - recursive call to update WC of the children
//  - update Bounding Box to world coordinates
//
// Precondition:
//
//  - m_placementWC of m_parent is up-to-date (or m_parent == 0)
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

void Node::updateWC() {
	if(m_parent == 0){//El padre no tiene transformación global
		m_placementWC->clone(m_placement);
	}else{//Componemos la transformación global con la del padre más la local del hijo
		m_placementWC->clone(m_parent->m_placementWC);
		m_placementWC->add(m_placement);
	}

	if(m_gObject == 0){
		//llama hijos
		for(auto it = m_children.begin(), end = m_children.end();
			it != end; ++it) {
			Node *theChild = *it;
			theChild->updateWC();
		}
	}

	updateBB();

}

//
//
//  Update geometric state of a node.
//
// given a node:
// - Update WC transformation of sub-tree starting at node (updateWC)
// - Propagate Bounding Box to root (propagateBBRoot), starting from the parent, if parent exists.

void Node::updateGS() {
	updateWC();
	if(m_parent){
		m_parent->propagateBBRoot(); //Correccion
	}
}


// Draw a (sub)tree.
//
// These functions can be useful:
//
// RenderState *rs = RenderState::instance();
//
// rs->addTrfm(RenderState::modelview, T); // Add T transformation to modelview
// rs->push(RenderState::modelview); // push current matrix into modelview stack
// rs->pop(RenderState::modelview); // pop matrix from modelview stack to current
//
// gobj->draw(); // draw geometry object (gobj)
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

void Node::draw() {

	ShaderProgram *prev_shader = 0;
	RenderState *rs = RenderState::instance();

	if (m_isCulled) return;

	// Set shader (save previous)
	if (m_shader != 0) {
		prev_shader = rs->getShader();
		rs->setShader(m_shader);
	}

	// Print BBoxes
	if(rs->getBBoxDraw() || m_drawBBox)
		BBoxGL::draw( m_containerWC );


	if(m_gObject){//hoja
		//push
		rs->push(RenderState::modelview);
		//cargar transf global
		rs->addTrfm(RenderState::modelview, m_placementWC);

		// CORRECCIÓN DE MAMEN
		// FIJAMOS LA TRANSFORMACION EN LA PILA DEL MODELO: 
		// (PARA CONSEGUIR DESPUES LA MATRIZ QUE VA DEL ESPACIO DEL MODELO AL ESPACIO DEL MUNDO)
		// See: TrfmStack m_modelStack; // model space to world space
		rs->loadTrfm(RenderState::model, m_placementWC);

		//dibujo
		m_gObject->draw();
		//Pop
		rs->pop(RenderState::modelview);

	}else{//dibuja hijos
	//UpdateWC o GS, ns
		for(auto it = m_children.begin(), end = m_children.end();
				it != end; ++it) {
				Node *theChild = *it;
				theChild->draw();
		}
	}


	if (prev_shader != 0) {
		// restore shader
		rs->setShader(prev_shader);
	}
}

// Set culled state of a node's children

void Node::setCulled(bool culled) {
	m_isCulled = culled;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		theChild->setCulled(culled); // Recursive call
	}
}

// @@ TODO: Frustum culling. See if a subtree is culled by the camera, and
//          update m_isCulled accordingly.

void Node::frustumCull(Camera *cam) {
	/* =================== PUT YOUR CODE HERE ====================== */
	int col  = cam->checkFrustum(m_containerWC, 0);

	if (col == -IREJECT){ // Todo está dentro
		setCulled(0);
	}else if (col == IREJECT){ // Todo está fuera
		setCulled(1);
	}else{ //intersecta
		if (m_gObject){ //objeto intersecta, se dibuja
			m_isCulled = 0;
		}else{//nodo intermedio intersecta, depende de los hijos
			m_isCulled = 0;
			for(auto it = m_children.begin(), end = m_children.end();
				it != end; ++it) {
				Node *theChild = *it;
				
				theChild->frustumCull(cam);

			}

		}
	}


	/* =================== END YOUR CODE HERE ====================== */
}

// @Check whether a BSphere (in world coordinates) intersects with a
// (sub)tree.
//
// Return a pointer to the Node which collides with the BSphere. 0
// if not collision.
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

const Node *Node::checkCollision(const BSphere *bsph) const {
	if (!m_checkCollision) return 0;
	//Si intersecta
	if(BSphereBBoxIntersect(bsph, m_containerWC) == IINTERSECT){
		if(m_gObject)//En caso de ser nodo hoja, devuelve su puntero
			return this;
		else{
            //afina buscando en sus hijos, si no termina chocando con ninguna hoja, no hay colisión real
			for(auto it = m_children.begin(), end = m_children.end();
				it != end; ++it) {
				const Node *theChild = *it;
				auto col = theChild->checkCollision(bsph);

				if(col != 0){
					return col;
				}
			}

			return 0;
		}
	}else{
		return 0;
	}
}

void Node::print_trfm(int sep) const {
	std::string delim(sep,' ');
	printf("%sNode:%s\n", delim.c_str(), m_name.c_str());
	m_placementWC->print(delim);
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		const Node *theChild = *it;
		theChild->print_trfm(sep + 1);
	}
}
