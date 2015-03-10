#include "box.h"
#include "parse.h"
#include "material.h"
#include "main.h"
#include "cmath"


Box::Box ()
: Shape ()
{
}


Box::~Box ()
{
}


double Box::planeIntersect (Rayd& ray, Point3d& p0, Vector3d& n)
{
    Vector3d direction = p0 - ray.getPos();
    if(direction.dot(n)==0){
        return -1;
    }
    
    double dist = (direction.dot(n))/(ray.getDir().dot(n));
    Point3d intersect = ray.getPos() + (dist*ray.getDir());

    if(n[0] == 1 || n[0] == -1){
        if(abs(double(intersect[1]-center[1])) < size[1]  &&
           abs(double(intersect[2]-center[2])) < size[2]){
            return dist;
        }
    }
  
    else if(n[1] == 1 || n[1] == -1){
        if(abs(double(intersect[0]-center[0])) < size[0]  &&
           abs(double(intersect[2]-center[2])) < size[2]){
            return dist;
        }
    }
    else if(n[2] == 1 || n[2] == -1){
        if(abs(double(intersect[0]-center[0])) < size[0]  &&
            abs(double(intersect[1]-center[1])) < size[1]){
            return dist;
        }
    }
    return -1;
}


double Box::intersect (Intersection& info)
{
    Vector3d normals[] = {Vector3d(1,0,0),Vector3d(0,1,0),Vector3d(0,0,1),
                        Vector3d(-1,0,0),Vector3d(0,-1,0),Vector3d(0,0,-1)};
    
    Point3d p0;
    double alpha = -1;
    double newAlpha;
    double sign = 1;
    
    for(int i =0; i<6;++i){
        if (i>2) {
            sign = -1;
        }
        
        p0 = center+((sign *size[i%3]/2) * normals[i]);
        
        newAlpha = planeIntersect(info.theRay, p0, normals[i]);
        
        if(alpha==-1 && newAlpha>0){
            alpha = newAlpha;
            info.normal = normals[i];
        }
        else if(newAlpha>0 && newAlpha<alpha){
            alpha = newAlpha;
            info.normal = normals[i];
        }
    }
    if (alpha>0) {
        info.iCoordinate = info.theRay.getPos() + (info.theRay.getDir() * alpha);
        info.material = material;

        info.entering = true;
        
        if(info.normal.dot(info.theRay.getDir()) > 0){
            info.normal *= -1;
            info.entering = false;
        }
        info.normal.normalize();
        return alpha;
    }
    
	return -1;
}


TexCoord2d Box::getTexCoordinates (Point3d i)
{
// skeleton delete
	TexCoord2d tCoord(0.0, 0.0);

	return tCoord;
}



void Box::glutTexturedCube (GLfloat size)
{
	static GLfloat n[6][3] =
	{ 
		{-1.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, -1.0, 0.0},
		{0.0, 0.0, 1.0},
		{0.0, 0.0, -1.0}
	};
	static GLint faces[6][4] =
	{
		{0, 1, 2, 3},
		{3, 2, 6, 7},
		{7, 6, 5, 4},
		{4, 5, 1, 0},
		{5, 6, 2, 1},
		{7, 4, 0, 3}
	};
	GLfloat v[8][3];
	GLint i;

	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -size / 2;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = size / 2;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -size / 2;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = size / 2;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = -size / 2;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = size / 2;

	for (i = 5; i >= 0; i--) {
		glBegin(GL_QUADS);
		glNormal3fv(&n[i][0]);
		glTexCoord2f(0.0,0.0);
		glVertex3fv(&v[faces[i][0]][0]);
		glTexCoord2f(1.0,0.0);
		glVertex3fv(&v[faces[i][1]][0]);
		glTexCoord2f(1.0,1.0);
		glVertex3fv(&v[faces[i][2]][0]);
		glTexCoord2f(0.0,1.0);
		glVertex3fv(&v[faces[i][3]][0]);
		glEnd();
	}
}


void Box::glDraw ()
{
	material->glLoad();

	glPushMatrix();
	// move to the origin of the Box
	glTranslatef(center[0], center[1], center[2]);

	// scale to the appropriate size
	glScalef(size[0], size[1], size[2]);

	if (options->textures && textured && material->textured())
	{
		glEnable(GL_TEXTURE_2D);
	}

	// draw a unit cube
	glutTexturedCube(1);
	glPopMatrix();

	material->glUnload();
	glDisable(GL_TEXTURE_2D);
}


inline istream& operator>> (istream& in, Box& s)
{
	return s.read(in);
}


istream& Box::read (istream& in)
{
	int numFlags = 4;
	Flag flags[4] = { { (char*)"-n", STRING, false, MAX_NAME,       name      },
	{ (char*)"-m", STRING, true,  MAX_NAME,       matname   },
	{ (char*)"-t", BOOL,   false, sizeof(bool),   &textured },
	{ (char*)"-u", DOUBLE, false, sizeof(double), &bumpScale }
	};

	if (parseFlags(in, flags, numFlags) == -1)
	{
		cerr << "ERROR: Box: flag parsing failed!" << endl;
		return in;
	}

	if (bumpScale != 0)
		bumpMapped = true;

	in >> center;
	in >> size;

	size[0] = fabs(size[0]);
	size[1] = fabs(size[1]);
	size[2] = fabs(size[2]);

	if (in.fail())
	{
		cerr << "ERROR: Box: unknown stream error" << endl;
		return in;
	}

	return in;
}


inline ostream& operator<< (ostream& out, Box& s)
{
	return s.write(out);
}


ostream& Box::write (ostream& out)
{
	out << "#box -m " << matname << flush;
	if (name[0] != '\0')
		out << " -n " << name << flush;
	if (textured)
		out << " -t" << flush;
	if (bumpMapped)
		out << " -u" << flush;
	out << " --" << endl;

	out << "  " << center << endl;
	out << "  " << size << endl;

	return out;
}
