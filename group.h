#ifndef __GROUP_H
#define __GROUP_H 1

#include <face.h>
#include <vector>

#include <gl.h>

/*
 *  group.h: class definition for a render group. A render group is a 
 *           collection of faces (defined in face.h) which share a 
 *           common material and shading model
 */

class group {

 public:

  group(void) {

    shading       = 0;
    consistant    = false;
    size          = 0;
    first         = true;

    this->ID = "default_0";
    return;
  }

  group( material m, unsigned int s ) {
    shading       = 0;
    consistant    = false;
    size          = 0;
    first         = true;
    this->mat = m;
    this->shading = s;
    this->ID = this->mat.getName() + "_" + std::to_string(s);

    return;
  }

  ~group(){
    faces.clear();
    vertices.clear();
    normals.clear();
    textures.clear();
    return;
  }

  group( const group &g ) {
    (*this) = g;
    return;
  }

  // Overload of the assignment operator
  inline group & operator = (const group &g) {
    (*this).faces         = g.faces;
    (*this).mat           = g.mat;
    (*this).shading       = g.shading;
    (*this).consistant    = g.consistant;
    (*this).ID            = g.ID;
    (*this).size          = g.size;
    (*this).first         = g.first;

    (*this).vertices      = g.vertices;
    (*this).normals       = g.normals;
    (*this).textures      = g.textures;

    return (*this);
  }

  void addFace( face f ) {
    this->faces.push_back(f);
    this->addVertexToVector(f);
    return;
  }

  void addFace( face *f ) {
    this->faces.push_back((*f));
    this->addVertexToVector(*f);
    return;
  }

  void flush( void ) {
    faces.clear();
    vertices.clear();
    normals.clear();
    textures.clear();
    mat.flush();
    shading = 0;
  }

  uint getNumberOfFaces( void ) {
    return this->faces.size();
  }

  void setMaterial( material m ) {
    this->mat = m;
    return;
  }

  material getMaterial( void ) {
    return this->mat;
  }

  void setAlpha( float alpha ) {
    this->mat.setD( alpha );
  }

  void setShading( unsigned int s=0 ) {
    this->shading = s;
    return;
  }

  unsigned int getShading( void ) {
    return this->shading;
  }

  std::string getID(void) {
    return this->ID;
  }

  std::vector <face> getFaceVector(void) {
    return this->faces;
  }

  bool checkConsistancy(void) {

    this->consistant = false;

    for ( uint i=1; i<this->faces.size(); i++ ) {
      if ( this->faces[i].getType() != this->faces[i-1].getType() ) {
	std::cout << "Group " << this->ID << " is inconsistant\n";
	return false;
      }
    }
    this->consistant = true;
    //std::cout << "Group " << this->ID << " is consistant\n";
    return true;
  }

  void addVertexToVector( face f ) {
    std::vector<vertex> vtx = f.getVertices();

    for ( uint j=0; j<vtx.size(); j++ ) {

      vec v = vtx[j].getVtx();
      vec n = vtx[j].getNorm();
      vec t;
      bool hasTex = vtx[j].hasTextureCoordinates();

      if ( hasTex )
	t = vtx[j].getTex();

      this->vertices.push_back(v.x);
      this->normals.push_back(n.x);
      if ( hasTex )
	this->textures.push_back(t.x);

      this->vertices.push_back(v.y);
      this->normals.push_back(n.y);
      if ( hasTex )
	this->textures.push_back(t.y);

      this->vertices.push_back(v.z);
      this->normals.push_back(n.z);
      if ( hasTex )
	this->textures.push_back(t.z);

    }

    return;
  }

  void drawPoints(void) {

    glBegin(GL_POINTS);
    for ( uint i=0; i<size; i+=3 ) {
      glVertex3f( vertices[i], vertices[i+1], vertices[i+2] );
    }
    glEnd();

    return;
  }

  void drawArrays(void) {
    float *v = this->vertices.data();
    float *n = this->normals.data();
    float *t = 0x0;

    if ( this->textures.size() > 0 )
      t = this->textures.data();

    int size = this->vertices.size()/3;
    
    glEnableClientState(GL_VERTEX_ARRAY);			// Enable vertex arrays
    glEnableClientState(GL_NORMAL_ARRAY);			// Enable normal arrays

    if ( this->textures.size() > 0 )
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);		// Enable texture arrays

    glVertexPointer(3, GL_FLOAT, 0, v);		               	// Vertex Pointer to vertex array
    glNormalPointer(GL_FLOAT, 0, n);			        // Normal pointer to normal array
    if ( this->textures.size() > 0 ) {
      glTexCoordPointer(3, GL_FLOAT, 0, t);		        // Texture pointer to normal array
    }

    if ( this->faces[0].getType() == TRIANGLE )
      glDrawArrays(GL_TRIANGLES, 0, size);                      // Draw the triangles

    else if ( this->faces[0].getType() == QUAD )
      glDrawArrays(GL_QUADS, 0, size);		                // or the quads

    glDisableClientState(GL_VERTEX_ARRAY);			// Disable vertex arrays
    glDisableClientState(GL_NORMAL_ARRAY);			// Disable normal arrays
    if ( this->textures.size() > 0 )
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);		// Disable texture arrays

    return;
  }

  void drawFaces(void) {

    for(std::vector<face>::iterator it=faces.begin(); it != faces.end(); it++ )
      it->draw();

    return;
  }

  void setupMaterial(void) {
    float *ka = this->mat.getKd();  // Ambient  color -- usually {0,0,0}, so use diffuse color instead
    float *kd = this->mat.getKd();  // Diffuse  color
    float *ks = this->mat.getKs();  // Specular color

    if ( this->shading ) {
      glEnable( GL_POLYGON_SMOOTH ) ;
      glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST ) ;
      glShadeModel(GL_SMOOTH);
    } else
      glDisable( GL_POLYGON_SMOOTH );

    if ( ka )
      glMaterialfv( GL_FRONT, GL_AMBIENT, ka );
    if ( kd )
      glMaterialfv( GL_FRONT, GL_DIFFUSE, kd );
    if ( ks )
      glMaterialfv( GL_FRONT, GL_SPECULAR, ks );
    
    GLfloat shininess[] = {this->mat.getNs()};
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
    glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, new float[4] {0.0f,0.0f,0.0f,0.0f});

    if ( this->mat.getTextureID() ) {
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glBindTexture( GL_TEXTURE_2D, this->mat.getTextureID() );
    }
  }

  void draw(void) {

    if ( first ) {
      checkConsistancy();
      first = false;
    }

    setupMaterial();
    
    if ( this->consistant )
      drawArrays();
    else
      drawFaces();

    if ( this->mat.getTextureID() )
      glDisable(GL_TEXTURE_2D);

    return;
  }

  friend std::ostream & operator << (std::ostream &, group &);

 protected:
  std::vector <face> faces;
  std::string ID;
  material     mat;
  unsigned int shading;
  bool consistant;
  uint size;

  bool first;

  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> textures;

};
#endif
