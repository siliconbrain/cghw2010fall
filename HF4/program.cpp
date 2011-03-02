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

// Constants:
const float EPSILON = 1e-7;
const float PI = 3.1415926535897932384626433832795;
const float PI_OVER_2 = 1.5707963267948966192313216916398;

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;
const float WINDOW_ASPECT_RATIO = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

const float GROUND_X = 100;
const float GROUND_Z = 100;

float RadiansToDegrees(float radians)
{
	return (radians / PI) * 180;
}

float DegreesToRadians(float degrees)
{
	return (degrees / 180) * PI;
}

struct TextureCoord
{
	float U;
	float V;

	TextureCoord(): U(0), V(0)
	{
	}

	TextureCoord(float u, float v) : U(u), V(v)
	{
	}
};

struct Vector3
{
	float X;
	float Y;
	float Z;

	Vector3() : X(0), Y(0), Z(0)
	{
	}

	Vector3(float x, float y, float z): X(x), Y(y), Z(z)
	{
	}

	Vector3& operator+=(const Vector3& rhs)
	{
		X += rhs.X;
		Y += rhs.Y;
		Z += rhs.Z;

		return *this;
	}

	const Vector3 operator+(const Vector3& rhs) const
	{
		return (Vector3(*this) += rhs);
	}

	Vector3& operator*=(const Vector3& rhs)
	{
		X *= rhs.X;
		Y *= rhs.Y;
		Z *= rhs.Z;

		return *this;
	}

	const Vector3 operator*(const Vector3& rhs) const
	{
		return (Vector3(*this) *= rhs);
	}
};

struct Vector4
{
	float W;
	float X;
	float Y;
	float Z;

	Vector4() : W(0), X(0), Y(0), Z(0)
	{
	}

	Vector4(float w, float x, float y, float z) : W(w), X(x), Y(y), Z(z)
	{
	}
};

struct Quaternion : public Vector4
{
	Quaternion() : Vector4(1, 0, 0, 0)
	{
	}

	Quaternion(Vector4 v) : Vector4(v)
	{
	}

	Quaternion(float w, float x, float y, float z) : Vector4(w, x, y, z)
	{
		float one_over_length = 1 / sqrt(x*x + y*y + z*z);
		x *= one_over_length;
		y *= one_over_length;
		z *= one_over_length;
	}

	Quaternion& operator*=(const Quaternion& rhs)
	{
		W = (W * rhs.W) - (X * rhs.X) - (Y * rhs.Y) - (Z * rhs.Z);
		X = (W * rhs.X) + (X * rhs.W) + (Y * rhs.Z) - (Z * rhs.Y);
		Y = (W * rhs.Y) + (Y * rhs.W) + (Z * rhs.X) - (X * rhs.Z);
		Z = (W * rhs.Z) + (Z * rhs.W) + (X * rhs.Y) - (Y * rhs.X);

		return *this;
	}

	const Quaternion operator*(const Quaternion& rhs) const
	{
		return (Quaternion(*this) *= rhs);
	}

	const Vector4 ToAxisAngleDegrees() const
	{
		return Vector4(RadiansToDegrees(2 * acos(W)), X, Y, Z);
	}

	static Quaternion FromAxisAngleRadians(Vector3 axis, float angle)
	{
		float sinThetaOver2 = sin(angle * 0.5);
		return Quaternion(cos(angle * 0.5), axis.X * sinThetaOver2, axis.Y * sinThetaOver2, axis.Z * sinThetaOver2);
	}

	static Quaternion FromAxisAngleDegrees(Vector3 axis, float angle)
	{
		return FromAxisAngleRadians(axis, DegreesToRadians(angle));
	}
};

struct Transformation
{
	Vector3 translation;
	Quaternion rotation;
	Vector3 scale;

	Transformation(Vector3 translation = Vector3(0,0,0), Quaternion rotation = Quaternion(1,0,0,0), Vector3 scale = Vector3(1,1,1))
		: translation(translation), rotation(rotation), scale(scale)
	{
	}

	void applyTransformation()
	{
		glTranslatef(translation.X, translation.Y, translation.Z);
		Vector4 rotAxAng = rotation.ToAxisAngleDegrees();
		glRotatef(rotAxAng.W, rotAxAng.X, rotAxAng.Y, rotAxAng.Z);
		glScalef(scale.X, scale.Y, scale.Z);
	}

