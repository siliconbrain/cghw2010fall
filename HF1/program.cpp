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

// ================ HEADER ================

// ================ structs: ================

struct PointF { float x;	float y; };

struct SizeF { float width;	float height; };

struct RectangleF {	PointF position; SizeF size; };

struct Color3 {	float r; float g; float b; };

// ================ classes: ================

class TrafficLight
{
public:
	enum State
	{
		Green,
		Yellow,
		Red,
		RedYellow
	};

	TrafficLight();
	void Draw();
	void Update(float elapsedSeconds);
	State getLightState();

private:
	State state;
	float cycleTime;
};

class Car
{
public:
	Car();
	void Draw();
	void Update(float elapsedSeconds);
	RectangleF getBody();

private:
	RectangleF body;
	Color3 car_color;

	void newCar();
};

class Pedestrian
{
public:
	Pedestrian();
	void Draw();
	void Update(float elapsedSeconds);
	void onMouseClick(float x, float y);
	void onKeyboard();

private:
	RectangleF body;
	bool is_dead;
	bool is_started;

	void newPedestrian();
};

// ================ /HEADER ================

// ================ globals: ================

RectangleF viewport;
Car car;
TrafficLight trafficLight;
Pedestrian pedestrian;
// used to calculate elapsed time
long totalElapsedMilliseconds;

// ================ helper functions: ================

#ifdef min(a,b)
#undef min(a,b)
#endif
float min(float a, float b)
{
	return (a<b)?a:b;
}

float clamp(float value, float min, float max)
{
	return (value < min)?min:((value > max)?max:value);
}

