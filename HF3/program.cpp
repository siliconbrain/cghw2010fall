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

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 700;
const int PHOTON_MAP_RADIUS = 1000;
const int PHOTON_MAP_SECTIONS = 3000;
const int PHOTON_COUNT = 50000000;
const float TWO_PI = 6.283185;
const float PI = 3.14159265358979323846;
const float EPSILON = 1e-4;
const int MAX_RECURSION_DEPTH = 6;
const float SCENE_RADIUS = 4;
long last_render_time = 0;

struct Color
{
	float r;
	float g;
	float b;

	Color(float red = 0.0, float green = 0.0, float blue = 0.0)
	{
		r = red;
		g = green;
		b = blue;
	}

	Color& operator+=(const Color& rhs)
	{
		r += rhs.r;
		g += rhs.g;
		b += rhs.b;
		return *this;
	}

	const Color operator+(const Color& rhs) const
	{
		return (Color(*this) += rhs);
	}

	Color& operator+=(const float scalar)
	{
		r += scalar;
		g += scalar;
		b += scalar;
		return *this;
	}

	const Color operator+(const float scalar) const
	{
		return (Color(*this) += scalar);
	}

	Color& operator-=(const float scalar)
	{
		r -= scalar;
		g -= scalar;
		b -= scalar;
		return *this;
	}

	const Color operator-(const float scalar) const
	{
		return (Color(*this) -= scalar);
	}

	Color& operator*=(const Color& rhs)
	{
		r *= rhs.r;
		g *= rhs.g;
		b *= rhs.b;
		return *this;
	}

	const Color operator*(const Color& rhs) const
	{
		return (Color(*this) *= rhs);
	}

	Color& operator*=(const float scalar)
	{
		r *= scalar;
		g *= scalar;
		b *= scalar;
		return *this;
	}

	const Color operator*(const float scalar) const
	{
		return (Color(*this) *= scalar);
	}

	Color& operator/=(const float scalar)
	{
		r /= scalar;
		g /= scalar;
		b /= scalar;
		return *this;
	}

	const Color operator/(const float scalar) const
	{
		return (Color(*this) /= scalar);
	}

	const Color operator-() const
	{
		return (Color(*this) *= -1);
	}

	const Color Inv() const
	{
		return Color(1 / r, 1 / g, 1 / b);
	}

	const Color Complement() const
	{
		return Color(1 - r, 1 - g, 1 - b);
	}
} pixels[WINDOW_WIDTH * WINDOW_HEIGHT], photon_map[PHOTON_MAP_RADIUS][PHOTON_MAP_SECTIONS];

struct Vector
{
	float x;
	float y;
	float z;