	const Transformation Concat(const Transformation other) const
	{
		return Transformation(translation + other.translation, rotation * other.rotation, scale * other.scale);
	}
};

struct Vertex
{
	Vector3 position;
	Vector3 normal;
	TextureCoord texture_coord;

	Vertex(Vector3 position, Vector3 normal, TextureCoord textureCoord) : position(position), normal(normal), texture_coord(textureCoord)
	{
	}
};

const int NUM_OF_CUBE_VERTICES = 24;
Vertex cube_vertices[] = 
{
	// FRONT:
	Vertex(Vector3(0,0,1), Vector3( 0, 0, 1), TextureCoord(0,0)),
	Vertex(Vector3(1,0,1), Vector3( 0, 0, 1), TextureCoord(1,0)),
	Vertex(Vector3(1,1,1), Vector3( 0, 0, 1), TextureCoord(1,1)),
	Vertex(Vector3(0,1,1), Vector3( 0, 0, 1), TextureCoord(0,1)),
	// RIGHT:
	Vertex(Vector3(1,0,1), Vector3( 1, 0, 0), TextureCoord(0,0)),
	Vertex(Vector3(1,0,0), Vector3( 1, 0, 0), TextureCoord(1,0)),
	Vertex(Vector3(1,1,0), Vector3( 1, 0, 0), TextureCoord(1,1)),
	Vertex(Vector3(1,1,1), Vector3( 1, 0, 0), TextureCoord(0,1)),
	// BACK:
	Vertex(Vector3(1,0,0), Vector3( 0, 0,-1), TextureCoord(0,0)),
	Vertex(Vector3(0,0,0), Vector3( 0, 0,-1), TextureCoord(1,0)),
	Vertex(Vector3(0,1,0), Vector3( 0, 0,-1), TextureCoord(1,1)),
	Vertex(Vector3(1,1,0), Vector3( 0, 0,-1), TextureCoord(0,1)),
	// LEFT:
	Vertex(Vector3(0,0,0), Vector3(-1, 0, 0), TextureCoord(0,0)),
	Vertex(Vector3(0,0,1), Vector3(-1, 0, 0), TextureCoord(1,0)),
	Vertex(Vector3(0,1,1), Vector3(-1, 0, 0), TextureCoord(1,1)),
	Vertex(Vector3(0,1,0), Vector3(-1, 0, 0), TextureCoord(0,1)),
	// BOTTOM:
	Vertex(Vector3(0,0,0), Vector3( 0,-1, 0), TextureCoord(0,0)),
	Vertex(Vector3(1,0,0), Vector3( 0,-1, 0), TextureCoord(1,0)),
	Vertex(Vector3(1,0,1), Vector3( 0,-1, 0), TextureCoord(1,1)),
	Vertex(Vector3(0,0,1), Vector3( 0,-1, 0), TextureCoord(0,1)),
	// TOP:
	Vertex(Vector3(1,1,1), Vector3( 0, 1, 0), TextureCoord(0,0)),
	Vertex(Vector3(1,1,0), Vector3( 0, 1, 0), TextureCoord(1,0)),
	Vertex(Vector3(0,1,0), Vector3( 0, 1, 0), TextureCoord(1,1)),
	Vertex(Vector3(0,1,1), Vector3( 0, 1, 0), TextureCoord(0,1))
};
const int NUM_OF_CUBE_INDICES = 36;
int cube_indices[] = 
{
	// FRONT:
	0, 1, 2,
	0, 2, 3,
	// RIGHT:
	4, 5, 6,
	4, 6, 7,
	// BACK:
	8, 9, 10,
	8, 10, 11,
	// LEFT:
	12, 13, 14,
	12, 14, 15,
	// BOTTOM:
	16, 17, 18,
	16, 18, 19,
	// TOP:
	20, 21, 22,
	20, 22, 23
};

