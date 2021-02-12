#ifndef __VERTEX_H
#define __VERTEX_H 1

#include <iostream>

/*
 *  struct vec: simple structure with operators that encapsulates 
 *              a vector. No, not std::vector(), an actual 
 *              vector. One that any competant physicist would
 *              recognize.
 */

struct vec {
  float x;
  float y;
  float z;

  // Overload of the assignment operator
  inline vec & operator = (const vec &rhs) {
    //std::cout << "\t\t\tEquating vectors\n";
    (*this).x = rhs.x;
    (*this).y = rhs.y;
    (*this).z = rhs.z;
    return *this;
  }

  // Overload of the self-subtraction operator
  inline vec & operator -= (const vec &rhs) {
    (*this).x -= rhs.x;
    (*this).y -= rhs.y;
    (*this).z -= rhs.z;
    return *this;
  }

  // Overload of the self-addition operator
  inline vec & operator += (const vec &rhs) {
    (*this).x += rhs.x;
    (*this).y += rhs.y;
    (*this).z += rhs.z;
    return *this;
  }

  // Addition operation
  inline vec operator + (const vec & rhs) const {
    vec v;
    v.x = (*this).x + rhs.x;
    v.y = (*this).y + rhs.y;
    v.z = (*this).z + rhs.z;
    return v;
  }

  // Subtraction operations
  inline vec operator - (const vec & rhs) const {
    vec v;
    v.x = (*this).x - rhs.x;
    v.y = (*this).y - rhs.y;
    v.z = (*this).z - rhs.z;
    return v;
  }

  // Multiplication -- cross product
    inline vec operator * (const vec & rhs) const {
    vec v;
    v.x = (*this).y * rhs.z - rhs.y * (*this).z;
    v.y = (*this).z * rhs.x - rhs.z * (*this).x;
    v.z = (*this).x * rhs.y - rhs.x * (*this).y;
    return v;
  }

  friend std::ostream & operator << (std::ostream&, vec&);

};

/*
 *  vertex.h : Class that holds the vectors defining an openGL vertex.
 *             Mainly for 3-vectors, but can easily accomodate 2 or 4
 *             vetors as well. In addition it holds the vertex normal
 *             and texture coordinate vectors in one big happy class.
 */

class vertex {
 public:
  vertex(const vertex &v) {
    (*this) = v;
    return;
  }

  vertex(uint s=3) {
    this->size = s;
    this->vtx.x = 0.0f;
    this->vtx.y = 0.0f;
    this->vtx.z = 0.0f;
    this->hasNorms = false;
    this->hasTexCoords = false;
    return;
  }

  vertex(vec v, uint s=3) {
    this->size = s;
    this->vtx = v;
    this->hasNorms = false;
    this->hasTexCoords = false;
    return;
  }

  vertex(vec v, vec n, uint s=3) {
    this->size = size;
    this->vtx = v;
    this->vnx = n;
    this->hasNorms = true;
    this->hasTexCoords = false;
    return;
  }

  vertex(vec v, vec n, vec t, uint s=3) {
    this->size = size;
    this->vtx = v;
    this->vnx = n;
    this->tex = t;
    this->hasNorms = true;
    this->hasTexCoords = true;
    return;
  }

  ~vertex() {return;}

  void setSize( uint s ) {this->size=s;}
  void setVertex( vec v ) {this->vtx=v;}
  void setNormal( vec n ) {this->vnx=n;this->hasNorms=true;}
  void setTextureCoordinates( vec t ) {this->tex=t;this->hasTexCoords=true;}

  uint getSize( void ) {
    return this->size;
  }

  vec getVtx(void) {
    return this->vtx;
  }

  vec getNorm(void) {
    return this->vnx;
  }

  vec getTex(void) {
    return this->tex;
  }

  bool hasNormals(void) {
    return this->hasNorms;
  }

  bool hasTextureCoordinates(void) {
    return this->hasTexCoords;
  }

  // Overload of the assignment operator
  inline vertex & operator = (const vertex &v) {
    (*this).vtx = v.vtx;
    (*this).vnx = v.vnx;
    (*this).tex = v.tex;
    (*this).size = v.size;
    (*this).hasNorms = v.hasNorms;
    (*this).hasTexCoords = v.hasTexCoords;
    return *this;
  }

  friend std::ostream & operator << (std::ostream&, vertex&);

 private:

 protected:
  uint size;

  bool hasNorms;
  bool hasTexCoords;

  vec  vtx; // Vertex coordinates
  vec  vnx; // Vertex normal coordinates
  vec  tex; // Vertex texture coordinates

};

#endif
