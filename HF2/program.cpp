//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2010-tol.          
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk. 
// A Hazi feladat csak ebben a fajlban lehet. 
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni (printf is fajlmuvelet!)
// ---------------------------------------------------------
// A keretben szereplokon kivul csak az anyagban szereplo gl/glu/glut fuggvenyek hasznalhatok: 
// Rendering pass: glBegin, glVertex[2|3]f, glColor3f, glNormal3f, glTexCoord2f, glEnd, glDrawPixels
// Transzformaciok: glViewport, glMatrixMode, glLoadIdentity, glMultMatrixf, gluOrtho2D, 
// glTranslatef, glRotatef, glScalef, gluLookAt, gluPerspective, glPushMatrix, glPopMatrix,
// Illuminacio: glMaterialfv, glMaterialfv, glMaterialf, glLightfv
// Texturazas: glGenTextures, glBindTexture, glTexParameteri, glTexImage2D, glTexEnvi, 
// Pipeline vezerles: glShadeModel, glEnable/Disable a kovetkezokre:
// GL_LIGHTING, GL_NORMALIZE, GL_DEPTH_TEST, GL_CULL_FACE, GL_TEXTURE_2D, GL_BLEND, GL_LIGHT[0..7]
//
// A beadott program nem tartalmazhat felesleges programsorokat!
//=============================================================================================

#include <math.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// MsWindows-on ez is kell
#include <windows.h>     
#endif // Win32 platform

#include <GL/gl.h>
#include <GL/glu.h>
// A GLUT-ot le kell tolteni: http://www.opengl.org/resources/libraries/glut/
#include <GL/glut.h>     

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

//--------------------------------------------------------
// Nev    : Dudás Ádám
// Neptun : WG9114
//--------------------------------------------------------

struct Vector
{
	double x, y;

	Vector()
	{
		x = y = 0.0f;
	}