	Vector(float x = 0.0, float y = 0.0, float z = 0.0)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector& operator+=(const Vector& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	const Vector operator+(const Vector& other) const
	{
		Vector v = *this;
		v += other;
		return v;
	}

	Vector& operator*=(const float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	const Vector operator*(const float scalar) const
	{
		Vector v = *this;
		v *= scalar;
		return v;
	}

	Vector& operator-=(const Vector& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	const Vector operator-(const Vector& other) const
	{
		Vector v = *this;
		v -= other;
		return v;
	}

	Vector& operator/=(const float scalar)
	{
		float rec = 1 / scalar;
		(*this) *= rec;
		return *this;
	}

	const Vector operator/(const float scalar) const
	{
		Vector v = *this;
		v /= scalar;
		return v;
	}

	float Dot(const Vector& other) const
	{
		return (x * other.x) + (y * other.y) + (z * other.z);
	}

	Vector Cross(const Vector& other) const
	{
		Vector v;
		v.x = (y * other.z) - (z * other.y);
		v.y = (z * other.x) - (x * other.z);
		v.z = (x * other.y) - (y * other.x);
		return v;
	}

	float LengthSquared() const
	{
		return (x * x) + (y * y) + (z * z);
	}

	float Length() const
	{
		return sqrt(LengthSquared());
	}

	void Normalize()
	{
		(*this) /= Length();
	}

	const Vector operator-() const
	{
		return (Vector(*this) *= -1);
	}
};

class Ray
{
private:
	Vector source;
	Vector direction;

public:
	Vector getDirection() const
	{
		return direction;
	}

	void setDirection(Vector v)
	{
		direction = v;
		direction.Normalize();
	}

	void unsafe_setDirection(Vector v)
	{
		direction = v;
	}

	Vector getSource() const
	{
		return source;
	}

	void setSource(Vector v)
	{
		source = v;
	}

	Ray(const Vector& source, const Vector& direction)
	{
		this->source = source;
		this->direction = direction;
		this->direction.Normalize();
	}

	Vector pointFromDistance(float distance) const
	{
		return (source + (direction * distance));
	}

	void InverseTransform(Vector objectTranslation)
	{
		source -= objectTranslation;
	}

	static Ray FromBeginEnd(const Vector& begin, const Vector& end)
	{
		return Ray(begin, end - begin);
	}
};

struct Material
{
	Color n;
	Color k;
	Color dc;
	bool isDiffuse;

	Material(Color refractiveIndex, Color extinctionCoefficient)
	{
		n = refractiveIndex;
		k = extinctionCoefficient;
		isDiffuse = false;
	}

	Material(Color diffuseColor = Color())
	{
		dc = diffuseColor;
		isDiffuse = true;
	}

}	glass(Color(1.5, 1.5, 1.5), Color(0,0,0)),
	gold(Color(0.17, 0.35, 1.5), Color(3.1, 2.7, 1.9)),
	silver(Color(0.14, 0.13, 0.16), Color(4.1, 3.1, 2.3)),
	copper(Color(0.2, 1.1, 1.2), Color(3.6, 2.6, 2.3));

class Object
{
protected:
	Vector translation;
	Material material;

public:
	Material getMaterial()
	{
		return material;
	}

	virtual bool Intersect(Ray ray, float* distance, Vector* normal) = 0;
} *scene_objects[5];

class Cylinder : public Object
{
private:
	float height;
	float radius;

public:
	Cylinder()
	{
		translation = Vector(0,0,0);
		material = gold;

		height = 3;
		radius = SCENE_RADIUS;
	}

	bool Intersect(Ray ray, float* distance, Vector* normal)
	{
		ray.InverseTransform(translation);
		
		Vector source = ray.getSource();
		Vector direction = ray.getDirection();
		float a = (direction.x * direction.x) + (direction.z * direction.z);

		if ((a < EPSILON) && (a > -EPSILON))
		{
			return false;
		}

		float b = 2 * ((source.x * direction.x) + (source.z * direction.z));
		float c = (source.x * source.x) + (source.z * source.z) - (radius * radius);

		float partSol = (b * b) - (4 * a * c);

		if (partSol > 0)
		{
			float root1 = (-b + sqrt(partSol)) / (2 * a);
			float root2 = (-b - sqrt(partSol)) / (2 * a);

			if (root1 > root2)
			{
				float temp = root1;
				root1 = root2;
				root2 = temp;
			}

			if (root1 > 0)
			{
				Vector intr = source + (direction * root1);

				if ((intr.y > 0) && (intr.y < height))
				{
					*distance = root1;
					*normal = source + (direction * (*distance));

					if (normal->Dot(direction) > 0)
					{
						*normal = -(*normal);
					}

					normal->Normalize();
					return true;
				}
			}
			
			if (root2 > 0)
			{
				Vector intr = source + (direction * root2);

				if ((intr.y > 0) && (intr.y < height))
				{
					*distance = root2;
					*normal = source + (direction * (*distance));
					
					if (normal->Dot(direction) > 0)
					{
						*normal = -(*normal);
					}

					normal->Normalize();
					return true;
				}
			}
		}
		
		return false;
	}
} wall;

class Disc : public Object
{
private:
	float radius;

public:
	Disc()
	{
		translation = Vector(0,0,0);
		material = Material(Color(0.4, 0.2, 0));

		radius = SCENE_RADIUS;
	}

	bool Intersect(Ray ray, float* distance, Vector* normal)
	{
		ray.InverseTransform(translation);
		
		Vector source = ray.getSource();
		Vector direction = ray.getDirection();

		if (direction.y == 0)
		{
			return false;
		}

		float dist = -source.y / direction.y;

		if (dist > 0)
		{
			Vector intr = source + (direction * dist);

			if ((intr.x * intr.x) + (intr.z * intr.z) - (radius * radius) <= 0)
			{
				*distance = dist;
				*normal = Vector(0,1,0);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
} scene_floor;

bool IntersectTriangle(const Vector& v0, const Vector& v1, const Vector& v2, const Ray& ray, float* const distance)
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;

	Vector dir = ray.getDirection();

	Vector p = dir.Cross(v);

	float det = u.Dot(p);

	if (det == 0)
	{
		return false;
	}

	float inv_det = 1 / det;

	Vector t = ray.getSource() - v0;

	float ut = t.Dot(p) * inv_det;

	if ((ut < -EPSILON) || (ut > (1 + EPSILON)))
	{
		return false;
	}

	Vector q = t.Cross(u);

	float vt = dir.Dot(q) * inv_det;

	if ((vt < -EPSILON) || (ut + vt > (1 + EPSILON)))
	{
		return false;
	}

	*distance = v.Dot(q) * inv_det;
	return true;
}

bool IntersectBoundingSphere(const float radius, const Ray& ray)
{
	Vector src = ray.getSource();
	Vector dir = ray.getDirection();

	float a = dir.LengthSquared();
	float b = 2 * src.Dot(dir);
	float c = src.LengthSquared() - (radius * radius);

	float partSol = (b * b) - (4 * a * c);

	if (partSol < 0)
	{
		return false;
	}
	else
	{
		float inv2a = 1 / (2 * a);
		float sqrtdps = sqrt(partSol);
		float root = (-b - sqrtdps) * inv2a;

		if (root < 0)
		{
			root = (-b + sqrtdps) * inv2a;

			if (root < 0)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
}

class Octahedron : public Object
{
private:
	const static int num_of_vertices = 6;
	const static int num_of_faces = 8;
	Vector vertices[num_of_vertices];
	int indices[num_of_faces * 3];

public:
	Octahedron()
	{
		translation = Vector(-2,0.1,0);
		material = copper;

		vertices[0] = Vector(0,0,0);
		vertices[1] = Vector(0.5,1,0);
		vertices[2] = Vector(0,1,-0.5);
		vertices[3] = Vector(-0.5,1,0);
		vertices[4] = Vector(0,1,0.5);
		vertices[5] = Vector(0,1.8,0);

		indices[0] = 0;	// face 0
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 0;	// face 1
		indices[4] = 2;
		indices[5] = 3;
		indices[6] = 0;	// face 2
		indices[7] = 3;
		indices[8] = 4;
		indices[9] = 0;	// face 3
		indices[10] = 4;
		indices[11] = 1;
		indices[12] = 5;	// face 4
		indices[13] = 2;
		indices[14] = 1;
		indices[15] = 5;	// face 5
		indices[16] = 3;
		indices[17] = 2;
		indices[18] = 5;	// face 6
		indices[19] = 4;
		indices[20] = 3;
		indices[21] = 5;	// face 7
		indices[22] = 1;
		indices[23] = 4;
	}

	bool Intersect(Ray ray, float* distance, Vector* normal)
	{
		ray.InverseTransform(translation);

		float minDist = 10000;
		int closestTriangleIndex = -1;

		for (int i = 0; i < num_of_faces; i++)
		{
			float dist;
			// get intersection with triangle:
			if (IntersectTriangle(vertices[indices[i * 3]], vertices[indices[i * 3 + 1]], vertices[indices[i * 3 + 2]], ray, &dist))
			{
				if ((dist > 0) && (dist < minDist))
				{
					minDist = dist;
					closestTriangleIndex = i;
				}
			}
		}

		if (closestTriangleIndex >= 0)
		{
			*distance = minDist;
			Vector* v0 = vertices + indices[closestTriangleIndex * 3];
			Vector* v1 = vertices + indices[closestTriangleIndex * 3 + 1];
			Vector* v2 = vertices + indices[closestTriangleIndex * 3 + 2];
			*normal = ((*v1) - (*v0)).Cross((*v2) - (*v0));

			if (normal->Dot(ray.getDirection()) > 0)
			{
				*normal = -(*normal);
			}

			normal->Normalize();
			return true;
		}
		else
		{
			return false;
		}
	}
} polyhedron;

class HeightField : public Object
{
private:
	static const int resolution = 20;
	static const int num_of_vertices = (resolution + 1) * (resolution + 1);
	static const int num_of_indices = resolution * resolution * 2 * 3;
	Vector vertices[num_of_vertices];
	int indices[num_of_indices];

	float bounding_sphere_radius;

public:
	HeightField()
	{
		translation = Vector(2,0.6,0);
		material = silver;

		bounding_sphere_radius = 0;

		// generate mesh
		float step = 1.0 / resolution;

		for (int i = 0; i < resolution + 1; i++)
		{
			for (int j = 0; j < resolution + 1; j++)
			{
				float x = (j - (resolution * 0.5)) * step;
				float y = (i - (resolution * 0.5)) * step;
				vertices[(i * (resolution + 1)) + j] = Vector(x, Eval(x, y), y);

				float length;
				if ((length = vertices[(i * (resolution + 1)) + j].Length()) > bounding_sphere_radius)
				{
					bounding_sphere_radius = length;
				}
			}
		}

		for (int i = 0; i < resolution; i++)
		{
			for (int j = 0; j < resolution; j++)
			{
				indices[(((i * resolution) + j) * 6) + 0] = (i * (resolution + 1)) + j;
				indices[(((i * resolution) + j) * 6) + 1] = (i * (resolution + 1)) + j + 1;
				indices[(((i * resolution) + j) * 6) + 2] = ((i + 1) * (resolution + 1)) + j + 1;
				indices[(((i * resolution) + j) * 6) + 3] = (i * (resolution + 1)) + j;
				indices[(((i * resolution) + j) * 6) + 4] = ((i + 1) * (resolution + 1)) + j + 1;
				indices[(((i * resolution) + j) * 6) + 5] = ((i + 1) * (resolution + 1)) + j;
			}
		}
	}

	float Eval(float u, float v)
	{
		return (u * u) - (2 * v * v);
	}

	bool Intersect(Ray ray, float* distance, Vector* normal)
	{
		ray.InverseTransform(translation);

		if (IntersectBoundingSphere(bounding_sphere_radius, ray))
		{
			float minDist = 10000, dist;
			int closestTriangleIndex = -1;

			for (int i = 0; i < num_of_indices; i += 3)
			{
				if (IntersectTriangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]], ray, &dist) && (dist > 0) && (dist < minDist))
				{
					minDist = dist;
					closestTriangleIndex = i;
				}
			}

			if (closestTriangleIndex < 0)
			{
				return false;
			}
			else
			{
				*distance = dist;
				Vector* v0 = vertices + indices[closestTriangleIndex];
				Vector* v1 = vertices + indices[closestTriangleIndex + 1];
				Vector* v2 = vertices + indices[closestTriangleIndex + 2];
				*normal = ((*v1) - (*v0)).Cross((*v2) - (*v0));

				if (normal->Dot(ray.getDirection()) > 0)
				{
					*normal = -(*normal);
				}

				normal->Normalize();
				return true;
			}
		}
	}
} parametricSurface;

class Ellipsoid : public Object
{
private:
	float radius_x, radius_y, radius_z;

public:
	Ellipsoid()
	{
		translation = Vector(0,1,0);
		material = glass;

		radius_x = 0.5;
		radius_y = 0.75;
		radius_z = 0.5;
	}

	bool Intersect(Ray ray, float* distance, Vector* normal)
	{
		ray.InverseTransform(translation);

		Vector src = ray.getSource();
		Vector dir = ray.getDirection();

		float rx2 = radius_x * radius_x;
		float ry2 = radius_y * radius_y;
		float rz2 = radius_z * radius_z;

		float rx2ry2 = rx2 * ry2;
		float ry2rz2 = ry2 * rz2;
		float rz2rx2 = rz2 * rx2;

		float a = (dir.x * dir.x * ry2rz2) + (dir.y * dir.y * rz2rx2) + (dir.z * dir.z * rx2ry2);
		float b = 2 * ((src.x * dir.x * ry2rz2) + (src.y * dir.y * rz2rx2) + (src.z * dir.z * rx2ry2));
		float c = (src.x * src.x * ry2rz2) + (src.y * src.y * rz2rx2) + (src.z * src.z * rx2ry2) - (rx2ry2 * rz2);

		float partSol = (b * b) - (4 * a * c);

		if (partSol < 0)
		{
			return false;
		}
		else
		{
			float inv2a = 1 / (2 * a);
			float sqrtdps = sqrt(partSol);
			float root = (-b - sqrtdps) * inv2a;

			if (root < 0)
			{
				root = (-b + sqrtdps) * inv2a;

				if (root < 0)
				{
					return false;
				}
				else
				{
					*distance = root;
					Vector point = src + (dir * (*distance));
					*normal = Vector(point.x / rx2, point.y / ry2, point.z / rz2);

					if (normal->Dot(dir) > 0)
					{
						*normal = -(*normal);
					}

					normal->Normalize();
					return true;
				}
			}
			else
			{
				*distance = root;
				Vector point = src + (dir * (*distance));
				*normal = Vector(point.x / rx2, point.y / ry2, point.z / rz2);

				if (normal->Dot(dir) > 0)
				{
					*normal = -(*normal);
				}

				normal->Normalize();
				return true;
			}
		}
	}
} implicitSurface;

class LightSource
{
private:
	Vector base;
	Color col;

	float MOVE_MAX;

	Vector move_dir;
	float move_size;
	float speed;

public:
	Vector getPosition()
	{
		return base + move_dir * move_size;
	}

	Color getColor()
	{
		return col;
	}

	LightSource()
	{
		MOVE_MAX = 2 * (SCENE_RADIUS - EPSILON);
		base = Vector(-(MOVE_MAX / 2),3,0);
		move_dir = Vector(1,0,0);
		move_size = 4.0;
		col = Color(1,1,1);
		speed = MOVE_MAX / 10;
	}

	void Update(float elapsedSeconds)
	{
		move_size += speed * elapsedSeconds;

		while ((move_size > MOVE_MAX) || (move_size < 0))
		{
			if (move_size > MOVE_MAX)
			{
				move_size = 2 * MOVE_MAX - move_size;
				speed = -speed;
			}
			else
			{
				move_size = -move_size;
				speed = -speed;
			}
		}
	}
} lamp;

class Camera
{
private:
	Vector position;
	Vector look_dir;
	Vector up;

	float lens_distance;
	float rot;
public:
	Vector getPosition()
	{
		return position;
	}

	void setPosition(Vector position)
	{
		this->position = position;
	}

	void setLookAt(Vector lookAt)
	{
		look_dir = lookAt - position;
		look_dir.Normalize();
		Vector right = look_dir.Cross(up);
		right.Normalize();
		up = right.Cross(look_dir);
	}

	Camera()
	{
		rot = 0;
		position = Vector(0,1,-3.8);
		up = Vector(0,1,0);
		setLookAt(Vector(0,0,1));
		lens_distance = (WINDOW_WIDTH * 0.5);
	}

	Vector getGridDirection(int x, int y)
	{
		Vector right = look_dir.Cross(up);
		float _x = x - (WINDOW_WIDTH * 0.5);
		float _y = y - (WINDOW_HEIGHT * 0.5);
		
		return (right * _x) + (up * _y) + (look_dir * lens_distance);
	}

	void Rotatate(float theta)
	{
		rot += theta;
		look_dir.x = sin(rot);
		look_dir.z = cos(rot);
		look_dir.Normalize();
	}
} camera;

// Lazanyi - Schlick Fresnel: ((n-1)^2 + 4n(1-cos(theta))^5 + k^2) / ((n+1)^2 + k^2)
float Fresnel(float n, float k, float cosTheta)
{
	float k2 = k * k;
	return (((n - 1) * (n - 1)) + (4 * n * pow(1 - cosTheta, 5)) + k2) / (((n + 1) * (n + 1)) + k2);
}

bool FindIntersection(const Ray& ray, float maxDistance)
{
	float d;
	Vector n;

	for (int i = 0; i < 5; i++)
	{
		if (scene_objects[i]->Intersect(Ray(ray), &d, &n))
		{
			if (d < maxDistance)
			{
				return true;
			}
		}
	}

	return false;
}

bool FindIntersection(const Ray& ray, Vector* hitPoint, Vector* hitPointNormal, Object** hitObject)
{
	bool wasHit = false;
	Vector normal;
	float hitPointDist = 10000, distance;
	// intersect each object
	for (int i = 0; i < 5; i++)
	{
		if (scene_objects[i]->Intersect(Ray(ray), &distance, &normal))
		{
			if (distance < hitPointDist)
			{
				*hitObject = scene_objects[i];
				*hitPointNormal = normal;
				hitPointDist = distance;
				wasHit = true;
			}
		}
	}

	if (wasHit)
	{
		// calc hit point
		*hitPoint = ray.getSource() + (ray.getDirection() * hitPointDist);
		return true;
	}
	else
	{
		return false;
	}
}

Vector ReflectVector(Vector incidentDir, Vector normal)
{
	return incidentDir - (normal * 2 * normal.Dot(incidentDir));
}

void getPhotonMapIndex(float u, float v, int* radius, int* sector)
{
	// radial coord
	float r = sqrt(u * u + v * v);
	float a;

	if (r < EPSILON)
	{
		a = 0;
	}
	else
	{
		a = atan2(v, u);
	}
	
	*radius = r / SCENE_RADIUS * PHOTON_MAP_RADIUS;
	*sector = (a + PI) / TWO_PI * PHOTON_MAP_SECTIONS;

	if (*radius > (PHOTON_MAP_RADIUS - 1))
	{
		*radius = (PHOTON_MAP_RADIUS - 1);
	}

	if (*sector > (PHOTON_MAP_SECTIONS - 1))
	{
		*sector = (PHOTON_MAP_SECTIONS - 1);
	}
}

Color TraceRay(Ray ray, int depth, bool inside)
{
	Color returnColor(0,0,0);

	if (MAX_RECURSION_DEPTH > depth)
	{
		Vector hitPoint, hitPointNormal;
		Object* hitObject;

		if (FindIntersection(ray, &hitPoint, &hitPointNormal, &hitObject))
		{	
			Material hitObjectMaterial = hitObject->getMaterial();

			Vector incidentDir = ray.getDirection();
			float dotNI = hitPointNormal.Dot(incidentDir);
			float lightTravelDistSqrd = (hitPoint - ray.getSource()).LengthSquared();

			Vector lightDir = lamp.getPosition() - hitPoint;	// calc light direction, but don't normalize right away...
			float lightDistSqrd = lightDir.LengthSquared();		// first calc light distance squared for light fall-off
			float lightDist = sqrt(lightDistSqrd);				// calc light distance
			lightDir.Normalize();								// now normalize direction vector

			float dotNL = hitPointNormal.Dot(lightDir);	// Normal.Light

			// Lighting:
			Color lightColor(0,0,0);	// set to black, as if the hit point was shadowed

			if (dotNL > 0)	// if we are faceing in the same direction as the light
			{
				float hitDistance;
				// hit point is moved a bit along the light direction to ensure no self-intersection because of floating point error
				if (!FindIntersection(Ray(hitPoint + (lightDir * EPSILON), lightDir), lightDist))
				{
					// if there were no object between the hit point and the light (not in a shadow),
					// set light color w/ accounting for fall-off
					lightColor = lamp.getColor() / (1 + lightDistSqrd);
				}
			}

			if (hitObjectMaterial.isDiffuse)
			{
				int sector, radius;
				getPhotonMapIndex(hitPoint.x, hitPoint.z, &radius, &sector);
				returnColor = (lightColor + photon_map[radius][sector]) * hitObjectMaterial.dc;
			}
			else
			{
				// = = = = = = = =
				// Specular:
				// = = = = = = = =
				Color specularColor(0,0,0);

				if (dotNL > 0)
				{
					Vector H = lightDir - incidentDir;
					H.Normalize();
					float dotNH = H.Dot(hitPointNormal);

					specularColor = Color(
						Fresnel(hitObjectMaterial.n.r, hitObjectMaterial.k.r, dotNL),
						Fresnel(hitObjectMaterial.n.g, hitObjectMaterial.k.g, dotNL),
						Fresnel(hitObjectMaterial.n.b, hitObjectMaterial.k.b, dotNL)
						) * lightColor * dotNH * dotNL;
				}

				// = = = = = = = =
				// Reflected Ray:
				// = = = = = = = =
				// Vrefl = Vinc - 2Vnorm(Vinc.Vnorm)
				Vector reflectionDir = incidentDir - (hitPointNormal * 2 * dotNI);
				
				float dotNR = hitPointNormal.Dot(reflectionDir);

				// hit point is moved a bit along the reflection direction to ensure no self-intersection because of floating point error
				Color reflectionColor = Color(
					Fresnel(hitObjectMaterial.n.r, hitObjectMaterial.k.r, dotNR),
					Fresnel(hitObjectMaterial.n.g, hitObjectMaterial.k.g, dotNR),
					Fresnel(hitObjectMaterial.n.b, hitObjectMaterial.k.b, dotNR)
					) * TraceRay(Ray(hitPoint + (reflectionDir * EPSILON), reflectionDir), depth + 1, inside);

				// = = = = = = = =
				// Refracted Ray:
				// = = = = = = = =
				Color refractionColor(0,0,0);	// set to black, as if the object had no refractions

				if ((hitObjectMaterial.k.r < 1) && (hitObjectMaterial.k.g < 1) && (hitObjectMaterial.k.b < 1))
				{
					// if the ray is going in: n_act = n / 1
					// if its coming out: n_act = 1 / n
					Color nCol = ((inside) ? hitObjectMaterial.n.Inv() : hitObjectMaterial.n);
					// get the average of the RGB refraction index
					float n = (nCol.r + nCol.g + nCol.b) / 3;

					float cos2NT = 1 - (1 - dotNI * dotNI) / (n * n);

					if (cos2NT > 0)	// if Normal.Refraction > 0 than its a refraction
					{
						float cosNT = sqrt(cos2NT);
						// ((-N.I * N + I) * n2 / n1) - (N * cosNT)
						Vector refractionDir = ((incidentDir + hitPointNormal * -dotNI) / n) - (hitPointNormal * cosNT);

						// hit point is moved a bit along the refraction direction to ensure no self-intersection because of floating point error
						refractionColor = Color(
							1 - Fresnel(hitObjectMaterial.n.r, hitObjectMaterial.k.r, cosNT),
							1 - Fresnel(hitObjectMaterial.n.g, hitObjectMaterial.k.g, cosNT),
							1 - Fresnel(hitObjectMaterial.n.b, hitObjectMaterial.k.b, cosNT)
							) * TraceRay(Ray(hitPoint + (refractionDir * EPSILON), refractionDir), depth + 1, !inside);
					}
					// else its a TIR (total internal reflection), and we've accounted for that with reflection
				}
				
				returnColor = specularColor + reflectionColor + refractionColor;
			}

			returnColor /= 1 + lightTravelDistSqrd;
		}
	}

	return returnColor;
}

void TracePhoton(const Ray& ray, Color cummulativeColor, int depth, bool inside)
{
	if (MAX_RECURSION_DEPTH > depth)
	{
		Vector hitPoint, hitPointNormal;
		Object* hitObject;

		if (FindIntersection(ray, &hitPoint, &hitPointNormal, &hitObject))
		{	
			Material hitObjectMaterial = hitObject->getMaterial();

			Vector incidentDir = ray.getDirection();
			float dotNI = hitPointNormal.Dot(incidentDir);
			cummulativeColor /= 1 + (hitPoint - ray.getSource()).LengthSquared();

			if (hitObject == &scene_floor)
			{
				if (depth > 0)
				{
					int radius, sector;
					getPhotonMapIndex(hitPoint.x, hitPoint.z, &radius, &sector);
					photon_map[radius][sector] += cummulativeColor;
				}
			}
			else
			{
				cummulativeColor *= Color(
					Fresnel(hitObjectMaterial.n.r, hitObjectMaterial.k.r, -dotNI),
					Fresnel(hitObjectMaterial.n.g, hitObjectMaterial.k.g, -dotNI),
					Fresnel(hitObjectMaterial.n.b, hitObjectMaterial.k.b, -dotNI)
					);

				// = = = = = = = =
				// Reflected Ray:
				// = = = = = = = =
				// Vrefl = Vinc - 2Vnorm(Vinc.Vnorm)
				Vector reflectionDir = incidentDir - (hitPointNormal * 2 * dotNI);
				
				// hit point is moved a bit along the reflection direction to ensure no self-intersection because of floating point error
				TracePhoton(Ray(hitPoint + (reflectionDir * EPSILON), reflectionDir), cummulativeColor, depth + 1, inside);

				// = = = = = = = =
				// Refracted Ray:
				// = = = = = = = =
				if ((hitObjectMaterial.k.r < 1) && (hitObjectMaterial.k.g < 1) && (hitObjectMaterial.k.b < 1))
				{
					// if the ray is going in: n_act = n / 1
					// if its coming out: n_act = 1 / n
					Color nCol = ((inside) ? hitObjectMaterial.n.Inv() : hitObjectMaterial.n);
					// get the average of the RGB refraction index
					float n = (nCol.r + nCol.g + nCol.b) / 3;

					float cos2NT = 1 - (1 - dotNI * dotNI) / (n * n);

					if (cos2NT > 0)	// if Normal.Refraction > 0 than its a refraction
					{
						float cosNT = sqrt(cos2NT);
						// ((-N.I * N + I) * n2 / n1) - (N * cosNT)
						Vector refractionDir = ((incidentDir + hitPointNormal * -dotNI) / n) - (hitPointNormal * cosNT);
		
						// hit point is moved a bit along the refraction direction to ensure no self-intersection because of floating point error
						TracePhoton(Ray(hitPoint + (refractionDir * EPSILON), refractionDir), cummulativeColor, depth + 1, !inside);
					}
					// else its a TIR (total internal reflection), and we've accounted for that with reflection
				}
			}
		}
	}
}

void RenderScene()
{
	// photon map
	for (int i = 0; i < PHOTON_MAP_RADIUS; i++)
	{
		for (int j = 0; j < PHOTON_MAP_SECTIONS; j++)
		{
			photon_map[i][j] = Color(0,0,0);
		}
	}

	for (int i = 0; i < PHOTON_COUNT; i++)
	{
		srand(i);
		Vector dir;
		float x;
		float y;
		float z;

		do
		{
			x = rand() / (float)RAND_MAX * 2 - 1;
			y = rand() / (float)RAND_MAX * 2 - 1;
			z = rand() / (float)RAND_MAX * 2 - 1;
			dir = Vector(x,y,z);
		}
		while(dir.Length() > 1 || y > 0);
		
		dir.Normalize();

		TracePhoton(Ray(lamp.getPosition(), dir), lamp.getColor(), 0, false);
	}

	for (int i = 0; i < PHOTON_MAP_RADIUS; i++)
	{
		for (int j = 0; j < PHOTON_MAP_SECTIONS; j++)
		{
			photon_map[i][j] /= (2 * (float)i + 1) * 0.005;
		}
	}

	// raytrace
	for (int v_scan = 0; v_scan < WINDOW_HEIGHT; v_scan++)
	{
		for (int h_scan = 0; h_scan < WINDOW_WIDTH; h_scan++)
		{
			pixels[(v_scan * WINDOW_WIDTH) + h_scan] = TraceRay(Ray(camera.getPosition(), camera.getGridDirection(h_scan, v_scan)), 0, false);
		}
	}

	// tonemap
	float Ymax = 0, alpha = 2;
	for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
	{
		float Y = 0.2126 * pixels[i].r + 0.7152 * pixels[i].g + 0.0722 * pixels[i].b;

		if (Y > Ymax)
		{
			Ymax = Y;
		}
	}

	float invYmax = 1 / Ymax;

	for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
	{
		float Y = 0.2126 * pixels[i].r + 0.7152 * pixels[i].g + 0.0722 * pixels[i].b;
		
		float Yr = Y * invYmax;
		float D = (alpha * Yr) / (1 + alpha * Yr);
		pixels[i] = pixels[i] * D / Y;
	}
}

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization( )
{
	scene_objects[0] = &wall;
	scene_objects[1] = &scene_floor;
	scene_objects[2] = &polyhedron;
	scene_objects[3] = &parametricSurface;
	scene_objects[4] = &implicitSurface;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	last_render_time = glutGet(GLUT_ELAPSED_TIME);
}
bool first_run = true;
// Rajzolas, minden frame megjelenitesehez ez a fuggveny hivodik meg
void onDisplay( )
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);		// torlesi szin beallitasa
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

	if (first_run)
	{
		first_run = false;
		RenderScene();
	}
	glDrawPixels(WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGB, GL_FLOAT, pixels);
    // ...

    glutSwapBuffers();     				// Buffercsere: rajzolas vege
}

// Billentyuzet esemenyeket lekezelo fuggveny
void onKeyboard(unsigned char key, int x, int y)
{
	if (key == 'w')
	{
	}
	else if (key == 'a')
	{
		camera.Rotatate(0.1);
		glutPostRedisplay();
	}
	else if (key == 's')
	{
	}
	else if (key == 'd')
	{
		camera.Rotatate(-0.1);
		glutPostRedisplay();
	}
}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT && state == GLUT_DOWN);  // A GLUT_LEFT_BUTTON / GLUT_RIGHT_BUTTON illetve GLUT_DOWN / GLUT_UP
}

// `Idle' esemenykezelo, jelzi, hogy az ido telik
void onIdle( )
{
     long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido
	 float elapsedSeconds = (time - last_render_time) * 0.001;
	 last_render_time = time;
	 //lamp.Update(elapsedSeconds);
	 //glutPostRedisplay();
}

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char **argv) {
    glutInit(&argc, argv); 				// GLUT inicializalasa
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);	// Windows ablak kezdeti meret 600x600 pixel 
    glutInitWindowPosition(0, 0);			// Az elozo Windows ablakhoz kepest hol tunik fel
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

