#ifndef __OBJECT_H
#define __OBJECT_H 1

#include <group.h>
#include <vector>

/*
 * object.h : Defines a high-level container object for Wavefront models
 *            And object is 1) named, and 2) contains a collection of 
 *            render groups (defined by a unique material and shading model)
 */

class object {

 public:

  object() {return;}

  ~object(){
    groups.clear();
    return;
  }

  object( const object &o ) {
    (*this) = o;
    return;
  }

  // Overload of the assignment operator
  inline object & operator = (const object &o) {
    (*this).name = o.name;
    copy ( o.groups.begin(), o.groups.end(), back_inserter( (*this).groups ) );
    return (*this);
  }

  void flush( void ) {
    groups.clear();
    this->name = "";
  }

  void setName( std::string name ) {
    this->name = name;
    return;
  }

  void setAlpha( float alpha ) {
    for ( uint i=0; i<this->groups.size(); i++ )
      this->groups[i].setAlpha( alpha );
    return;
  }

  std::string getName(void) {
    return this->name;
  }

  unsigned int getNumberOfGroups(void) {
    return this->groups.size();
  }

  std::vector <group> getGroupVec(void) {
    return this->groups;
  }

  // Given a material and a shading model....
  // find the group in this object which matches
  group getGroup( material m, unsigned int s ) {


    std::string id = m.getName() + "_" + std::to_string(s);

    for ( uint i=0; i<groups.size(); i++ ) {
      if ( groups[i].getID() == id )
	return groups[i];
    }

    // In the (unlikely) event that (m,s) don't define a group.... create it
    std::cout << "No such group... creating it\n";
    this->addGroup(m,s);
    return this->groups[ this->groups.size()-1 ];
  }

  // Get a pointer to a particular group with ID 
  group * getGroup( std::string id ) {

    for ( uint i=0; i<groups.size(); i++ ) {
      if ( groups[i].getID() == id )
	return &groups[i];
    }

    // If the group doesn't exist... bitch & whine and return null
    std::cout << "No such group!\n";
    return 0x0;
  }

  // Add a new group to the list and return it for use
  group addGroup( material m, unsigned int s ) {
    
    std::string id = m.getName() + "_" + std::to_string(s);
      
    // Make sure this group doesn't already exist
    for ( uint i=0; i<groups.size(); i++ ) {
      if ( groups[i].getID() == id )
	return groups[i];
    }

    groups.push_back(group(m,s));
    return groups[ groups.size()-1 ];

  }

  group * addGroup( group * g ) {
    for ( uint i=0; i<groups.size(); i++ ) {
      if ( g->getID() == groups[i].getID() )
	return 0x0;
    }
    groups.push_back(*g);
    return &groups[ groups.size()-1 ];
  }

  void addGroup( group g ) {
    for ( uint i=0; i<groups.size(); i++ ) {
      if ( g.getID() == groups[i].getID() )
	return;
    }
    groups.push_back(g);
    return;
  }

  bool hasGroup( std::string materialName, unsigned int shading ) {
    std::string id = materialName + "_" + std::to_string(shading);
    for ( uint i=0; i<groups.size(); i++ ) {
      if ( groups[i].getID() == id )
	return true;
    }
    return false;
  }

  bool hasGroup( std::string id ) {
    for ( uint i=0; i<groups.size(); i++ ) {
      if ( groups[i].getID() == id )
	return true;
    }
    return false;
  }

  // If we accidentally added a group that ended up with no faces, remove it
  void purgeGroups(void) {

    for(std::vector<group>::iterator it=groups.begin(); it != groups.end(); it++ ) {
      if ( it->getNumberOfFaces() == 0 ) {
	this->groups.erase(it);
      }
    }
    return;
  }

  void setupMaterial(void) {

    for(std::vector<group>::iterator it=groups.begin(); it != groups.end(); it++ )
	it->setupMaterial();

    return;
  }

  GLuint makeList(void) {
    GLuint theList = glGenLists (1);
    glNewList( theList, GL_COMPILE );
    draw();
    glEndList();
    return theList;
  }

  void draw(void) {

    for(std::vector<group>::iterator it=groups.begin(); it != groups.end(); it++ )
      it->draw();
    
    return;
  }

  friend std::ostream & operator << (std::ostream &, object &);

 protected:
  std::string name;
  std::vector <group> groups;
};
#endif