	Vector(double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	Vector operator+=(Vector other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	Vector operator+(Vector other)
	{
		Vector v = *this;
		v += other;
		return v;
	}

	Vector operator-=(Vector other)
	{
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}

	Vector operator-(Vector other)
	{
		Vector v = *this;
		v -= other;
		return v;
	}

	Vector operator*=(double scalar)
	{
		this->x *= scalar;
		this->y *= scalar;
		return *this;
	}

	Vector operator*(double scalar)
	{
		Vector v = *this;
		v *= scalar;
		return v;
	}

	bool operator==(Vector a)
	{
		return (abs(x - a.x) < 0.000001) && (abs(y - a.y) < 0.000001);
	}

	bool operator!=(Vector a)
	{
		return !((*this) == a);
	}

	void Normalize()
	{
		double length = Length();
		x /= length;
		y /= length;
	}

	double Dot(Vector b)
	{
		return ((this->x * b.x) + (this->y * b.y));
	}

	static double Area(Vector a, Vector b)
	{
		return (a.x*b.y - a.y*b.x);
	}

	double Length()
	{
		return sqrt(x*x + y*y);
	}
} normal_control_points[] =
	{
		Vector(1.f,		3.f),
		Vector(1.25f,	2.75f),
		Vector(1.125f,	2.5f),
		Vector(1.1f,	2.35f),
		Vector(1.25f,	2.25f),
		Vector(1.625f,	2.f),
		Vector(1.875f,	1.75f),
		Vector(1.9f,	1.6f),
		Vector(1.75f,	1.6f),
		Vector(1.25f,	2.f),
		Vector(1.25f,	1.5f),
		Vector(1.5f,	0.5f),
		Vector(1.5f,	0.25f),
		Vector(1.25f,	0.5f),
		Vector(1.f,		1.25f), // lábközép
		Vector(0.75f,	0.5f),
		Vector(0.5f,	0.25f),
		Vector(0.5f,	0.5f),
		Vector(0.75f,	1.5f),
		Vector(0.75f,	2.f),
		Vector(0.25f,	1.6f),
		Vector(0.1f,	1.6f),
		Vector(0.125f,	1.75f),
		Vector(0.375f,	2.f),
		Vector(0.75f,	2.25f),
		Vector(0.9f,	2.35f),
		Vector(0.875f,	2.5f),
		Vector(0.75f,	2.75f)
	}, jumping_control_points[] =
	{
		Vector(1.f,		3.f),
		Vector(1.25f,	2.75f),
		Vector(1.125f,	2.5f),
		Vector(1.1f,	2.35f),
		Vector(1.25f,	2.25f),
		Vector(1.625f,	2.f),
		Vector(1.875f,	1.75f),
		Vector(1.9f,	1.6f),
		Vector(1.75f,	1.6f),
		Vector(1.25f,	2.f),
		Vector(1.25f,	1.5f),
		Vector(1.80f,	1.2f),
		Vector(1.95f,	1.f),
		Vector(1.80f,	0.95f),
		Vector(1.f,		1.25f), // lábközép
		Vector(0.2f,	0.95f),
		Vector(0.05f,	1.f),
		Vector(0.2f,	1.2f),
		Vector(0.75f,	1.5f),
		Vector(0.75f,	2.f),
		Vector(0.25f,	1.6f),
		Vector(0.1f,	1.6f),
		Vector(0.125f,	1.75f),
		Vector(0.375f,	2.f),
		Vector(0.75f,	2.25f),
		Vector(0.9f,	2.35f),
		Vector(0.875f,	2.5f),
		Vector(0.75f,	2.75f)
	};
#define NUM_OF_CTRL_POINTS 28

struct Color
{
	double r, g, b;

	Color()
	{
		r = g = b = 0.f;
	}

	Color(double red, double green, double blue)
	{
		r = red;
		g = green;
		b = blue;
	}
} monochromatic[] = 
	{
		Color(1.83970e-003, -4.53930e-004, 1.21520e-002),
		Color(4.61530e-003, -1.04640e-003, 3.11100e-002),
		Color(9.62640e-003, -2.16890e-003, 6.23710e-002),
		Color(1.89790e-002, -4.43040e-003, 1.31610e-001),
		Color(3.08030e-002, -7.20480e-003, 2.27500e-001),
		Color(4.24590e-002, -1.25790e-002, 3.58970e-001),
		Color(5.16620e-002, -1.66510e-002, 5.23960e-001),
		Color(5.28370e-002, -2.12400e-002, 6.85860e-001),
		Color(4.42870e-002, -1.99360e-002, 7.96040e-001),
		Color(3.22200e-002, -1.60970e-002, 8.94590e-001),
		Color(1.47630e-002, -7.34570e-003, 9.63950e-001),
		Color(-2.33920e-003, 1.36900e-003, 9.98140e-001),
		Color(-2.91300e-002, 1.96100e-002, 9.18750e-001),
		Color(-6.06770e-002, 4.34640e-002, 8.24870e-001),
		Color(-9.62240e-002, 7.09540e-002, 7.85540e-001),
		Color(-1.37590e-001, 1.10220e-001, 6.67230e-001),
		Color(-1.74860e-001, 1.50880e-001, 6.10980e-001),
		Color(-2.12600e-001, 1.97940e-001, 4.88290e-001),
		Color(-2.37800e-001, 2.40420e-001, 3.61950e-001),
		Color(-2.56740e-001, 2.79930e-001, 2.66340e-001),
		Color(-2.77270e-001, 3.33530e-001, 1.95930e-001),
		Color(-2.91250e-001, 4.05210e-001, 1.47300e-001),
		Color(-2.95000e-001, 4.90600e-001, 1.07490e-001),
		Color(-2.97060e-001, 5.96730e-001, 7.67140e-002),
		Color(-2.67590e-001, 7.01840e-001, 5.02480e-002),
		Color(-2.17250e-001, 8.08520e-001, 2.87810e-002),
		Color(-1.47680e-001, 9.10760e-001, 1.33090e-002),
		Color(-3.51840e-002, 9.84820e-001, 2.11700e-003),
		Color(1.06140e-001, 1.03390e+000, -4.15740e-003),
		Color(2.59810e-001, 1.05380e+000, -8.30320e-003),
		Color(4.19760e-001, 1.05120e+000, -1.21910e-002),
		Color(5.92590e-001, 1.04980e+000, -1.40390e-002),
		Color(7.90040e-001, 1.03680e+000, -1.46810e-002),
		Color(1.00780e+000, 9.98260e-001, -1.49470e-002),
		Color(1.22830e+000, 9.37830e-001, -1.46130e-002),
		Color(1.47270e+000, 8.80390e-001, -1.37820e-002),
		Color(1.74760e+000, 8.28350e-001, -1.26500e-002),
		Color(2.02140e+000, 7.46860e-001, -1.13560e-002),
		Color(2.27240e+000, 6.49300e-001, -9.93170e-003),
		Color(2.48960e+000, 5.63170e-001, -8.41480e-003),
		Color(2.67250e+000, 4.76750e-001, -7.02100e-003),
		Color(2.80930e+000, 3.84840e-001, -5.74370e-003),
		Color(2.87170e+000, 3.00690e-001, -4.27430e-003),
		Color(2.85250e+000, 2.28530e-001, -2.91320e-003),
		Color(2.76010e+000, 1.65750e-001, -2.26930e-003),
		Color(2.59890e+000, 1.13730e-001, -1.99660e-003),
		Color(2.37430e+000, 7.46820e-002, -1.50690e-003),
		Color(2.10540e+000, 4.65040e-002, -9.38220e-004),
		Color(1.81450e+000, 2.63330e-002, -5.53160e-004),
		Color(1.52470e+000, 1.27240e-002, -3.16680e-004),
		Color(1.25430e+000, 4.50330e-003, -1.43190e-004),
		Color(1.00760e+000, 9.66110e-005, -4.08310e-006),
		Color(7.86420e-001, -1.96450e-003, 1.10810e-004),
		Color(5.96590e-001, -2.63270e-003, 1.91750e-004),
		Color(4.43200e-001, -2.62620e-003, 2.26560e-004),
		Color(3.24100e-001, -2.30270e-003, 2.15200e-004),
		Color(2.34550e-001, -1.87000e-003, 1.63610e-004),
		Color(1.68840e-001, -1.44240e-003, 9.71640e-005),
		Color(1.20860e-001, -1.07550e-003, 5.10330e-005),
		Color(8.58110e-002, -7.90040e-004, 3.52710e-005),
		Color(6.02600e-002, -5.67650e-004, 3.12110e-005),
		Color(4.14800e-002, -3.92740e-004, 2.45080e-005),
		Color(2.81140e-002, -2.62310e-004, 1.65210e-005),
		Color(1.91170e-002, -1.75120e-004, 1.11240e-005),
		Color(1.33050e-002, -1.21400e-004, 8.69650e-006),
		Color(9.40920e-003, -8.57600e-005, 7.43510e-006),
		Color(6.51770e-003, -5.76770e-005, 6.10570e-006),
		Color(4.53770e-003, -3.90030e-005, 5.02770e-006),
		Color(3.17420e-003, -2.65110e-005, 4.12510e-006)
	};
#define NUM_OF_WAVELENGTHS 69

class VectorList
{
private:
	Vector* list;
	int virt_size;
	int act_size;
public:
	VectorList(int initSize = 100)
	{
		virt_size = 0;
		act_size = initSize;
		list = new Vector[act_size];
	}

	~VectorList()
	{
		virt_size = 0;
		act_size = 0;
		delete list;
	}

	int size()
	{
		return virt_size;
	}

	Vector& operator[](int index)
	{
		if ((index >= 0) && (index < virt_size))
		{
			return list[index];
		}
		else
		{
			throw "Argument Exception";
		}
	}

	void add(Vector v)
	{
		++virt_size;

		if (virt_size > act_size)
		{
			// creat new
			Vector* new_list = new Vector[act_size * 2];

			// memcpy
			for (int i = 0; i < act_size; i++)
			{
				new_list[i] = list[i];
			}
			
			// delete old
			delete list;

			// assign new
			act_size *= 2;
			list = new_list;
		}
		list[virt_size - 1] = v;
	}

	void insert(int index, Vector v)
	{
		if (virt_size + 1 > act_size)
		{
			// creat new
			Vector* new_list = new Vector[act_size * 2];

			// memcpy
			for (int i = 0; i < act_size; i++)
			{
				new_list[i] = list[i];
			}
			
			// delete old
			delete list;

			// assign new
			act_size *= 2;
			list = new_list;
		}

		for (int i = virt_size; i > index; i--)
		{
			list[i] = list[i-1];
		}

		list[index] = v;
		virt_size++;
	}

	void remove(int index)
	{
		if ((index >= 0) && (index < virt_size))
		{
			for (int i = index; i < virt_size - 1; i++)
			{
				list[i] = list[i + 1];
			}
			virt_size--;
		}
		else
		{
			throw "Argument Exception";
		}
	}

	Vector& last()
	{
		return list[virt_size - 1];
	}
};

void DrawBox()
{
	glBegin(GL_LINE_STRIP);
	glColor3d(0.5f, 0.5f, 0.5f);
	glVertex2d(0.f, 0.f);
	glVertex2d(0.f, 3.f);
	glVertex2d(2.f, 3.f);
	glVertex2d(2.f, 0.f);
	glVertex2d(0.f, 0.f);
	glEnd();
}

class Balerina
{
private:
	Vector fill_direction;
	double fill_length;
	
protected:
	Vector* control_points;
	Vector translation;
	bool is_jumping;
	
	void setFillDirection(Vector dir)
	{
		fill_direction = dir;
		fill_direction.Normalize();
		fill_length = fill_direction.Dot(Vector(2.f, 3.f));
	}

	Color GetColorAtPoint(Vector point)
	{
		double dist = fill_direction.Dot(point);
		// normalize dist *= 1/sqrt(2*2 + 3*3)
		dist *= 0.27735f;
		
		return monochromatic[NUM_OF_WAVELENGTHS - (int)(dist * NUM_OF_WAVELENGTHS)];
	}

	bool isPointInTriangle(Vector a, Vector b, Vector c, Vector p)
	{
		Vector vc = c - a;
		Vector vb = b - a;
		Vector vp = p - a;

		double dotcc = vc.Dot(vc);
		double dotcb = vc.Dot(vb);
		double dotcp = vc.Dot(vp);
		double dotbb = vb.Dot(vb);
		double dotbp = vb.Dot(vp);

		double invDenom = 1 / ((dotcc * dotbb) - (dotcb * dotcb));
		double u = ((dotbb * dotcp) - (dotcb * dotbp)) * invDenom;
		double v = ((dotcc * dotbp) - (dotcb * dotcp)) * invDenom;

		return ((u > 0) && (v > 0) && (u + v < 1));
	}

	void getFilling(VectorList& curveVertices, double gridSize, VectorList& fillingVertices)
	{
		double invGridSize = 1 / gridSize;
		VectorList gridVertices;

		for (int i = 0; i <= curveVertices.size(); i++)
		{
			// two points from the curve
			Vector v0 = curveVertices[i%curveVertices.size()];
			Vector v1 = curveVertices[(i + 1) % curveVertices.size()];

			int xMin = floor(((v0.x < v1.x)?v0.x:v1.x) * invGridSize) - 1;
			int xMax = floor(((v0.x > v1.x)?v0.x:v1.x) * invGridSize) + 1;
			int yMin = floor(((v0.y < v1.y)?v0.y:v1.y) * invGridSize) - 1;
			int yMax = floor(((v0.y > v1.y)?v0.y:v1.y) * invGridSize) + 1;

			// determine third point from grid
			double minDist = 1000; // infinity
			Vector v2;

			for (int x = xMin; x <= xMax; x++)
			{
				for (int y = yMin; y <= yMax; y++)
				{
					Vector v = Vector(x * gridSize, y * gridSize);
					Vector va = v0 - v;
					Vector vb = v1 - v;

					if (Vector::Area(vb, va) > 0.00001)
					{
						double dist = va.Length() + vb.Length();
						if (dist < minDist)
						{
							minDist = dist;
							v2 = v;
						}
					}
				}
			}

			// save triangle
			fillingVertices.add(v0);
			fillingVertices.add(v1);
			fillingVertices.add(v2);
			
			if (i == 0)
			{
				gridVertices.add(v2);
			}

			// fill touched cell
			if (gridVertices.last() != v2)
			{
				// fill between triangles
				fillingVertices.add(v0);
				fillingVertices.add(v2);
				fillingVertices.add(gridVertices.last());

				// fill if any left
				if ((abs(gridVertices.last().x - v2.x) > 0.000001) && (abs(gridVertices.last().y - v2.y) > 0.000001))
				{
					Vector vt;
					
					if (((gridVertices.last().x > v2.x) && (gridVertices.last().y > v2.y)) || ((gridVertices.last().x < v2.x) && (gridVertices.last().y < v2.y)))
					{
						vt = Vector(v2.x, gridVertices.last().y);
					}
					else //if (((gridVertices.last().x < v2.x) && (gridVertices.last().y > v2.y)) || ((gridVertices.last().x > v2.x) && (gridVertices.last().y < v2.y)))
					{
						vt = Vector(gridVertices.last().x, v2.y);
					}

					fillingVertices.add(gridVertices.last());
					fillingVertices.add(v2);
					fillingVertices.add(vt);
					gridVertices.add(vt);
				}

				gridVertices.add(v2);
			}
		
		}
		
		for (int i = 0; i < gridVertices.size() - 1;)
		{
			if ((gridVertices[i] - gridVertices[i + 1]).Length() < 0.0000001)
			{
				gridVertices.remove(i + 1);
			}
			else
			{
				i++;
			}
		}

		for (int i = 0; i < gridVertices.size() - 2;)
		{
			if ((gridVertices[i] - gridVertices[i + 2]).Length() < 0.00001)
			{
				gridVertices.remove(i + 1);
				gridVertices.remove(i + 1);
			}
			else
			{
				i++;
			}
		}
		
		for (int i = 0; i < gridVertices.size() - 2;)
		{
			if ((gridVertices[i] - gridVertices[i + 2]).Length() < 0.00001)
			{
				gridVertices.remove(i + 1);
				gridVertices.remove(i + 1);
			}
			else
			{
				i++;
			}
		}

		for (int i = 0; i < gridVertices.size() - 1; i++)
		{
			double dx = gridVertices[i + 1].x - gridVertices[i].x;
			double dy = gridVertices[i + 1].y - gridVertices[i].y;
			double sx = (dx / abs(dx));
			double sy = (dy / abs(dy));

			if ((abs(dx) < 0.00001) && (abs(dy) > gridSize))
			{
				gridVertices.insert(i + 1, Vector(gridVertices[i].x, gridVertices[i].y + (sy * gridSize)));
			}
			else if ((abs(dx) > gridSize) && (abs(dy) < 0.00001))
			{
				gridVertices.insert(i + 1, Vector(gridVertices[i].x + (sx * gridSize), gridVertices[i].y));
			}
		}

		int gridX = 2 * invGridSize;
		int gridY = 3 * invGridSize;

		char** grid = new char*[gridX];

		for (int i = 0; i < gridX; i++)
		{
			grid[i] = new char[gridY];

			for (int j = 0; j < gridY; j++)
			{
				grid[i][j] = 0;
			}
		}

		// create floodfill edge in grid
		for (int i = 0; i < gridVertices.size() - 1; i++)
		{
			int x, y;

			if (gridVertices[i].x < gridVertices[i + 1].x) // top edge ->
			{
				x = floor(gridVertices[i].x * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
				y = floor(gridVertices[i].y * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
			}
			else if (gridVertices[i].x > gridVertices[i + 1].x) // bottom edge <-
			{
				x = floor(gridVertices[i].x * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
				y = floor(gridVertices[i].y * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
			}
			else if (gridVertices[i].y < gridVertices[i + 1].y) // left edge ^
			{
				x = floor(gridVertices[i].x * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
				y = floor(gridVertices[i].y * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
			}
			else // if (gridVertices[i].y > gridVertices[i + 1].y) // right edge ¡
			{
				x = floor(gridVertices[i].x * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
				y = floor(gridVertices[i].y * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
			}

			if (((x >= 0) && (x < gridX)) && ((y >= 0) && (y < gridY)))
			{
				grid[x][y] = 1;
			}
		}

		// get an inner starting point
		int start_x, start_y;
		
		if (gridVertices[0].x < gridVertices[1].x) // top edge ->
		{
			start_x = floor(gridVertices[0].x * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
			start_y = floor(gridVertices[0].y * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
		}
		else if (gridVertices[0].x > gridVertices[1].x) // bottom edge <-
		{
			start_x = floor(gridVertices[0].x * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
			start_y = floor(gridVertices[0].y * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
		}
		else if (gridVertices[0].y < gridVertices[1].y) // left edge ^
		{
			start_x = floor(gridVertices[0].x * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
			start_y = floor(gridVertices[0].y * invGridSize + 0.5);	// should be around .00, but if .99 the +0.5 makes it right
		}
		else // if (gridVertices[0].y > gridVertices[1].y) // right edge ¡
		{
			start_x = floor(gridVertices[0].x * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
			start_y = floor(gridVertices[0].y * invGridSize + 0.5) - 1;	// should be around .00, but if .99 the +0.5 makes it right
		}
		
		// floodfill grid
		start_x = 20;
		start_y = 31;

		floodfill(grid, gridX, gridY, start_x, start_y, gridSize, fillingVertices);
		
		for (int i = 0; i < gridX; i++)
		{
			delete grid[i];
		}
		delete grid;
	}

	void floodfill(char** grid, int gx, int gy, int x, int y, double gridSize, VectorList& list)
	{
		if ((x >= 0) && (x < gx) && (y >= 0) && (y < gy) && grid[x][y] == 0)
		{
			grid[x][y] = 1;

			list.add(Vector(gridSize * x, gridSize * y));
			list.add(Vector(gridSize * (x + 1), gridSize * y));
			list.add(Vector(gridSize * (x + 1), gridSize * (y + 1)));
			list.add(Vector(gridSize * x, gridSize * y));
			list.add(Vector(gridSize * (x + 1), gridSize * (y + 1)));
			list.add(Vector(gridSize * x, gridSize * (y + 1)));

			floodfill(grid, gx, gy, x + 1, y, gridSize, list);
			floodfill(grid, gx, gy, x - 1, y, gridSize, list);
			floodfill(grid, gx, gy, x, y + 1, gridSize, list);
			floodfill(grid, gx, gy, x, y - 1, gridSize, list);
		}
	}

	virtual void getCurve(VectorList& vertices) = 0;
public:
	void Draw()
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(translation.x, translation.y, 0.f);

		VectorList curveVertices(400);
		getCurve(curveVertices);

		VectorList fillingVertices(5000);
		getFilling(curveVertices, 0.045, fillingVertices);
		
		
		// Draw filling
		glBegin(GL_TRIANGLES);
		Color c;
		for (int i = 0; i < fillingVertices.size(); i++)
		{
			c = GetColorAtPoint(fillingVertices[i]);
			glColor3d(c.r, c.g, c.b);
			glVertex2d(fillingVertices[i].x, fillingVertices[i].y);
		}
		glEnd();

		
		// Draw outline
		glBegin(GL_LINE_STRIP);
		glColor3d(1.f, 1.f, 1.f);
		// curve vertices
		for (int i = 0; i <= curveVertices.size(); i++)
		{
			glVertex2d(curveVertices[i%curveVertices.size()].x, curveVertices[i%curveVertices.size()].y);
		}
		glEnd();
		
		DrawBox();

		glPopMatrix();
	}

	virtual void Jump()
	{
		if (!is_jumping)
		{
			translation.y += 1.f;
			control_points = jumping_control_points;
			is_jumping = true;
		}
	}
	
	virtual void Fall()
	{
		if (is_jumping)
		{
			translation.y -= 1.f;
			control_points = normal_control_points;
			is_jumping = false;
		}
	}

	bool isPointInside(Vector v)
	{
		v -= translation;

		if (v.x >= 0.f &&
			v.x <= 2.f &&
			v.y >= 0.f &&
			v.y <= 3.f)
		{
			VectorList curveVertices;
			getCurve(curveVertices);
			VectorList fillingVertices;
			getFilling(curveVertices, 0.045, fillingVertices);

			for (int i = 0; i < fillingVertices.size(); i += 3)
			{
				if (isPointInTriangle(fillingVertices[i], fillingVertices[i+1], fillingVertices[i+2], v))
				{
					return true;
				}
			}

			return false;
		}
		else
		{
			return false;
		}
	}
};

class Benedike : public Balerina
{
private:
	double basis(int i, double div)
	{
		double binomial = 1;
		
		for(int j = 1; j <= i; j++)
		{
			binomial *= (double)(NUM_OF_CTRL_POINTS - j + 1) / j;
		}
		
		return binomial * pow(div, i) * pow(1 - div, NUM_OF_CTRL_POINTS - i);
	}

	Vector Eval(double div)
	{
		Vector point = Vector();

		for (int i = 0; i <= NUM_OF_CTRL_POINTS; i++)
		{
			point += control_points[i % NUM_OF_CTRL_POINTS] * basis(i, div);
		}

		return point;
	}

	void getCurve(VectorList& vertices)
	{
		for (int i = 0; i < 200; i++)
		{
			vertices.add(Eval(0.005 * i));
		}
	}

public:
	Benedike()
	{
		translation = Vector(1.f, 2.5f);
		is_jumping = false;
		control_points = normal_control_points;
		setFillDirection(Vector(0.f, 1.f));
	}
} benedike;

class Cezarina : public Balerina
{
private:
	Vector Eval(double t, int i)
	{
		double t2, t3;

		t2 = t * t;
		t3 = t2 * t;

		return (control_points[i % NUM_OF_CTRL_POINTS])*(t2 - 0.5f*(t + t3)) + (control_points[(i + 1) % NUM_OF_CTRL_POINTS])*(1.5f*t3 - 2.5f*t2 + 1.f) + (control_points[(i + 2) % NUM_OF_CTRL_POINTS])*(0.5f*t + 2.f*t2 - 1.5f*t3) + (control_points[(i + 3) % NUM_OF_CTRL_POINTS])*(0.5f*(t3 - t2));
	}

	void getCurve(VectorList& vertices)
	{
		for (int i = 0; i < NUM_OF_CTRL_POINTS; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				vertices.add(Eval(j * 0.1, i));
			}
		}
	}

public:
	Cezarina()
	{
		translation = Vector(4.f, 2.5f);
		is_jumping = false;
		control_points = normal_control_points;
		setFillDirection(Vector(1.f, 1.f));
	}
} cezarina;

class Nikodemia : public Balerina
{
private:
	double tknot(int i)
	{
		if (i < 3)
		{
			return 0;
		}
		else if (i > NUM_OF_CTRL_POINTS)
		{
			return NUM_OF_CTRL_POINTS - 1;
		}
		else
		{
			return i - 2;
		}
	}

	double basis(int i, int k, double t)
	{
    	if (k == 1)
		{ 
			if (tknot(i) <= t && t < tknot(i+1))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			double b1 = basis(i, k-1, t);
			double dt = (tknot(i+k-1) - tknot(i));
		
			if (dt < 0.00001)
			{
				b1 = 0;
			}
			else
			{
				b1 /= dt;
			}

			double b2 = basis(i+1, k-1, t);
			dt = (tknot(i+k) - tknot(i+1));
			
			if (dt < 0.000001)
			{
				b2 = 0;
			}
 			else
			{
				b2 /= dt;
			}

 			return (t-tknot(i)) * b1 + (tknot(i+k)-t) * b2;
		}
	}

	Vector Eval(double t)
	{
		Vector rt = Vector();

		for(int i = 0; i <= NUM_OF_CTRL_POINTS; i++)
		{
			rt += control_points[i%NUM_OF_CTRL_POINTS] * basis(i, 3, t);
		}

		return rt;
	}

	void getCurve(VectorList& vertices)
	{
		for (int i = 0; i < (NUM_OF_CTRL_POINTS - 1)*10; i++)
		{
			vertices.add(Eval(i * 0.1));
		}
	}

public:
	Nikodemia()
	{
		translation = Vector(7.f, 2.5f);
		is_jumping = false;
		control_points = normal_control_points;
		setFillDirection(Vector(1.f, 3.f));
	}

} nikodemia;

struct Camera
{
	int current_balerina;
	int zoom_level;
	double zoom;
	Vector focus;
	
	Camera()
	{
		zoom = 0.2; // 0.2
		zoom_level = 0;
		current_balerina = 1; // 1
		focus = Vector(5.0, 5.0); // 5.0, 5.0
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glPushMatrix();
		glScaled(zoom, zoom, 1.f);
		glTranslated(-focus.x, -focus.y, 0.f);
	}

	void ZoomIn()
	{
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPushMatrix();
		current_balerina = (current_balerina + 1) % 3;
		zoom_level++;
		switch (current_balerina)
		{
			case 0:
				focus = Vector(2.0, 5.0);
				break;
			case 1:
				focus = Vector(5.0, 5.0);
				break;
			case 2:
				focus = Vector(8.0, 5.0);
				break;
		}
		zoom = 0.2f * pow(1.2f, zoom_level);
		glScaled(zoom, zoom, 1.f);
		glTranslated(-focus.x, -focus.y, 0.f);
	}

} camera;

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization( )
{
	benedike = Benedike();
	cezarina = Cezarina();
	nikodemia = Nikodemia();
	camera = Camera();
	glViewport(0, 0, 600, 600);
}

// Rajzolas, minden frame megjelenitesehez ez a fuggveny hivodik meg
void onDisplay( )
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);		// torlesi szin beallitasa
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

	benedike.Draw();
	cezarina.Draw();
	nikodemia.Draw();
    // ...

    glutSwapBuffers();     				// Buffercsere: rajzolas vege

}

// Billentyuzet esemenyeket lekezelo fuggveny
void onKeyboard(unsigned char key, int x, int y)
{
    if (key == 'b')
	{
		benedike.Jump();
	}
	else if (key == 'c')
	{
		cezarina.Jump();
	}
	else if (key == 'n')
	{
		nikodemia.Jump();
	}
	else if (key == ' ')
	{
		camera.ZoomIn();
	}

	glutPostRedisplay();
}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT && state == GLUT_DOWN)
	{
		Vector m;

		// invert y
		y = 600 - y;

		// screen->homogenious: h = s / 600 * 2 - 1
		// homogeniuos->view: v = h / z + t
		m.x = (x / 300.f - 1.f) / camera.zoom + camera.focus.x;
		m.y = (y / 300.f - 1.f) / camera.zoom + camera.focus.y;

		if (benedike.isPointInside(m))
		{
			benedike.Fall();
		}
		else if (cezarina.isPointInside(m))
		{
			cezarina.Fall();
		}
		else if (nikodemia.isPointInside(m))
		{
			nikodemia.Fall();
		}
	}

	glutPostRedisplay();
}

// `Idle' esemenykezelo, jelzi, hogy az ido telik
void onIdle( )
{
     long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido
}

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char **argv) {
    glutInit(&argc, argv); 				// GLUT inicializalasa
    glutInitWindowSize(600, 600);			// Windows ablak kezdeti meret 600x600 pixel 
    glutInitWindowPosition(100, 100);			// Az elozo Windows ablakhoz kepest hol tunik fel
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	// 8 bites R,G,B,A + dupla buffer + melyseg buffer

    glutCreateWindow("Grafika hazi feladat");		// Windows ablak megszuletik es megjelenik a kepernyon

    glMatrixMode(GL_MODELVIEW);				// A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);			// A PROJECTION transzformaciot egysegmatrixra inicializaljuk
    glLoadIdentity();

    onInitialization();					// Az altalad irt inicializalast lefuttatjuk

    glutDisplayFunc(onDisplay);				// Esemenykezelok regisztralasa
    glutMouseFunc(onMouse); 
    glutIdleFunc(onIdle);
    glutKeyboardFunc(onKeyboard);

    glutMainLoop();					// Esemenykezelo hurok
    
    return 0;
}

