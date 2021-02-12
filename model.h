#ifndef __MODEL_H
#define __MODEL_H 1

#include <string>
#include <vector>

#include <object.h>

#define POINTS    1
#define LINES     2
#define TRIANGLES 3
#define QUADS     4

struct initial_conditions {
  float x;
  float y;
  float z;

  float phi;
  float theta;

  float alpha;

  float scale;
};

class model {
public: 
  model(std::string, std::string="");
  ~model();

  void draw(void);
  void setAlpha( float );
  void makeList(void);
  void set_initial_conditions( initial_conditions i ) {
    ic = i;
  }
  void set_initial_conditions( float x, float y, float z, float p=0.0, float t=0.0, float a=1.0, float s=1.0 ) {
    ic.x = x;
    ic.y = y;
    ic.z = z;

    ic.phi = p;
    ic.theta = t;

    ic.alpha = a;
    ic.scale = s;
    return;
  }
  initial_conditions get_initial_conditions(void) {
    return ic;
  }

  GLuint getList(void) {
    return listNum;
  }

  std::string getObjFile(void) {
    return this->objFile;
  }

  std::string getName(void) {

    //cout << "\n\n" << objFile.find("/") << ", " << objFile.length() << "\n";

    //if ( objFile.find("/") != objFile.length() )
    //return objFile.substr(objFile.find_last_of("/"),objFile.length()-4);
    //else
      return objFile.substr(0,objFile.length()-4);
  }

  std::string getMtlFile(void) {
    if ( this->materials.size() )
      return this->mtlFile;
    else
      return "none";
  }

  std::vector <object> getObjectVector(void) {
    return this->objects;
  }

  object getObject(uint which) {
    if ( which < this->objects.size() )
      return this->objects[which];
    return object();
  }

  friend std::ostream & operator << (std::ostream &, model &);

 protected:
  std::vector <material> materials;
  std::vector <object>   objects;

  std::string objFile;
  std::string mtlFile;

  initial_conditions ic;
  GLuint       listNum;

  unsigned int NumberOfVertices;
  unsigned int NumberOfTextures;
  unsigned int NumberOfNormals;
  unsigned int NumberOfObjects;

  bool      parseModel         ( void );
  bool      loadModel          ( void );
  bool      loadMaterials      ( void );
  material  getMaterialByName  ( std::string );

 private:
  static const bool debug = false;

};

#endif
