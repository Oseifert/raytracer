#include "pointlight.h"
#include "shapegroup.h"
#include "geometry.h"
#include "parse.h"
#include "material.h"
#include "main.h"
#include <stdlib.h>


PointLight::PointLight ()
: Light()
{
}


PointLight::~PointLight ()
{
}


Color3d PointLight::getDiffuse (Intersection& info)
{
  /*
   * Intensity of diffuse lighting is equal to the dot product of
   * the normal with the vector opposite the incident light's direction.
   * Then factor in attenuation.
   */
    
    Vector3d direction = info.iCoordinate - location;
    double distance = direction.length();
    direction.normalize();
    double angleFactor = -direction.dot(info.normal);
    
    if (angleFactor<0)
        return Color3d(0,0,0);  // light is falling on other side of surface
    else{
        Color3d answer =  color * info.material->getDiffuse(info) * angleFactor;
        double att = 1/(constAtten + linearAtten*distance + pow(distance,2)*quadAtten);
        return att*answer;
    }
}


Color3d PointLight::getSpecular (Intersection& info)
{
  /*
   * Intensity of specular lighting is the dot product of the light's
   * reflected ray and the ray pointing to the camera, raised to
   * some power (in this case, kshine). Then factor in attenuation.
   */
	//compute direction light falls on surface

    Vector3d direction = info.iCoordinate - location;
    double distance = direction.length();
    direction.normalize();
    Vector3d reflect=direction - info.normal*(2*direction.dot(info.normal));
    reflect.normalize();
    double angleFactor = - reflect.dot(info.theRay.getDir());
    if (angleFactor<0)
        return Color3d(0,0,0);
    else {
        angleFactor = pow(angleFactor,info.material->getKshine());
        double att = 1/(constAtten + (linearAtten*distance) + (pow(distance,2)*quadAtten));
        return   att * color * info.material->getSpecular() * angleFactor;
    }
}


bool PointLight::getShadow (Intersection& iInfo, ShapeGroup* root)
{
  /*
   * To determine if an intersection is in shadow or not with respect
   * to a light, cast a ray from the intersection towards the light
   * and see if it intersects anything. 
   */

    Vector3d direction = iInfo.iCoordinate - location;
    
    if (direction.dot(iInfo.normal)>0)
        return true;
    // otherwise we'll check
    Rayd shadowRay;
    shadowRay.setDir(direction*-1);
    double distance = shadowRay.getDir().length();
    shadowRay.getDir().normalize();
    shadowRay.setPos(iInfo.iCoordinate + iInfo.normal * EPSILON);
    Intersection tmpInfo;
    tmpInfo.theRay=shadowRay;
    double alpha = root->intersect(tmpInfo);
    if (alpha > EPSILON && alpha<distance)
        return true;
    return false;

}


void PointLight::glLoad (GLenum light)
{
  /*
   * load the specified OpenGL light as a point light with the 
   * correct position, attenuation and color.
   */
  // unfortunately, OpenGL requires it's input in this format

  GLfloat lightLoc[4] = {  static_cast<GLfloat>(location[0]), static_cast<GLfloat>(location[1]), static_cast<GLfloat>(location[2]), 1.0 };
  GLfloat lightCol[4] = { static_cast<GLfloat>(color[0]),    static_cast<GLfloat>(color[1]),    static_cast<GLfloat>(color[2]),    0.0 };


  glLightfv(light, GL_POSITION,              lightLoc);
  glLightfv(light, GL_DIFFUSE,               lightCol);
  glLightfv(light, GL_SPECULAR,              lightCol);
  glLightf (light, GL_CONSTANT_ATTENUATION,  constAtten);
  glLightf (light, GL_LINEAR_ATTENUATION,    linearAtten);
  glLightf (light, GL_QUADRATIC_ATTENUATION, quadAtten);

  glEnable(light);
}


void PointLight::glDraw ()
{
  /*
   * draw a point light as a small flat shaded sphere
   */

  glDisable(GL_LIGHTING);
  glColor3f(color[0], color[1], color[2]);

  //glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
    // create a small sphere at the light's location
    //glTranslatef(location[0], location[1], location[2]);
    //glutSolidSphere(0.1, 6, 6);
  //glPopMatrix();

  if (options->lighting)
    glEnable(GL_LIGHTING);
}


ostream&  PointLight::write     (ostream&       out)
{
  out << "#light_point" << flush;
  if (name[0] != '\0')
    out << " -n " << name << flush;
  out << " --" << endl;
  
  out << "  " << color << endl;
  out << "  " << location << endl;
  out << "  " << constAtten << " " << linearAtten << " " << quadAtten << endl;
  
  return out;
}
  

istream& PointLight::read (istream& in)
{
  int numFlags = 1;
  Flag flags[1] = { { (char*)"-n", STRING, false,  MAX_NAME, name    }
                  };

  if (parseFlags(in, flags, numFlags) == -1)
  {
    cerr << "ERROR: Point Light: flag parsing failed!" << endl;
    return in;
  }
  
  in >> color >> location;

  constAtten  = nextDouble(in);
  linearAtten = nextDouble(in);
  quadAtten   = nextDouble(in);
    constAtten = (constAtten>0)? constAtten : 0;
  linearAtten = (linearAtten>0)? linearAtten : 0;
  quadAtten = (quadAtten>0)? quadAtten : 0;
  if (constAtten==0 && linearAtten==0 && quadAtten==0)
  {
	  constAtten=1;
  }

  if (in.fail())
  {
    cerr << "ERROR: Point Light: unknown stream failure" << endl;
    return in;
  }

  return in;
}


istream& operator>> (istream& in, PointLight& l)
{
  return l.read(in);
}


ostream& operator<< (ostream& out, PointLight& l)
{
  return l.write(out);
}

