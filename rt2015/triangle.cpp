#include "triangle.h"
#include "parse.h"
#include "material.h"
#include "main.h"


Triangle::Triangle ()
: Shape ()
{
}


Triangle::~Triangle ()
{
}


double Triangle::intersect (Intersection& intersectionInfo)
{
	/*
	* This method solves for intersection between intersectionInfo.theRay and
	* the triangle. 
	*   If there is no intersection OR if the ray lies in the
	*	plane containing the triangle we return -1;
	*	Otherwise we return alpha the distance from the
	*	starting point of intersectionInfo.theRay to the intersection point
	
	*
	* Then, decompose the vector p - v0 into a linear combination of
	*/
    
    // RAY_CASTING TODO (intersection)
    // (1) determine intersection with plane (if any)
    // (2) test if intersection point is in triangle
    // (3) compute distance from start of ray to intersection point & normal in direction of incoming ray

//    1. check if n dot v = 0; if so return -1
//        2. next alpha = (n dot u) / (n dot v)
//        3. if alpha <0 there is no intersection, return -1
//            4. next you use alpha to solve for beta and gamma. (note, you should pre-solve the system of equations so basically you have beta = some function of alpha, v, us, w1, w2 and
//            gamma = some function of alpha, v, u, w1, w2, beta
//    5. test that beta>=0, gamma>=0 and beta+gamma<=1. if so you have an intersection point -- if not return -1
    
    Vector3d vectorV = intersectionInfo.theRay.getDir();
    Vector3d u = v[0] - intersectionInfo.theRay.getPos();
    vectorV.normalize();
    
    if(n.dot(vectorV)==0){
        return -1;
    }
    
    double alpha = (n.dot(u))/(n.dot(vectorV));
    
    if(alpha<0){
        return -1;
    }

    
//    Vector3d w1 = v[1] -v[0];
//    Vector3d w2 = v[2]-v[0];
//    Vector3d A = (alpha*vectorV)-n;
//    double A1 = A.dot(w1);
//    double A2 = A.dot(w2);
//    double w1w1 = w1.dot(w1);
//    double w2w2 = w2.dot(w2);
//    double w1w2 = w1.dot(w2);
//    
//    double beta = (A1 - w1w2*((A2 - (A1*w1w2)/w1w1)/(w2w2 - w1w2 * w1w2 / w1w1))) / w1w1;
//    
//    double gamma = (A2 - (A1 * w1w2)/w1w1)/(w2w2 - w1w2 * w1w2 / w1w1);
    
    Vector3d w1 = v[1] -v[0];
    Vector3d w2 = v[2]-v[0];
    double w1LS = w1.dot(w1);
    double w2LS = w2.dot(w2);
    Vector3d avn = (alpha*vectorV) -n;
    
    double beta = ((w2LS * avn.dot(w1)) - (w1.dot(w2)*(avn.dot(w2))))  /
                    ((w2LS * w1LS)- pow(w1.dot(w2),2));
    
    double gamma = (avn.dot(w2) - beta * w1.dot(w2))  /
                    (w2LS);
    
    if(beta>=0 && gamma>=0 && (beta+gamma) <=1){
        
        intersectionInfo.iCoordinate = intersectionInfo.theRay.getPos() + (alpha * vectorV);
        
        intersectionInfo.normal = n.normalize();
        
        if(n.dot(intersectionInfo.iCoordinate -
                 intersectionInfo.theRay.getPos()) > 0){
            intersectionInfo.normal *=-1;
        }
        
        intersectionInfo.material = material;
        intersectionInfo.entering = true;
        return alpha;
    }
    
	return -1;
}




void Triangle::glDraw ()
{
	/*
	* draw the sphere with the appropriate material and textured status
	* at the desired position and radius
	*/

	material->glLoad();

	if (options->textures && textured && material->textured())
	{
		glEnable(GL_TEXTURE_2D);
		// draw the triangle with normal and tex coords
		glBegin(GL_TRIANGLES);
		n.glLoad();
		t[0].glLoad();
		v[0].glLoad();
		t[1].glLoad();
		v[1].glLoad();
		t[2].glLoad();
		v[2].glLoad();
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		// draw the triangle with normal but no tex coords
		glBegin(GL_TRIANGLES);
		n.glLoad();
		v[0].glLoad();
		v[1].glLoad();
		v[2].glLoad();
		glEnd();
	}

	material->glUnload();
}


inline istream& operator>> (istream& in, Triangle& t)
{
	return t.read(in);
}


istream& Triangle::read (istream& in)
{
	int numFlags = 4;
	Flag flags[4] = { { (char*)"-n", STRING, false, MAX_NAME,     name      },
	{ (char*)"-m", STRING, true,  MAX_NAME,     matname   },
	{ (char*)"-t", BOOL,   false, sizeof(bool), &textured },
	{ (char*)"-u", DOUBLE, false, sizeof(double), &bumpScale }

	};

	if (parseFlags(in, flags, numFlags) == -1)
	{
		cerr << "ERROR: Triangle: flag parsing failed!" << endl;
		return in;
	}
	if (bumpScale != 0)
		bumpMapped = true;

	if (textured || bumpMapped)
		in >> v[0] >> t[0] >> v[1] >> t[1] >> v[2] >> t[2];
	else
		in >> v[0] >> v[1] >> v[2];

	if (in.fail())
	{
		cerr << "ERROR: Triangle: unknown stream error" << endl;
		return in;
	}

	// Vectors from vertex 0 to the other vertices
	Vector3d v1 = v[1]-v[0];
	Vector3d v2 = v[2]-v[0];  

	// Calculate this triangle's normal
	n = (v1).cross(v2);
	n.normalize();

	return in;
}


inline ostream& operator<< (ostream& out, Triangle& t)
{
	return t.write(out);
}


ostream& Triangle::write (ostream& out)
{
	out << "#triangle -m " << matname << flush;
	if (name[0] != '\0')
		out << " -n " << name << flush;
	if (textured)
		out << " -t" << flush;
	out << " --" << endl;

	if (textured)
	{
		out << "  " << v[0] << "   " << t[0] << endl
			<< "  " << v[1] << "   " << t[1] << endl
			<< "  " << v[2] << "   " << t[2] << endl;
	}
	else
	{
		out << "  " << v[0] << endl
			<< "  " << v[1] << endl
			<< "  " << v[2] << endl;
	}

	return out;
}
