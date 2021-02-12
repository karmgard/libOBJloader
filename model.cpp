#include <model.h>

#include <iostream>
#include <fstream>

using namespace std;

// Material
ostream & operator << (ostream &os, material &mat) {
  os << "name = " << mat.getName() << ", Ns = " << mat.getNs() 
     << ", Ka = (" << mat.getKa()[0] << ", " << mat.getKa()[1] << ", " << mat.getKa()[2] << ", " << mat.getKa()[3] << ")"
     << ", Kd = (" << mat.getKd()[0] << ", " << mat.getKd()[1] << ", " << mat.getKd()[2] << ", " << mat.getKd()[3] << ")"
     << ", Ks = (" << mat.getKs()[0] << ", " << mat.getKs()[1] << ", " << mat.getKs()[2] << ", " << mat.getKs()[3] << ")"
     << ", Ni = " << mat.getNi() 
     << ", d = "  << mat.getD() 
     << ", illum = " << mat.getIllum();

  return os;
}

ostream & operator << (ostream & os, model &m ) {

  os << "Model loaded " << m.objects.size() << " objects from " << m.objFile << " with materials in " << m.mtlFile << "\n";
  for ( uint i=0; i<m.objects.size(); i++ ) {
    object o = m.objects[i];
    os << o << "\n";
  }
  return os;
}

ostream & operator << (ostream &os, object &o) {

  os << "  Object " << o.name << " has " << o.groups.size() << " render groups" << "\n";
  for ( uint i=0; i<o.groups.size(); i++ )
    os << o.groups[i];

  return os;
}

ostream & operator << (ostream &os, group &g) {

  os << "    Group " << g.ID << " has " << g.faces.size() << " faces using material " 
     << g.mat.getName() << " with shading " << g.shading << "\n";
  
  /*for ( uint i=0; i<g.faces.size(); i++ ) {

    if ( g.faces[i].getType() == 3 )
      os << "\tFace " << i << " is a triangle\n";
    else if ( g.faces[i].getType() == 4 )
      os << "\tFace " << i << " is a quadrilateral\n";
    else 
      os << "\tFace " << i << " has " << g.faces[i].getType() << "\n";

    os << g.faces[i];
    }*/
  
  return os;
}

ostream & operator << (ostream &os, face &f) {

  for ( uint i=0; i<f.vertices.size(); i++ ) {
    cout << "\t  vertex " << i;
    if ( f.vertices[i].hasNormals() )
      os << " has a normal ";
    else
      os << " does not have a normal ";

    os << "v[" << i << "] = ";
    os << f.vertices[i];
  }

  return os;
}

ostream & operator << (ostream &os, vertex &v) {
  os << v.vtx;
  return os;
}

ostream & operator << (ostream &os, vec &v) {
  os << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
  return os;
}

model::model(string objFile, string mtlFile) {

  this->NumberOfVertices = this->NumberOfTextures = this->NumberOfNormals = 0;

  this->objFile = objFile;
  if ( mtlFile == "" ) {
    string base = objFile.substr( 0, objFile.length()-4 );
    this->mtlFile = base + ".mtl";
  } else
    this->mtlFile = mtlFile;

  if ( ! this->parseModel() ) {
    cout << "Error parsing " << objFile << "\n";
    return;
  }

  if ( this->debug )
    cout << this->objFile << ", " << this->mtlFile << "\n";

  if ( ! this->loadMaterials() )
    cout << "No materials associated with model\n";

  if ( ! this->loadModel() )
    cout << "Failed to load model from " << this->objFile << "\n";

  ic = {0.0f,0.0f,0.0f,0.0f,0.0f,1.0f};
  listNum = 0;

  //cout << *this << "\n";

  return;
}

model::~model(void) {
  return;
}

void model::draw(void) {

  for(std::vector<object>::iterator it=objects.begin(); it != objects.end(); it++ ) {
    it->draw();
  }
  return;
}

void model::makeList(void) {
  listNum = glGenLists (1);
  glNewList( listNum, GL_COMPILE );
    draw();
  glEndList();
  return;
}

void model::setAlpha( float alpha ) {
  for ( uint i=0; i<this->objects.size(); i++ )
    this->objects[i].setAlpha( alpha );
  
  return;
}