const int NUM_OF_GROUND_VERTICES = 4;
Vertex ground_vertices[] =
{
	Vertex(Vector3(-GROUND_X / 2,0,-GROUND_Z / 2), Vector3(0,1,0), TextureCoord(0,0)),
	Vertex(Vector3(-GROUND_X / 2,0, GROUND_Z / 2), Vector3(0,1,0), TextureCoord(0,2)),
	Vertex(Vector3( GROUND_X / 2,0, GROUND_Z / 2), Vector3(0,1,0), TextureCoord(2,2)),
	Vertex(Vector3( GROUND_X / 2,0,-GROUND_Z / 2), Vector3(0,1,0), TextureCoord(2,0))
};
const int NUM_OF_GROUND_INDICES = 6;
int ground_indices[] =
{
	0, 1, 2,
	0, 2, 3
};

struct Mesh
{
	Vertex* vertices;
	int* indices;
	int num_of_vertices;
	int num_of_indices;

	Mesh(Vertex* vertices, int numOfVertices, int* indices, int numOfIndices)
		: vertices(vertices), num_of_vertices(numOfVertices), indices(indices), num_of_indices(numOfIndices)
	{
	}
} cube_mesh = Mesh(cube_vertices, NUM_OF_CUBE_VERTICES, cube_indices, NUM_OF_CUBE_INDICES);

struct Material
{
	float shininess;
	float diffuse[4];
	float specular[4];
	float emissive[4];

	Material(float dR, float dG, float dB, float dA,
		float sR, float sG, float sB, float sA,
		float eR, float eG, float eB, float eA,
		float shine)
	{
		diffuse[0] = dR;
		diffuse[1] = dG;
		diffuse[2] = dB;
		diffuse[3] = dA;
		specular[0] = sR;
		specular[1] = sG;
		specular[2] = sB;
		specular[3] = sA;
		emissive[0] = eR;
		emissive[1] = eG;
		emissive[2] = eB;
		emissive[3] = eA;
		shininess = shine;
	}
};

class Model
{
private:
	Mesh mesh;
	Material material;
	GLuint texture;
	Transformation mesh_transform;

public:
	Model(Mesh mesh, Material material, Transformation meshTransform = Transformation(), GLuint texture = 0)
		: mesh(mesh), material(material), mesh_transform(meshTransform), texture(texture)
	{
	}

	void setTexture(GLuint texture)
	{
		this->texture = texture;
	}

	void Draw()
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material.diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material.specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material.emissive);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material.shininess);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		mesh_transform.applyTransformation();

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < mesh.num_of_indices; i++)
		{
			Vertex& curVertex = mesh.vertices[mesh.indices[i]];
			glTexCoord2f(curVertex.texture_coord.U, curVertex.texture_coord.V);
			glNormal3f(curVertex.normal.X, curVertex.normal.Y, curVertex.normal.Z);
			glVertex3f(curVertex.position.X, curVertex.position.Y, curVertex.position.Z);
		}
		glEnd();

		glPopMatrix();
	}
};

// Globals:
long lastUpdateTime;

class Camera
{
private:
	// for View matrix
	Vector3 position;
	Vector3 look_at;
	Vector3 up;
	float rot;

	void setupProjectionMatrix()
	{
		// switch to Projection matrix
		glMatrixMode(GL_PROJECTION);
		// save identity matrix
		glPushMatrix();
		// set projection matrix
		gluPerspective(PI_OVER_2, WINDOW_ASPECT_RATIO, 100, 2000);
	}

	void createViewMatrix()
	{
		// set look-at matrix
		gluLookAt(position.X, position.Y, position.Z, look_at.X, look_at.Y, look_at.Z, up.X, up.Y, up.Z);
		glTranslatef(look_at.X, look_at.Y, look_at.Z);
		glRotatef(rot, 0, 1, 0);
		glTranslatef(-look_at.X, -look_at.Y, -look_at.Z);
	}

	void setupViewMatrix()
	{
		// switch to Model-View matrix
		glMatrixMode(GL_MODELVIEW);
		// save identity matrix
		glPushMatrix();

		createViewMatrix();
	}

	void refreshViewMatrix()
	{
		// switch to Model-View matrix
		glMatrixMode(GL_MODELVIEW);
		// pop off the last view matrix from the stack and expose identity matrix
		glPopMatrix();
		// push identity matrix down the stack, and leave a copy of it on top
		glPushMatrix();

		createViewMatrix();
	}

public:
	Camera()
	{
		position = Vector3(700,400,700);
		look_at = Vector3(0,0,0);
		up = Vector3(0,1,0);
		rot = 0;
	}

