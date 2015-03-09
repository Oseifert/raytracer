#include "spotlight.h"
#include "shapegroup.h"
#include "geometry.h"
#include "parse.h"
#include "material.h"
#include "main.h"
#include <stdlib.h>


SpotLight::SpotLight ()
: Light ()
{
}


SpotLight::~SpotLight ()
{
}


Color3d SpotLight::getDiffuse (Intersection& info)
{
  /*
   * Intensity of diffuse lighting is equal to the dot product of
   * the normal with the vector opposite the incident light's direction.
   * Then factor in attenuation and the spotlight effect.
   */

    Vector3d direction = info.iCoordinate - location;
    double distance = direction.length();
    Vector3d beam = info.theRay.getDir();
    
    double angle = acos(direction.dot(beam)/(beam.length()*direction.length()));
    if(angle>cutOffAngle){
        return Color3d(0,0,0);
    }
    
    
    direction.normalize();
    double angleFactor = -direction.dot(info.normal);
    
    if (angleFactor<0)
        return Color3d(0,0,0);  // light is falling on other side of surface
    else{
        Color3d answer =  color * info.material->getDiffuse(info) * angleFactor;
        double att = 1/(constAtten + linearAtten*distance + pow(distance,2)*quadAtten);
        double sp = pow(direction.dot(beam),128*dropOffRate);
        return att*answer*sp;
    }

}


Color3d SpotLight::getSpecular (Intersection& info)
{
  /*
   * Intensity of specular lighting is the dot product of the light's
   * reflected ray and the ray pointing to the camera, raised to
   * some power (in this case, kshine). Then factor in attenuation and 
   * the spotlight effect.
   */


    Vector3d direction = info.iCoordinate - location;
    double distance = direction.length();
    Vector3d beam = info.theRay.getDir();
    
    double angle = acos(direction.dot(beam)/(beam.length()*direction.length()));
    if(angle>cutOffAngle || angle<0){
        return Color3d(0,0,0);
    }
    
    
    
    
    direction.normalize();
    Vector3d reflect=direction - info.normal*(2*direction.dot(info.normal));
    reflect.normalize();
    double angleFactor = - reflect.dot(info.theRay.getDir());
    if (angleFactor<0)
        return Color3d(0,0,0);
    else {
        angleFactor = pow(angleFactor,info.material->getKshine());
        double att = 1/(constAtten + (linearAtten*distance) + (pow(distance,2)*quadAtten));
        double sp = pow(direction.dot(beam),128*dropOffRate);
        return   sp*att * color * info.material->getSpecular() * angleFactor;
    }
}


bool SpotLight::getShadow (Intersection& iInfo, ShapeGroup* root)
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


void SpotLight::glLoad (GLenum light)
{
  /*
   * load the specified OpenGL light as a spot light with the
   * correct position, direction, spotlight attributes, attenuation and 
   * color.
   */

  // unfortunately, OpenGL requires its arguments in this format
  GLfloat vals[3][4] = { { static_cast<GLfloat>(location[0]),  static_cast<GLfloat>(location[1]),  static_cast<GLfloat>(location[2]),  1.0 },
                         { static_cast<GLfloat>(beamCenter[0]), static_cast<GLfloat>(beamCenter[1]), static_cast<GLfloat>(beamCenter[2]), 0.0 },
                         { static_cast<GLfloat>(color[0]),     static_cast<GLfloat>(color[1]),     static_cast<GLfloat>(color[2]),     0.0 }
                       };

  glLightfv(light, GL_POSITION,              vals[0]);
  glLightfv(light, GL_DIFFUSE,               vals[2]);
  glLightfv(light, GL_SPECULAR,              vals[2]);
  glLightf (light, GL_CONSTANT_ATTENUATION,  constAtten);
  glLightf (light, GL_LINEAR_ATTENUATION,    linearAtten);
  glLightf (light, GL_QUADRATIC_ATTENUATION, quadAtten);
  glLightfv(light, GL_SPOT_DIRECTION,        vals[1]);
  glLightf (light, GL_SPOT_CUTOFF,           rad2deg(cutOffAngle));
  glLightf (light, GL_SPOT_EXPONENT,         clamp(dropOffRate,0,1));

  glEnable(light);
}


void SpotLight::glDraw ()
{
  /* 
   * draw a point light as a small flat shaded sphere, with a wireframe
   * cone representing the volume the spotlight illuminates.
   */

  glDisable(GL_LIGHTING);
  glColor3f(color[0], color[1], color[2]);

  //glMatrixMode(GL_MODELVIEW);
  //glPushMatrix();
    // draw a small sphere at the light's location
    //glTranslatef(location[0], location[1], location[2]);
    //glutSolidSphere(0.5, 6, 6);

    // draw a cone representing the light from the spotlight
    //glRotatef(rad2deg(acos(-beamCenter[2])),
     //         direction[1], -beamCenter[0], 0.0);
    //glTranslatef(0,0,-10);
    //glutWireCone(10 * tan(cutOffAngle), 10, 8, 1);
  //glPopMatrix();

  if (options->lighting)
    glEnable(GL_LIGHTING);
}


ostream&  SpotLight::write     (ostream&       out)
{
  out << "#light_spot" << flush;
  if (name[0] != '\0')
    out << " -n " << name << flush;
  out << " --" << endl;
  
  out << "  " << color << endl;
  out << "  " << location << endl;
  out << "  " << beamCenter << endl;
  out << "  " << constAtten << " " << linearAtten << " " << quadAtten << endl;
  out << "  " << rad2deg(cutOffAngle) << " " << dropOffRate << endl;
  
  return out;
}
  

istream& SpotLight::read (istream& in)
{
  int numFlags = 1;
  Flag flags[1] = { { (char*)"-n", STRING, false,  MAX_NAME, name }
                  };

  if (parseFlags(in, flags, numFlags) == -1)
  {
    cerr << "ERROR: Spot Light: flag parsing failed!" << endl;
    return in;
  }
  
  in >> color >> location >> beamCenter;

  constAtten  = nextDouble(in);
  linearAtten = nextDouble(in);
  quadAtten   = nextDouble(in);
  cutOffAngle = deg2rad(nextDouble(in));
  dropOffRate = nextDouble(in);

  beamCenter.normalize();
  
  if (in.fail())
  {
    cerr << "ERROR: Spot Light: unknown stream failure" << endl;
    return in;
  }

  return in;
}


istream& operator>> (istream& in, SpotLight& l)
{
  return l.read(in);
}


ostream& operator<< (ostream& out, SpotLight& l)
{
  return l.write(out);
}