bool model::loadModel( void ) {

  // Open the object file, if the open fails, bail now and yell about it
  ifstream objectFile (this->objFile.c_str());	
  if ( !objectFile.is_open() ) {
    perror(this->objFile.c_str());
    return false;
  }

  string line;
  material currentMaterial;
  object   currentObject;
  group  * currentGroup = 0x0;
  uint shading=0;

  if ( this->debug )
    cout << "\nLoading model from " << this->objFile << "\n";

  // OBJ files start numbering vertices from 1, not from 0 as C arrays
  // So make the arrays 1 bigger than otherwise, and start counting from 1
  // instead of 0 to adjust the vertex we're grabbing to the OBJ numbering
  uint vertices=1, texcoords=1, normals=1;
  vec V[this->NumberOfVertices+1];  // object vertices
  vec N[this->NumberOfNormals+1];   // vertex normals
  vec T[this->NumberOfTextures+1];  // texture coordinates

  // If there's no objects defined in the obj file, make a default
  if ( !this->NumberOfObjects ) {
    // Set a default name
    currentObject.setName( "Object001" );
  }

  // Read in the file line-by-line and push the information to the right place
  while ( !objectFile.eof() ) {

    getline( objectFile, line );
    char c1 = line.c_str()[0], c2 = line.c_str()[1];

    if ( c1 == '#' )                              // Comment
      continue;

    else if ( c1 == 'o' && c2 == 32 ) {           // New object

      // If this is a new object, store the current object (if it exists) ...
      if ( currentObject.getName() != "" ) {
	currentObject.purgeGroups();
	this->objects.push_back(currentObject);
      }

      // Clear out the current object
      currentObject.flush();

      // And store the new name (other attributes will get stored as they're read in from the file)
      currentObject.setName( line.substr(2, line.length()-2) );

    } // End else if (c1 == 'o' && c2 == 32 )
    
    else if ( c1 == 'f' && c2 == 32 ) {           // New face

      uint type = 0;
      // Figure out if we're loading triangles or quads by 
      // counting the number of spaces in this "f" -- face line
      for ( uint i=0; i<line.length(); i++ ) {
	char c = line.c_str()[i];
	if ( c == 32 )
	  type++;
      }

      if ( type != LINES && type != TRIANGLES && type != QUADS ) {
	cout << line << "\n";
	cout << "Unknown face type " << type << " not attempting to process\n";
	continue;
      }

      // Using the type and the number of normals/texture coordinates there are in the file
      // figure out what the format string for sscanf has to look like
      string baseForm, format;
      if ( this->NumberOfNormals == 0 && this->NumberOfTextures == 0 )
	baseForm = "%i";
      else if ( this->NumberOfNormals > 0 && this->NumberOfTextures  == 0 )
	baseForm = "%i//%i";
      else if ( this->NumberOfNormals == 0 && this->NumberOfTextures > 0 )
	baseForm = "%i/%i";
      else
	baseForm = "%i/%i/%i";

      // Make the format string
      for (uint i=0; i<type; i++ )
	format += baseForm + " ";

      face *f = new face(type);

      uint v[type], n[type], t[type];
      line = line.substr(2, line.length()-2);

      if ( this->NumberOfNormals == 0 && this->NumberOfTextures == 0 ) {     // Only object vertices

	// Scan in the 2, 3, or 4 vertices corresponding to LINES, TRIANGLES, or QUADS
	if ( type == LINES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &v[1] );

	else if ( type == TRIANGLES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &v[1], &v[2] );

	else if ( type == QUADS )
	  sscanf( line.c_str(), format.c_str(), &v[0], &v[1], &v[2], &v[3] );

	else
	  cout << "Unknown type " << type << "\n";

	// Add this vertex to the current face
	for ( uint i=0; i<type; i++ )
	  f->addVertex( new vertex(V[v[i]]) );

      } // End if ( this->NumberOfNormals == 0 && this->NumberOfTextures == 0 )

      else if ( this->NumberOfNormals > 0 && this->NumberOfTextures == 0 ) { // Object vertices & normals

	// Scan in the 2, 3, or 4 vertices corresponding to LINES, TRIANGLES, or QUADS
	if ( type == LINES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &n[0], &v[1], &n[1] );

	else if ( type == TRIANGLES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &n[0], &v[1], &n[1], &v[2], &n[2] );

	else if ( type == QUADS )
	  sscanf( line.c_str(), format.c_str(), &v[0], &n[0], &v[1], &n[1], &v[2], &n[2], &v[3], &n[3] );

	else
	  cout << "Unknown type " << type << "\n";

	// Add this vertex to the current face
	for ( uint i=0; i<type; i++ )
	  f->addVertex( new vertex(V[v[i]], N[n[i]], type) );

      } // End else if ( this->NumberOfNormals > 0 && this->NumberOfTextures == 0 )
            
      else if ( this->NumberOfNormals == 0 && this->NumberOfTextures > 0 ) { // Object vertices & texture coordinates

	// Scan in the 2, 3, or 4 vertices corresponding to LINES, TRIANGLES, or QUADS
	if ( type == LINES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &t[0], &v[1], &t[1] );

	else if ( type == TRIANGLES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &t[0], &v[1], &t[1], &v[2], &t[2] );

	else if ( type == QUADS )
	  sscanf( line.c_str(), format.c_str(), &v[0], &t[0], &v[1], &t[1], &v[2], &t[2], &v[3], &t[3] );

	else
	  cout << "Unknown type " << type << "\n";

	// Add this vertex to the current face
	vertex *vtx;
	for ( uint i=0; i<type; i++ ) {
	  vtx = new vertex( V[v[i]], type );
	  vtx->setTextureCoordinates(T[t[i]]);
	  f->addVertex(vtx );
	}
	delete vtx;

      } // End else if ( this->NumberOfNormals == 0 && this->NumberOfTextures > 0 )

      else {                                                                 // Object vertices, normals, & texture coordinates

	// Scan in the 2, 3, or 4 vertices corresponding to LINES, TRIANGLES, or QUADS
	if ( type == LINES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &n[0], &t[0], &v[1], &n[1], &t[1] );

	else if ( type == TRIANGLES )
	  sscanf( line.c_str(), format.c_str(), &v[0], &n[0], &t[0], &v[1], &n[1], &t[1], &v[2], &n[2], &t[2] );

	else if ( type == QUADS )
	  sscanf( line.c_str(), format.c_str(), &v[0], &n[0], &t[0], &v[1], &n[1], &t[1], &v[2], &n[2], &t[2], &v[3], &n[3], &t[3] );

	else
	  cout << "Unknown type " << type << "\n";

	// Add this vertex to the current face
	for ( uint i=0; i<type; i++ )
	  f->addVertex( new vertex(V[v[i]], N[n[i]], T[t[i]], type) );

      } // End else

      // And add this face to the render group in the current object
      if ( !currentGroup ) {
	// If the current render group is undefined... create a new one and get a pointer to it
	currentGroup = currentObject.addGroup( new group() );
      }
      currentGroup->addFace(f);

    } // End else if ( c1 == 'f' && c2 == 32 )

    else if ( c1 == 's' && c2 == 32 ) {           // New shading model

      if ( line.substr(2, 3) == "off" )
	shading = 0;
      else
	shading = atoi( line.substr(2,1).c_str() );

      // Create a new render group if it doesn't already exist
      if ( !currentObject.hasGroup( currentMaterial.getName(), shading ) )
	currentGroup =	currentObject.addGroup( new group(currentMaterial, shading) );
      else
	currentGroup = currentObject.getGroup(currentMaterial.getName() + "_" + to_string(shading));

    } // End else if ( c1 == 's' && c2 == 32 )

    else if ( line.substr(0,6) == "usemtl" ) {    // Switch materials
      string name = line.substr(7, line.length()-7);
      currentMaterial = this->getMaterialByName( name );

      // Create a new render group
      if ( !currentObject.hasGroup( currentMaterial.getName(), shading ) ) 
	currentGroup =	currentObject.addGroup( new group(currentMaterial, shading) );
      else
	currentGroup = currentObject.getGroup(currentMaterial.getName() + "_" + to_string(shading));

    } // End else if ( line.substr(0,6) == "usemtl" )

    else if ( c1 == 'v' && c2 == 32 ) {           // A new vertex

      if ( vertices <= this->NumberOfVertices ) {
	vec v;

	line[0] = line[1] = ' ';
	sscanf( line.c_str(), " %f %f %f", &v.x, &v.y, &v.z );
	V[vertices++] = v;

	if ( vertices > this->NumberOfVertices+1 )
	  cout << "There's a problem with the number of vertices!\n";

      }

    } // End else if ( c1 == 'v' && c2 == 32 )

    else if ( c1 == 'v' && c2 == 't' ) {          // A new texture coordinate

      if ( texcoords <= this->NumberOfTextures ) {

	vec v;

	line[0] = line[1] = ' ';

	sscanf( line.c_str(), " %f %f %f", &v.x, &v.y, &v.z );
	T[texcoords++] = v;

	if ( texcoords > this->NumberOfTextures+1 )
	  cout << "There's a problem with the number of textures!\n";

      }

    } // End else if ( c1 == 'v' && c2 == 't )

    else if ( c1 == 'v' && c2 == 'n' ) {          // A new vertex normal

      if ( normals <= this->NumberOfNormals ) {

	vec v;

	line[0] = line[1] = ' ';

	sscanf( line.c_str(), " %f %f %f", &v.x, &v.y, &v.z );
	N[normals++] = v;

	if ( normals > this->NumberOfNormals+1 )
	  cout << "There's a problem with the number of normals!\n";

      }

    } // End  else if ( c1 == 'v' && c2 == 'n' )

  } // End while ( !objectFile.eof() )

  objectFile.close();

  if ( this->debug )
    cout << "Loaded " << vertices-1 << " vertices, " << normals-1 << " normals, " << texcoords-1 << " texture coordinates\n";

  // Finalize the last object in the model
  if ( currentObject.getName() != "" ) {
    currentObject.purgeGroups();
    this->objects.push_back(currentObject);
  }

  /*
  for ( uint i=0; i<this->objects.size(); i++ ) {
    vector<group> g = this->objects[i].getGroupVec();

    for ( uint j=0; j<g.size(); j++ ) {
      g[j].checkConsistancy();
    }
  } 
  */
  return true;
}

