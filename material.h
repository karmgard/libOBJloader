#ifndef __MATERIAL_H
#define __MATERIAL_H 1

#include <string>
#include <iostream>
#include <gl.h>
#include <Magick++.h> 
using namespace Magick; 

/*
 *  material.h : Defines a structure for holding information on materials
 *               mainly means colors (ambient, diffuse, and specular) as
 *               well as transparency and illumination
 */

class material {
 public:

  material(void) {flush();return;}
  ~material(void){return;}

  material ( const material &m ) {
    (*this) = m;
    return;
  }

  void flush( void ) {
    this->name            = "";
    this->Ns              = 0.0f;
    this->Ni              = 0.0f;
    this->d               = 0.0f;
    textureID             = 0;
    this->illum           = 0;    
    this->diffuseTexture  = "";
    this->ambientTexture  = "";
    this->specularTexture = "";

    for ( uint i=0; i<4; i++ ) 
      this->Ka[i] = this->Kd[i] = this->Ks[i] = 0.0f;
    return;
  }

  inline material & operator = (const material &rhs) {
    (*this).name            = rhs.name;
    (*this).Ns              = rhs.Ns;
    (*this).Ni              = rhs.Ni;
    (*this).d               = rhs.d;
    (*this).illum           = rhs.illum;
    (*this).textureID       = rhs.textureID;
    (*this).diffuseTexture  = rhs.diffuseTexture;
    (*this).ambientTexture  = rhs.ambientTexture;
    (*this).specularTexture = rhs.specularTexture;
    for ( uint i=0; i<4; i++ ) {
      (*this).Ka[i] = rhs.Ka[i];
      (*this).Kd[i] = rhs.Kd[i];
      (*this).Ks[i] = rhs.Ks[i];
    }
    return (*this);
  };

  void setName  ( std::string n ) {this->name = n;}
  void setNs    ( float Ns )      {this->Ns = Ns;}
  void setNi    ( float Ni )      {this->Ni = Ni;}
  void setIllum ( int illum )     {this->illum = illum;}

  void setD     ( float d )       {
    this->d = d;
    Ka[3] = Kd[3] = Ks[3] = this->d;
  }

  void setKa( float ka[3] ) {
    for ( int i=0; i<3; i++ )
      this->Ka[i] = ka[i];
  }
  void setKd( float kd[3] ) {
    for ( int i=0; i<3; i++ )
      this->Kd[i] = kd[i];
  }
  void setKs( float ks[3] ) {
    for ( int i=0; i<3; i++ )
      this->Ks[i] = ks[i];
  }

  void setDiffuseTexture( std::string textureFile ) {
    this->textureID = 0;
    this->diffuseTexture = textureFile;
    this->textureID = getImageData(textureFile);
    std::cout << "Image " << textureFile << " bound to diffuse texture " << textureID 
	      << " for material " << this->name << "\n";
    return;
  }
  void setAmbientTexture( std::string textureFile ) {
    this->textureID = 0;
    this->ambientTexture = textureFile;
    textureID = getImageData(textureFile);
    std::cout << "Image " << textureFile << " bound to ambient texture " << textureID 
	      << " for material " << this->name << "\n";
    return;
  }
  void setSpecularTexture( std::string textureFile ) {
    this->textureID = 0;
    this->specularTexture = textureFile;
    textureID = getImageData(textureFile);
    std::cout << "Image " << textureFile << " bound to specular texture " << textureID 
	      << " for material " << this->name << "\n";
    return;
  }

  float  getNs                   (void)    {return this->Ns;}
  float  getNi                   (void)    {return this->Ni;}
  float  getD                    (void)    {return this->d;}
  float  getIllum                (void)    {return this->illum;}
  float *getKa                   (void)    {return this->Ka;}
  float *getKd                   (void)    {return this->Kd;}
  float *getKs                   (void)    {return this->Ks;}
  uint   getTextureID            (void)    {return this->textureID;}
  std::string getName            (void)    {return this->name;}
  std::string getDiffuseTexture  (void)    {return this->diffuseTexture;}
  std::string getAmbientTexture  (void)    {return this->ambientTexture;}
  std::string getSpecularTexture (void)    {return this->specularTexture;}

 protected:
  std::string name;  // From the newmtl line
  float Ns;          // Specular exponent (shininess)
  float Ka[4];       // Ambient  RGB color
  float Kd[4];       // Diffuse  RGB color
  float Ks[4];       // Specular RGB color
  float Ni;
  float d;           // Transparency RGB color
  int   illum;       // Illumination model

  std::string diffuseTexture;  // Diffuse  texture image : map_Kd in mtl file
  std::string ambientTexture;  // Ambient  texture image : map_Ka in mtl file
  std::string specularTexture; // Specular texture image : map_Ks in mtl file

  uint textureID;

  unsigned int getImageData( std::string textureName ) {

    InitializeMagick("");

    // Construct the image object (on the stack). Seperating image
    // construction from the read operation ensures that a failure
    // to read the image file doesn't render the image object useless. 
    Image image;
    Blob blob;

    std::cout << "Loading texture from " << textureName << "\n";

    try { 
      // Read a file into image object 
      image.read( textureName );

      // Set the write format as JPEG
      image.magick( "JPEG" );

      // And write it to a blob
      image.write( &blob, "RGBA" );

      uint w = image.columns();
      uint h = image.rows();
      uint texID;

      // Create a new OpenGL texture...
      glGenTextures(1, &texID);

      // Bind the new texture to a GL_TEXTURE_2D... Future texture functions will modify this texture
      glBindTexture(GL_TEXTURE_2D, texID);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      // Load the image data
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, blob.data());

      // Return the blob
      return texID;
    }
    catch( Exception &error ) { 
      std::cout << "Caught exception: " << error.what() << std::endl;
      return 0;
    } 

    return 0;

  }


};

#endif

/* Illumination model
   0. Color on and Ambient off
   1. Color on and Ambient on
   2. Highlight on
   3. Reflection on and Ray trace on
   4. Transparency: Glass on, Reflection: Ray trace on
   5. Reflection: Fresnel on and Ray trace on
   6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
   7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
   8. Reflection on and Ray trace off
   9. Transparency: Glass on, Reflection: Ray trace off
   10. Casts shadows onto invisible surfaces
*/