// see if a point is in a rectangle
bool pointIsInRectangle(float pointX, float pointY, RectangleF rectangle)
{
	if ((pointX > rectangle.position.x) &&
		(pointX < (rectangle.position.x + rectangle.size.width)) &&
		(pointY > rectangle.position.y) &&
		(pointY < (rectangle.position.y + rectangle.size.height)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// see if two rectangles intersect. rectangle size matters: the bigger can never be entirely in the smaller
bool intersect(RectangleF bigger, RectangleF smaller)
{
	if (pointIsInRectangle(smaller.position.x, smaller.position.y, bigger) ||
		pointIsInRectangle(smaller.position.x + smaller.size.width, smaller.position.y, bigger) ||
		pointIsInRectangle(smaller.position.x + smaller.size.width, smaller.position.y + smaller.size.height, bigger) ||
		pointIsInRectangle(smaller.position.x, smaller.position.y + smaller.size.height, bigger))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// Draw vertex defined in virtual space to screen space. Also applies clamping.
void mglVertex(float x, float y)
{
	glVertex2f(clamp((x * 0.1f * viewport.size.width) + viewport.position.x, viewport.position.x, viewport.position.x + viewport.size.width),
		clamp((y * 0.1f * viewport.size.height) + viewport.position.y, viewport.position.y, viewport.position.y + viewport.size.height));
}

// Draw rectangle from 4 floats
void mglRectangle(float x, float y, float width, float height)
{
	mglVertex(x, y);
	mglVertex(x + width, y);
	mglVertex(x + width, y + height);
	mglVertex(x, y + height);
}

// Draw rectangle from RectangleF
void mglRectangleR(RectangleF rectangle)
{
	mglRectangle(rectangle.position.x, rectangle.position.y, rectangle.size.width, rectangle.size.height);
}

// Set color from Color3
void mglColorC(Color3 color)
{
	glColor3f(color.r, color.g, color.b);
}

// ================ environment drawer functions: ================

void DrawEnvironment()
{
	// draw asphalt
	glColor3f(0.1f, 0.1f, 0.1f);
	mglRectangle(0.0f, 0.0f, 10.0f, 10.0f);

	// draw crossing
	float crossingX = 3.2f;
	float crossingY = 3.5f;

	float pieceWidth = 0.4f;
	float pieceHeight = 3.0f;

	float pieceSpacing = 0.4f;

	glColor3f(1.0f, 1.0f, 1.0f);

	for (int i = 0; i < 5; i++)
	{
		mglRectangle(crossingX + (i * (pieceSpacing + pieceWidth)), crossingY, pieceWidth, pieceHeight);
	}

	// draw sidewalks
	float sidewalkHeight = 10.0f;
	float sidewalkWidth = 2.8f;

	glColor3f(0.4f, 0.4f, 0.4f);

	// Left sidewalk
	mglRectangle(0.0f, 0.0f, sidewalkWidth, sidewalkHeight);

	// Right sidewalk
	mglRectangle(10.0f - sidewalkWidth, 0.0f, sidewalkWidth, sidewalkHeight);
}

// ================ event handlers: ================

// Inicializacio, a program futasanak kezdeten, az OpenGL kontextus letrehozasa utan hivodik meg (ld. main() fv.)
void onInitialization( ) {
	// start in fullscreen (window)
	viewport.position.x = -1.0f;
	viewport.position.y = -1.0;
	viewport.size.width = 2.0;
	viewport.size.height = 2.0;

	// seed random
	srand(glutGet(GLUT_ELAPSED_TIME));
}

// Rajzolas, minden frame megjelenitesehez ez a fuggveny hivodik meg
void onDisplay( ) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		// torlesi szin beallitasa
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // kepernyo torles

	glBegin(GL_QUADS);
	DrawEnvironment();
	trafficLight.Draw();
	pedestrian.Draw();
	car.Draw();
	glEnd();
    // ...

    glutSwapBuffers();     				// Buffercsere: rajzolas vege

}

// Billentyuzet esemenyeket lekezelo fuggveny
void onKeyboard(unsigned char key, int x, int y) {
	if (key == ' ')
	{
		// create new random viewport parameters
		viewport.position.x = ((rand() % 1000) * 0.002f) - 1.0f;
		viewport.position.y = ((rand() % 1000) * 0.002f) - 1.0f;
		float randSize = (rand() % 1000) * 0.002;
		float maxX = 1.0 - viewport.position.x;
		float maxY = 1.0 - viewport.position.y;
		float newSize = (randSize<min(maxX, maxY))?randSize:min(maxX, maxY);
		viewport.size.height = newSize;
		viewport.size.width = newSize;

		// call pedestrian SPACE handler
		pedestrian.onKeyboard();
	}
}

// Eger esemenyeket lekezelo fuggveny
void onMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT && state == GLUT_DOWN)
	{
		// calculate virtual mouse position
		float mousePosX = (((x - 300.0f) / 300.0f) - viewport.position.x) / viewport.size.width * 10.0f;
		float mousePosY = (((y - 300.0f) / -300.0f) - viewport.position.y) / viewport.size.height * 10.0f;
		// call pedestrian mouse handler
		pedestrian.onMouseClick(mousePosX, mousePosY);
	}
}

// `Idle' esemenykezelo, jelzi, hogy az ido telik
void onIdle( ) {
	long time = glutGet(GLUT_ELAPSED_TIME);		// program inditasa ota eltelt ido

	// calculate elapsed seconds
	float elapsedSeconds = (time - totalElapsedMilliseconds) * 0.001f;
	totalElapsedMilliseconds = time;
	// update all enities
	trafficLight.Update(elapsedSeconds);
	car.Update(elapsedSeconds);
	pedestrian.Update(elapsedSeconds);

	glutPostRedisplay();
}

// ================ function definitions ================

Car::Car()
{
	newCar();
}

void Car::newCar()
{
	body.position.x = 3.1f;
	body.position.y = 10.0f;
	body.size.width = 2.0f;
	body.size.height = 4.0f;
	// assign random color
	car_color.r = (rand() % 1000) * 0.001;
	car_color.g = (rand() % 1000) * 0.001;
	car_color.b = (rand() % 1000) * 0.001;
}

void Car::Draw()
{
	mglColorC(car_color);
	mglRectangleR(body);
}

void Car::Update(float elapsedSeconds)
{
	// if car is out of view create new car
	if (body.position.y < -body.size.height)
	{
		newCar();
	}

	// if the light is green or yellow, or the car passed the stop lane move forward
	if ((trafficLight.getLightState() == TrafficLight::Green) || (trafficLight.getLightState() == TrafficLight::Yellow) || (body.position.y < 6.8f))
	{
		body.position.y -= 10.0f / 3.0f * elapsedSeconds;
	}
}

RectangleF Car::getBody()
{
	return body;
}

TrafficLight::TrafficLight()
{
	state = TrafficLight::Green;
	cycleTime = 0.0f;
}

void TrafficLight::Draw()
{
	// car green (bottom) light
	switch (state)
	{
		case Green:
			glColor3f(0.0f, 1.0f, 0.0f);
			break;
		default:
			glColor3f(0.0f, 0.0f, 0.0f);
	}
	mglRectangle(2.4f, 6.5f, 0.3f, 0.3f);

	// car yellow (middle) light
	switch (state)
	{
		case Yellow:
		case RedYellow:
			glColor3f(1.0f, 1.0f, 0.0f);
			break;
		default:
			glColor3f(0.0f, 0.0f, 0.0f);
	}
	mglRectangle(2.4f, 6.2f, 0.3f, 0.3f);

	// car red (top) light
	switch (state)
	{
		case Red:
		case RedYellow:
			glColor3f(1.0f, 0.0f, 0.0f);
			break;
		default:
			glColor3f(0.0f, 0.0f, 0.0f);
	}
	mglRectangle(2.4f, 5.9f, 0.3f, 0.3f);

	// pedestrian green (bottom) light
	switch (state)
	{
		case Red:
		case RedYellow:
			glColor3f(0.0f, 1.0f, 0.0f);
			break;
		default:
			glColor3f(0.0f, 0.0f, 0.0f);
	}
	mglRectangle(2.1f, 5.6f, 0.3f, 0.3f);

	// pedestrian red (top) light
	switch (state)
	{
		case Green:
		case Yellow:
			glColor3f(1.0f, 0.0f, 0.0f);
			break;
		default:
			glColor3f(0.0f, 0.0f, 0.0f);
	}
	mglRectangle(1.8f, 5.6f, 0.3f, 0.3f);
}

TrafficLight::State TrafficLight::getLightState()
{
	return state;
}

void TrafficLight::Update(float elapsedSeconds)
{
	cycleTime += elapsedSeconds;

	if (cycleTime < 5.0f)
	{
		state = Green;
	}
	else if (cycleTime < 9.0f)
	{
		state = Yellow;
	}
	else if (cycleTime < 13.0f)
	{
		state = Red;
	}
	else if (cycleTime < 14.0f)
	{
		state = RedYellow;
	}
	else
	{
		state = Green;
		cycleTime -= 14.0f;
	}
}

Pedestrian::Pedestrian()
{
	newPedestrian();
}

void Pedestrian::newPedestrian()
{
	body.size.width = 0.3f;
	body.size.height = 0.6f;
	body.position.x = 10.0f - body.size.width;
	body.position.y = (10.0f - body.size.height) / 2.0f;
	is_dead = false;
	is_started = false;
}

void Pedestrian::Draw()
{
	if (!is_dead)
	{
		glColor3f(0.9336f, 0.8125f, 0.7086f);
		mglRectangleR(body);
	}
}

void Pedestrian::Update(float elapsedSeconds)
{
	// see if car hit pedestrian
	if (intersect(car.getBody(), body))
	{
		is_dead = true;
	}

	// if the pedestrian if alive and started moving, move forward till the other end and then die
	if (!is_dead && is_started)
	{
		if (body.position.x < -body.size.width)
		{
			is_dead = true;
		}

		body.position.x -= 2.0f * elapsedSeconds;
	}
}

void Pedestrian::onMouseClick(float x, float y)
{
	// start if the mouse is over the pedestrian
	if ((x > body.position.x) &&
		(x < (body.position.x + body.size.width)) &&
		(y > body.position.y) &&
		(y < (body.position.y + body.size.height)))
	{
		is_started = true;
	}
}

void Pedestrian::onKeyboard()
{
	// if the pedestrian is dead, create new pedestrian
	if (is_dead)
	{
		newPedestrian();
	}
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