bool model::parseModel( void ) {
  /*
   * Parse the input file, read through and count the number of vertices, normals, uv, faces, materials, etc
   */

  ifstream objectFile (this->objFile.c_str());	
  if ( !objectFile.is_open() ) {
    perror(this->objFile.c_str());
    return false;
  }

  string line;
  unsigned int normals = 0, textures = 0, vertices = 0, faces = 0, objects = 0;

  while ( !objectFile.eof() ) {

    getline( objectFile, line );

    // Grad the first two characters to check the vertex tags
    char c1 = line.c_str()[0], c2 = line.c_str()[1];

    if ( c1 == '#' || c1 == 's' )                // Ignore these tags for now
      continue;                                  // s is the shading model, # is a comment
    else if ( c1 == 'o' )                        // Objects
      objects++;
    else if ( c1 == 'v' && c2 == 'n' )           // Vertex normals
      normals++;
    else if ( c1 == 'v' && c2 == 't' )           // Texture vertices
      textures++;
    else if ( c1 == 'v' && c2 == 32 )            // Model vertices
      vertices++;
    else if ( c1 == 'f' && c2 == 32 ) {          // Face definitions
      faces++;
    } else if ( line.substr(0,6) == "mtllib" ) { // Name of the material libary file to use

      std::string key = "/";
      std::size_t pos = this->objFile.rfind( key );

      if ( pos == std::string::npos )
	this->mtlFile = line.substr(7, line.length());
      else
	this->mtlFile = this->objFile.substr(0, pos+1) + line.substr( 7, line.length() );

    }

  } // End while ( !objectFile.eof() )
  
  if ( debug )
    cout << "Found " << vertices << " vertices, " << normals << " normals, " 
	 << textures << " textures " << faces << " faces and " << objects << " objects\n";

  objectFile.close();

  // If we don't have the minimum we need 
  // build a model, bail right now
  if ( !vertices || !faces )
    return false;

  if ( !objects ) 
    cout << "No objects defined in " << this->objFile << "\n";

  // Save these since we'll need the information in the loader
  this->NumberOfVertices  = vertices;
  this->NumberOfTextures  = textures;
  this->NumberOfNormals   = normals;
  this->NumberOfObjects   = objects;

  return true;
}