	void Init()
	{
		setupProjectionMatrix();
		setupViewMatrix();
	}

	void Update(float elapsedSeconds)
	{
		rot += 24 * elapsedSeconds;

		refreshViewMatrix();
	}
};

class VisibleObject
{
protected:
	Transformation transform;

	VisibleObject(Transformation transform) : transform(transform)
	{
	}

	void BeginDraw()
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		transform.applyTransformation();
	}

	void EndDraw()
	{
		glPopMatrix();
	}

public:
	virtual void Draw() = 0;
};

class PrimitiveVisibleObject : public VisibleObject
{
protected:
	Model model;

	PrimitiveVisibleObject(Transformation transform, Model model) : VisibleObject(transform), model(model)
	{
	}

public:
	void Draw()
	{
		BeginDraw();

		model.Draw();

		EndDraw();
	}
};

struct Sun
{
	float direction[4];

	Sun(Vector3 position = Vector3(0,0,0))
	{
		direction[0] = position.X;
		direction[1] = position.Y;
		direction[2] = position.Z;
		direction[3] = 0;
	}
};

class Obstacle : public PrimitiveVisibleObject
{
public:
	Obstacle(Transformation transform = Transformation()) : PrimitiveVisibleObject(transform,
		Model(cube_mesh, Material(0.3,0.1,0,1,	0,0,0,0,	0,0,0,0,	0), Transformation(Vector3(-1, 0, -1), Quaternion(), Vector3(2, 3, 2))))
	{
	}

	void Init()
	{
		float noise[16*16];
		for (int i = 0; i < 16*16; i++)
		{
			noise[i] = rand() % 1000 * 0.001;
		}

		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 16, 16, 0, GL_LUMINANCE, GL_FLOAT, noise);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		model.setTexture(texture);
	}
};

class Ground : public PrimitiveVisibleObject
{
private:
	static const int TEX_SIZE = 256;

public:
	Ground(Transformation transform)
		: PrimitiveVisibleObject(transform, Model(
				Mesh(ground_vertices, NUM_OF_GROUND_VERTICES, ground_indices, NUM_OF_GROUND_INDICES),
				Material(0.5,0.3,0,1,	0,0,0,0,	0,0,0,0,	0)))
	{
	}

	void Init()
	{
		// generate texture:
		float tex_data[TEX_SIZE*TEX_SIZE];

		srand(glutGet(GLUT_ELAPSED_TIME));

		for (int i = 0; i < TEX_SIZE * TEX_SIZE; i++)
		{
			tex_data[i] = ((rand() % 1000) * 0.00075) + 0.25;
		}

		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEX_SIZE, TEX_SIZE, 0, GL_LUMINANCE, GL_FLOAT, tex_data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		model.setTexture(texture);
	}
};

class Environment
{
private:
	Ground ground;
	Sun suns[3];
	Obstacle obstacles[20];

	void setupLights()
	{
		float ambientLight[4] = {0,0,0,1};
		float diffuseAndSpecularLight[4] = {1,1,1,1};
		glLightfv(GL_LIGHT0, GL_POSITION, suns[0].direction);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseAndSpecularLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, diffuseAndSpecularLight);
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT1, GL_POSITION, suns[1].direction);
		glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseAndSpecularLight);
		glLightfv(GL_LIGHT1, GL_SPECULAR, diffuseAndSpecularLight);
		glEnable(GL_LIGHT1);
		glLightfv(GL_LIGHT2, GL_POSITION, suns[2].direction);
		glLightfv(GL_LIGHT2, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuseAndSpecularLight);
		glLightfv(GL_LIGHT2, GL_SPECULAR, diffuseAndSpecularLight);
		glEnable(GL_LIGHT2);
	}

