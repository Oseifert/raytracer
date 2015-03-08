#include "sphere.h"
#include "parse.h"
#include "material.h"
#include "main.h"
#include "common.h"

#define PI 3.14159
// single glu quadric object to do opengl sphere drawing
static GLUquadricObj* sphereQuadric = gluNewQuadric();


Sphere::Sphere ()
: Shape ()
{
}


Sphere::~Sphere ()
{
}


double Sphere::intersect (Intersection& intersectionInfo)
{
	/*
	* This method determines if intersectionInfo.theRay visibly intersects
	* the sphere from the position of the start of intersectionInfo.theRay
    *
    * The first visible intersection point is at alpha along
    * the ray where
	*	alpha is the smallest non-negative real root of
	*	0=||v||^2 alpha^2 - 2 (u dot v)alpha + ||u||^2 - r^2
	*	where 
	*	v is the unit direction vector of intersectionInfo.theRay
	*   u is the vector for the start of intersectionInfo.theRay to the
	*		center of the sphere
	*	r is the radius of the sphere
	*/
    
    
    Vector3d v = intersectionInfo.theRay.getDir();
    Vector3d u;
    u = center - intersectionInfo.theRay.getPos();
    
//    u[0] = center[0] - intersectionInfo.theRay.getPos()[0];
//    u[1] = center[1] - intersectionInfo.theRay.getPos()[1];
//    u[2] = center[2] - intersectionInfo.theRay.getPos()[2];
    
    double r = radius;
    
    double a = pow(v.length(), 2);
    double b = (-2)*(u.dot(v));
    double c = pow(u.length(),2) - pow(r, 2);
    
    double d = (b*b) - (4*a*c);
    
    double root1;
    double root2;
    
    if(d>=0){
        root1 = (-b+sqrt(d))/(2*a);
        root2 = (-b-sqrt(d))/(2*a);
    }
    //WE NEED TO ACCOUNT FOR NEGATIVE ROOTS
    
    else{
        return -1;
    }
 
    double alpha=-1;
    
    if(root1<0 && root2>0){
        alpha = root2;
    }
    else if (root1>0 && root2<0){
        alpha = root1;
    }
    else if (root1>0 && root2>0){
        alpha = min(root1,root2);
    }

    
   
    intersectionInfo.iCoordinate = intersectionInfo.theRay.getPos() + (alpha * intersectionInfo.theRay.getDir());
    
    intersectionInfo.normal = intersectionInfo.iCoordinate - center;
    
    intersectionInfo.normal.normalize();
    
    intersectionInfo.material = material;
    
    //dont' need textures yet.
    //intersectionInfo.textured = textured;
    
    //intersectionInfo.texCoordinate = texCoordinate;
    
    intersectionInfo.entering = true;
    
    if(intersectionInfo.normal.dot(v) > 0){
        intersectionInfo.normal *= -1;
        intersectionInfo.entering = false;
    }
    
    if(textured){
        
        intersectionInfo.textured = true;
        double r = radius;
        Point3d newIntersect = intersectionInfo.iCoordinate - center;
        
        double phi = atan2(newIntersect[1] , newIntersect[0]);
        double theta = acos(newIntersect[2] / r);
        
        double normTheta = theta / PI;
        double normPhi = (phi + PI) / (2*PI);
        
        intersectionInfo.texCoordinate[0] = normPhi;
        intersectionInfo.texCoordinate[1] = normTheta;
        
    }
    
    if(bumpMapped){
    
        Vector3d v = Vector3d(0,1,0);
        Vector3d up = v - intersectionInfo.normal.dot(v) * intersectionInfo.normal;
        
        double r = radius;
        Point3d newIntersect = intersectionInfo.iCoordinate - center;
        
        double phi = atan2(newIntersect[1] , newIntersect[0]);
        double theta = acos(newIntersect[2] / r);
        
        double normTheta = theta / PI;
        double normPhi = (phi + PI) / (2*PI);
        
        intersectionInfo.texCoordinate[0] = normPhi;
        intersectionInfo.texCoordinate[1] = normTheta;
        
        up.normalize();
        Vector3d right = intersectionInfo.normal.cross(up);
        
        Vector3d n = intersectionInfo.normal;
        
        material->bumpNormal(n, up, right, intersectionInfo, bumpScale);
        

        
    }
    
    // RAY_CASTING TODO (sphere intersection)
    // Determine if intersectionInfo.theRay intersects the sphere in front of the camera
    // if so, store intersectionInfo on intersection point in intersectionInfo
    // RAY_CASTING TODO (sphere intersection)
    
    return alpha;

}


void Sphere::glDraw ()
{
	/*
	* draw the sphere with the appropriate material and textured status
	* at the desired position and radius, using a glu quadric (for
	* easy texture coordinate generation)
	*/

	material->glLoad();

	glPushMatrix();
	// move to the origin of the sphere
	glTranslatef(center[0], center[1], center[2]);

	// set up the glu object's parameters - smooth filled polys with normals
	gluQuadricDrawStyle(sphereQuadric, GLU_FILL);
	gluQuadricOrientation(sphereQuadric, GLU_OUTSIDE);
	gluQuadricNormals(sphereQuadric, GLU_SMOOTH);

	// only calculate tex coords if we need to
	if (options->textures && textured && material->textured())
	{
		gluQuadricTexture(sphereQuadric, GLU_TRUE);
		glEnable(GL_TEXTURE_2D);
	}

	// draw the sphere
	gluSphere(sphereQuadric, radius, 50, 50);
	glPopMatrix();

	material->glUnload();

	glDisable(GL_TEXTURE_2D);
}


inline istream& operator>> (istream& in, Sphere& s)
{
	return s.read(in);
}


istream& Sphere::read (istream& in)
{
	int numFlags = 4;
	Flag flags[4] = { { (char*)"-n", STRING, false, MAX_NAME,     name      },
	{ (char*)"-m", STRING, true,  MAX_NAME,     matname   },
	{ (char*)"-t", BOOL,   false, sizeof(bool), &textured },
	{ (char*)"-u", DOUBLE, false, sizeof(double), &bumpScale }
	};

	if (parseFlags(in, flags, numFlags) == -1)
	{
		cerr << "ERROR: Sphere: flag parsing failed!" << endl;
		return in;
	}

	if (bumpScale != 0)
		bumpMapped = true;


	in >> center;

	radius = nextDouble(in);

	if (in.fail())
	{
		cerr << "ERROR: Sphere: unknown stream error" << endl;
		return in;
	}

	return in;
}


inline ostream& operator<< (ostream& out, Sphere& s)
{
	return s.write(out);
}


ostream& Sphere::write (ostream& out)
{
	out << "#sphere -m " << matname << flush;
	if (name[0] != '\0')
		out << " -n " << name << flush;
	if (textured)
		out << " -t" << flush;
	out << " --" << endl;

	out << "  " << center << endl;
	out << "  " << radius << endl;

	return out;
}