bool model::loadMaterials( void ) {

  material mat;
  float    v[3];
  string   line;
  float    Ni = 0.0f, d = 0.0f;
  int      illum = 0;
  char     dummy[8];

  char     *texMap;

  // Open the materials file... If it's not there 
  // complain, set NumberOfMaterials to 0, and return
  ifstream materialFile (this->mtlFile.c_str());	
  if ( !materialFile.is_open() ) {
    perror(this->mtlFile.c_str());
    return false;
  }

  // Read in each material definition in the file
  // See protected section in material.h for definitions 
  // of what the tags mean
  while ( !materialFile.eof() ) {

    getline( materialFile, line );

    if ( line.substr(0,6) == "newmtl" ) {
      
      // If we're changing materials store the current material
      if ( mat.getName() != "" ) {
	// Dump out the material definition
	if ( this->debug )
	  cout << "material #" << this->materials.size() << ": " << mat << "\n";

	// Store the current material
	this->materials.push_back(mat);

	// And flush the current values in preparation for the next one
	mat.flush();
      }

      mat.setName( line.substr(7, line.length()) );

    } else if ( line.substr(0,2) == "Ns" )
      mat.setNs( atof( line.substr(3, line.length()).c_str() ) );

    else if ( line.substr(0,2) == "Ka" ) {
      sscanf( line.c_str(), "%s %f %f %f", dummy, &v[0], &v[1], &v[2] );
      mat.setKa( v );
    }
    else if ( line.substr(0,2) == "Kd" ) {
      sscanf( line.c_str(), "%s %f %f %f", dummy, &v[0], &v[1], &v[2] );
      mat.setKd( v );
    }
    else if ( line.substr(0,2) == "Ks" ) {
      sscanf( line.c_str(), "%s %f %f %f", dummy, &v[0], &v[1], &v[2] );
      mat.setKs( v );
    }
    else if ( line.substr(0,2) == "Ni" ) {
      sscanf( line.c_str(), "%s %f", dummy, &Ni );
      mat.setNi( Ni );
    }
    else if ( line.substr(0,1) == "d" ) {
      sscanf( line.c_str(), "%s %f", dummy, &d );
      mat.setD( d );
    }
    else if ( line.substr(0,5) == "illum" ) {
      sscanf( line.c_str(), "%s %i", dummy, &illum );
      mat.setIllum( illum );
    }
    else if ( line.substr(0,6) == "map_Kd" ) {
      texMap = (char *)malloc( sizeof(char) * line.size() + 1 );
      sscanf( line.c_str(), "%s %s", dummy, texMap );
      mat.setDiffuseTexture( texMap );
    }
    else if ( line.substr(0,6) == "map_Ka" ) {
      texMap = (char *)malloc( sizeof(char) * line.size() + 1 );
      sscanf( line.c_str(), "%s %s", dummy, texMap );
      mat.setAmbientTexture( texMap );
    }
    else if ( line.substr(0,6) == "map_Ks" ) {
      texMap = (char *)malloc( sizeof(char) * line.size() + 1 );
      sscanf( line.c_str(), "%s %s", dummy, texMap );
      mat.setSpecularTexture( texMap );
    }

  } // End while ( !materialFile.eof() )

  // Done reading the materials... close it up
  materialFile.close();

  // Store the final material
  // Dump out the material definition
  if ( this->debug )
    cout << "material #" << this->materials.size() << ": " << mat << "\n";

  // Store the current material, and get set for a new one
  this->materials.push_back(mat);

  if ( this->debug )
    cout << "Loaded " << this->materials.size() << " materials\n";

  return true;
}

material model::getMaterialByName( string name ) {

  material mat;
  for (uint i=0; i<this->materials.size(); i++ ) {
    if ( this->materials[i].getName() == name ) {
      mat = this->materials[i];
      break;
    }
  }

  return mat;
}