public:
	Environment() : ground(Transformation(Vector3(0,0,0)))
	{
		suns[0] = Sun(Vector3(-1.0, 1.0,-1.0));
		suns[1] = Sun(Vector3( 1.0, 1.0,-1.0));
		suns[2] = Sun(Vector3( 1.0, 0.2, 1.0));

		for (int i = 0; i < 20; i++)
		{
			float x = rand() % 10000 * 0.008;
			if (x < 40)
			{
				x += 10;
			}
			else
			{
				x *= -1;
				x += 30;
			}

			float z = rand() % 10000 * 0.008;
			if (z < 40)
			{
				z += 10;
			}
			else
			{
				z *= -1;
				z += 30;
			}

			obstacles[i] = Obstacle(Transformation(Vector3(x, 0, z)));
		}
	}

	void Draw()
	{
		glLightfv(GL_LIGHT0, GL_POSITION, suns[0].direction);
		glLightfv(GL_LIGHT1, GL_POSITION, suns[1].direction);
		glLightfv(GL_LIGHT2, GL_POSITION, suns[2].direction);

		ground.Draw();

		for (int i = 0; i < 20; i++)
		{
			obstacles[i].Draw();
		}
	}

	void Init()
	{
		setupLights();

		ground.Init();
		for (int i = 0; i < 20; i++)
		{
			obstacles[i].Init();
		}
	}
};

class Leg : public PrimitiveVisibleObject
{
public:
	Leg(Transformation transform) : PrimitiveVisibleObject(transform,
		Model(cube_mesh, Material(0.5,0.5,0.5,1, 1,1,1,1, 0,0,0,0, 20), Transformation(Vector3(-0.4, -2, -0.15), Quaternion(), Vector3(0.8, 2, 0.3))))
	{

	}

	void setTexture(GLuint texture)
	{
		model.setTexture(texture);
	}

	void Rotate(Quaternion rotation)
	{
		transform.rotation *= rotation;
	}
};

class Thigh : public VisibleObject
{
private:
	Model model;
	Leg leg;

public:
	Thigh(Transformation transform) : VisibleObject(transform), leg(Transformation(Vector3(0, -2, 0))),
		model(Model(cube_mesh, Material(0.5,0.5,0.5,1, 1,1,1,1, 0,0,0,0, 20), Transformation(Vector3(-0.5, -2, -0.2), Quaternion(), Vector3(1, 2, 0.4))))
	{

	}

	void Draw()
	{
		BeginDraw();

		model.Draw();
		leg.Draw();

		EndDraw();
	}

	void setTexture(GLuint texture)
	{
		model.setTexture(texture);
	}

	void Rotate(Quaternion rotation)
	{
		transform.rotation *= rotation;
	}

	void BendLeg(float amount)
	{
		leg.Rotate(Quaternion::FromAxisAngleDegrees(Vector3(1, 0, 0), 90 * amount));
	}

	void LiftLeg(float amount)
	{
		transform.rotation *= Quaternion::FromAxisAngleDegrees(Vector3(1, 0, 0), -90 * amount);
	}
};

class LowerTorso : public VisibleObject
{
private:
	Model model;
	Thigh left_thigh;
	Thigh right_thigh;

public:
	LowerTorso(Transformation transform) : VisibleObject(transform), left_thigh(Transformation(Vector3(0.75, 0, 0))), right_thigh(Transformation(Vector3(-0.75, 0, 0))),
		model(Model(cube_mesh, Material(0.5,0.5,0.5,1, 1,1,1,1, 0,0,0,0, 20), Transformation(Vector3(-2, 0, -2), Quaternion(), Vector3(4, 1, 4))))
	{
	}

	void Draw()
	{
		BeginDraw();

		model.Draw();
		left_thigh.Draw();
		right_thigh.Draw();

		EndDraw();
	}

	void setTexture(GLuint texture)
	{
		model.setTexture(texture);
		left_thigh.setTexture(texture);
		right_thigh.setTexture(texture);
	}

	void BendLeftLeg(float amount)
	{
		left_thigh.BendLeg(amount);
	}

	void LiftLeftLeg(float amount)
	{
		left_thigh.LiftLeg(amount);
	}

	void BendRightLeg(float amount)
	{
		right_thigh.BendLeg(amount);
	}

	void LiftRightLeg(float amount)
	{
		right_thigh.LiftLeg(amount);
	}
};

class Barrel : public PrimitiveVisibleObject
{
public:
	Barrel(Transformation transform) : PrimitiveVisibleObject(transform,
		Model(cube_mesh, Material(0.5,0.5,0.5,1, 1,1,1,1, 0,0,0,0, 20), Transformation(Vector3(-0.15, -0.15, 0), Quaternion(), Vector3(0.3, 0.3, 2.5))))
	{
	}

