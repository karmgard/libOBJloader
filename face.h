#ifndef __FACE_H
#define __FACE_H 1

#define LINE     2
#define TRIANGLE 3
#define QUAD     4

#include <vertex.h>
#include <material.h>

#include <gl.h>
#include <vector>
#include <cmath>

/*
 *  face.h : Class file for building openGL "faces". A face is 
 *           either a triangle or a quadrilaterial (this class
 *           tries to be agnostic about which it is). Mainly 
 *           holds a set of 3 or 4 vertices
 */

class face {

 public:
  face(uint type = 3) {
    if ( type != LINE && type != TRIANGLE && type != QUAD )
      type = TRIANGLE;

    this->type = type;
    this->vertices.reserve(type);

    return;
  }

  face(material m, uint t=3, uint s=0) {
    if ( t != LINE && t != TRIANGLE && t != QUAD )
      t = TRIANGLE;

    this->type = t;
    this->vertices.reserve(type);

    return;
  }

  ~face(void) {
    this->vertices.clear();
    return;
  }

  // Overload the assignment operator
  inline face operator = (const face &f) {
    copy( f.vertices.begin(), f.vertices.end(), back_inserter( (*this).vertices ) );
    (*this).type     = f.type;
    return (*this);
  }

  void addVertex( vertex  v ) {
    vertices.push_back(v);

    if ( vertices.size() == type && !v.hasNormals() ) {
      vec n = this->calculateNormal();
    
      for ( uint i=0; i<this->vertices.size(); i++ )
	this->vertices[i].setNormal( n );
    }
    return;
  }

  void addVertex( vertex *v ) {
    this->vertices.push_back((*v));

    if ( vertices.size() == type && !v->hasNormals() ) {
      vec n = this->calculateNormal();

      for ( uint i=0; i<this->vertices.size(); i++ )
	this->vertices[i].setNormal( n );

    }
  
    return;
  }

  int getType(void) {
    return this->type;
  }

  int getNumVertices(void) {
    return this->vertices.size();
  }

  std::vector<vertex> getVertices(void) {
    return this->vertices;
  }

  void draw(void) {

    if ( this->type == TRIANGLE ) {
      glBegin(GL_TRIANGLES);
    } else if ( this->type == QUAD ) {
      glBegin(GL_QUADS);
    }

    for ( uint i=0; i<vertices.size(); i++ ) {
      vec v = vertices[i].getVtx();
      glVertex3f( v.x, v.y, v.z );

      if ( vertices[i].hasNormals() ) {
	v = vertices[i].getNorm();
	glNormal3f( v.x, v.y, v.z );
      }

      if ( vertices[i].hasTextureCoordinates() ) {
	v = vertices[i].getTex();
	glTexCoord3f( v.x, v.y, v.z );
      }

    }

    glEnd();

  }

  friend std::ostream & operator << (std::ostream&, face&);

 protected:
  std::vector<vertex> vertices;
  unsigned int type;

  vec calculateNormal(void) {

    vec norm;

    // Calculate vector1 and vector2 -- two edges of the triangle/quad
    vec va, vb, vr;
    float val;

    va = this->vertices[0].getVtx() - this->vertices[1].getVtx();
    vb = this->vertices[0].getVtx() - this->vertices[2].getVtx();

    // Cross product of the two edges
    vr = va * vb;

    // Magnitude for normalization
    val = sqrt( vr.x*vr.x + vr.y*vr.y + vr.z*vr.z );
 
    norm.x = vr.x/val;
    norm.y = vr.y/val;
    norm.z = vr.z/val;

    return norm;
  }

};

#endif