	void setTexture(GLuint texture)
	{
		model.setTexture(texture);
	}
};

class UpperTorso : public VisibleObject
{
private:
	Model model;
	Barrel barrel;

public:
	UpperTorso(Transformation transform) : VisibleObject(transform), barrel(Transformation(Vector3(0, 1, 2))),
		model(Model(cube_mesh, Material(0.5,0.5,0.5,1, 1,1,1,1, 0,0,0,0, 20), Transformation(Vector3(-2, 0, -2), Quaternion(), Vector3(4, 3, 4))))
	{
	}

	void Draw()
	{
		BeginDraw();

		model.Draw();
		barrel.Draw();

		EndDraw();
	}

	void setTexture(GLuint texture)
	{
		model.setTexture(texture);
		barrel.setTexture(texture);
	}
};

class ATEW : public VisibleObject
{
private:
	UpperTorso upper_torso;
	LowerTorso lower_torso;

public:
	ATEW(Vector3 startPos = Vector3(0,0,0)) : VisibleObject(Transformation(Vector3(startPos.X, startPos.Y + 4.7, startPos.Z))),
		upper_torso(Transformation()), lower_torso(Transformation(Vector3(0, -1.1, 0)))
	{

	}

	void Draw()
	{
		BeginDraw();

		upper_torso.Draw();
		lower_torso.Draw();

		EndDraw();
	}

	void Init()
	{
		float texData[] = {0.75};

		GLuint texture = 0;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1, 1, 0, GL_LUMINANCE, GL_FLOAT, texData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		upper_torso.setTexture(texture);
		lower_torso.setTexture(texture);

		lower_torso.BendLeftLeg(0.5);
		lower_torso.LiftLeftLeg(0.5);
		lower_torso.LiftRightLeg(-0.5);
	}

	void Update(float elapsedSeconds)
	{
	}
};

class Scene
{
private:
	Camera camera;
	Environment environment;
	ATEW atews[2];

public:
	Scene()
	{
		camera = Camera();
		environment = Environment();
		atews[0] = ATEW(Vector3(6, 0, 6));
		atews[1] = ATEW(Vector3(-6, 0, -6));
	}

	void Draw()
	{
		atews[0].Draw();
		atews[1].Draw();

		environment.Draw();
	}

	void Init()
	{
		camera.Init();
		environment.Init();
		atews[0].Init();
		atews[1].Init();
	}

	void Update(float elapsedSeconds)
	{
		camera.Update(elapsedSeconds);
		atews[0].Update(elapsedSeconds);
		atews[1].Update(elapsedSeconds);
	}
} scene;

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization( )
{
	// setup OGL:
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	GLfloat ambientColor[] = {0.2,0.2,0.2,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambientColor);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);

	// init time
	lastUpdateTime = glutGet(GLUT_ELAPSED_TIME);

	// init scene
	scene.Init();
}

// Rajzolas, minden frame megjelenitesehez ez a fuggveny hivodik meg
void onDisplay( )
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// torlesi szin beallitasa
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

	scene.Draw();

	glutSwapBuffers();     				// Buffercsere: rajzolas vege
}

// Billentyuzet esemenyeket lekezelo fuggveny
void onKeyboard(unsigned char key, int x, int y)
{
	if (key == 'd') glutPostRedisplay( ); 		// d beture rajzold ujra a kepet

}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT && state == GLUT_DOWN);  // A GLUT_LEFT_BUTTON / GLUT_RIGHT_BUTTON illetve GLUT_DOWN / GLUT_UP
}

// `Idle' esemenykezelo, jelzi, hogy az ido telik
void onIdle( )
{
	// calc elapsed time since last update
	long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido
	float elapsedSeconds = (time - lastUpdateTime) * 0.001;
	lastUpdateTime = time;

	scene.Update(elapsedSeconds);

	glutPostRedisplay();
}

// ...Idaig modosithatod
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// A C++ program belepesi pontja, a main fuggvenyt mar nem szabad bantani
int main(int argc, char **argv) {
    glutInit(&argc, argv); 				// GLUT inicializalasa
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);			// Windows ablak kezdeti meret 600x600 pixel 
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

