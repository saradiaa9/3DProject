#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <chrono>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <string>
#include <vector> 
#include <Windows.h>



#define GLUT_KEY_ESCAPE 27
#define DEG2RAD(a) (a * 0.0174532925)

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

int WIDTH = 1280;
int HEIGHT = 720;
void* font = GLUT_BITMAP_TIMES_ROMAN_24;

GLuint tex;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;
int knightX = 660; //670
int knightY = 45; //45
int knightZ = -100; //-70
int knightR = 180;
bool level1 = true;

GLfloat lightIntensity[] = { 0.5, 0.5, 0.5, 1.0f };
GLfloat maxIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
GLfloat minIntensity[] = { 0.4, 0.4, 0.4, 1.0f };
bool increaseIntensity = true;
float lightposition = 0.0f;
std::chrono::time_point<std::chrono::steady_clock> intensityChangeTime = std::chrono::steady_clock::now();
std::chrono::time_point<std::chrono::steady_clock> startTime = std::chrono::steady_clock::now();


//=======================================================================
// Helper Methods
//=======================================================================
void updateLightingIntensity() {
	std::chrono::time_point<std::chrono::steady_clock> currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsedSeconds = currentTime - intensityChangeTime;

	if (elapsedSeconds.count() >= 2) {
		intensityChangeTime = currentTime;
		if (!level1) {
			if (increaseIntensity) {
				for (int i = 0; i < 3; ++i) {
					if (lightIntensity[i] < maxIntensity[i]) {
						lightIntensity[i] += 0.1f;
					}
				}
				if (lightIntensity[0] >= maxIntensity[0]) {
					increaseIntensity = false;
				}
			}
			else {
				for (int i = 0; i < 3; ++i) {
					if (lightIntensity[i] > minIntensity[i]) {
						lightIntensity[i] -= 0.1f;
					}
				}
				if (lightIntensity[0] <= minIntensity[0]) {
					increaseIntensity = true;
				}
			}
		}
		else {
			for (int i = 0; i < 3; ++i) {
				if (lightIntensity[i] < maxIntensity[i]) {
					lightIntensity[i] = maxIntensity[i];
				}
				else {
					lightIntensity[i] = minIntensity[i];
				}
			}
		}
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	}
}


class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 3.0f, float eyeY = 3.0f, float eyeZ = 3.0f, float centerX = 2.0f, float centerY = 0.0f, float centerZ = 2.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}

	void setFrontView() {
		eye = Vector3f(1.0f, 1.0f, 2.0f);
		center = Vector3f(1.0f, 1.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);


		// Rotate the view to the left
		float angle = -90.0f; // Adjust the angle as needed
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(angle)) + right * sin(DEG2RAD(angle));
		up = view.cross(right);
		center = eye + view;
	}


	void setTopView() {
		// Move the camera and look-at point along the x-axis
		eye = Vector3f(0.0f, 70.0f, 0.0f);
		center = Vector3f(0.0f, 0.0f, 0.0f);

		// Keep the up vector unchanged
		up = Vector3f(0.0f, 0.0f, -1.0f);
	}

	void setSideView() {
		eye = Vector3f(1.0f, 1.0f, 2.0f);
		center = Vector3f(1.0f, 1.0f, 0.0f);
		up = Vector3f(0.0f, 1.0f, 0.0f);

		// Rotate the view to the left
		float angle = 30.0f; // Adjust the angle as needed
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(angle)) + right * sin(DEG2RAD(angle));
		up = view.cross(right);
		center = eye + view;
	}
};


Camera camera;


// Model Variables
Model_3DS model_castle;
Model_3DS model_tower;
Model_3DS model_maze;
Model_3DS model_knight;
Model_3DS model_girl;
Model_3DS model_key;
Model_3DS model_wall;
Model_3DS model_coin;
Model_3DS model_sword;
Model_3DS model_rock;

int knightYPrev = 0;
bool win = false;
bool haveKey = false;
bool isJumping = false;
bool jumpStairs = false;
bool hasBoost = false;
int coinsCollected = 0;
bool coin1 = true;
bool coin2 = true;
bool coin3 = true;
bool coin4 = true;
bool coin5 = true;
int swordsCollected = 0;
bool sword1 = true;
bool sword2 = true;
bool sword3 = true;
bool sword4 = true;
bool sword5 = true;
bool sword6 = true;
bool sword7 = true;
bool sword8 = true;
bool sword9 = true;

bool cameraP = false;
int score = 0;
bool level1Text = false;

// Textures
GLTexture tex_ground;
GLTexture tex_ground1;


void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1200 / 750, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}
//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource() {
	// Material properties
	GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Increased ambient intensity
	GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Increased diffuse intensity
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat shininess[] = { 100.0f };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	// Light properties
	GLfloat lightIntensity[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Increased intensity
	GLfloat lightPosition[] = { -7.0f, 6.0f, 3.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightIntensity);

	// Enable lighting and light 0
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);  // Enable normalization of normals for proper lighting
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*****//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*****//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	//*****//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*****//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glScalef(2, 2, 2);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderGround1()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground1.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glScalef(2, 2, 2);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderGround2()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glScalef(2, 2, 2);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void updateKnight() {
	if (hasBoost) {
		knightY += 100.0;
		knightX -= 100.0;
		hasBoost = false;
	}
	if (isJumping) {
		// Apply upward motion during the jump
		knightY += 2.0; // Adjust the jump height as needed
		if (knightR == 180) {
			if (knightZ <= -1050) {
				knightZ = -1050;
			}
			else {
				knightZ -= 40;
			}
		}
		else if (knightR == 0) {
			if (knightZ >= 1050) {
				knightZ = 1050;
			}
			else {
				knightZ += 40;
			}
		}
		else if (knightR == 90) {
			if (knightX == -20 && !haveKey) {
				knightX = -10;
			}
			else {
				knightX -= 40;
			}

		}
		else if (knightR == -90) {
			if (knightX >= 670) {
				knightX = 670;
			}
			else {
				knightX += 40;
			}
		}
		if (knightY >= 50.0) { // Adjust the maximum jump height
			isJumping = false;
		}
	}
	else {
		// Apply gravity or any other vertical motion
		knightY -= 1.0; // Adjust the gravity or falling speed as needed
		if (knightY <= 45.0) {
			knightY = 45.0;
		}
	}



}



void checkRockCollision_1() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -115;
	float rockY = 20;
	float rockZ = -45;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 70;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword1) {
			sword1 = false;
			swordsCollected++;

		}
	}

}

void checkRockCollision_2() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -165;
	float rockY = 40;
	float rockZ = -55;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword2) {
			sword2 = false;
			swordsCollected++;
			hasBoost = true;
		}
	}

}


void checkRockCollision_3() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -215;
	float rockY = 60;
	float rockZ = -65;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 90;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword3) {
			sword3 = false;
			swordsCollected++;
		}
	}

}


void checkRockCollision_4() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -225;
	float rockY = 100;
	float rockZ = -75;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword4) {
			sword4 = false;
			swordsCollected++;
		}
	}

}
void checkRockCollision_5() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -245;
	float rockY = 140;
	float rockZ = -85;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword5) {
			sword5 = false;
			swordsCollected++;
		}
	}

}

void checkRockCollision_6() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -265;
	float rockY = 180;
	float rockZ = -95;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword6) {
			sword6 = false;
			swordsCollected++;

		}
	}

}


void checkRockCollision_7() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -275;
	float rockY = 220;
	float rockZ = -105;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword7) {
			sword7 = false;
			swordsCollected++;
		}
	}

}




void checkRockCollision_8() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -285;
	float rockY = 240;
	float rockZ = -115;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword8) {
			sword8 = false;
			swordsCollected++;
		}
	}

}
void checkRockCollision_9() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -305;
	float rockY = 260;
	float rockZ = -125;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 80;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
		if (sword9) {
			sword9 = false;
			swordsCollected++;
		}
	}

}
void checkRockCollision_10() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -315;
	float rockY = 280;
	float rockZ = -135;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 110;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
	}

}
void checkRockCollision_11() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -325;
	float rockY = 310;
	float rockZ = -145;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 110;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
	}

}

void checkRockCollision_12() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -335;
	float rockY = 330;
	float rockZ = -155;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 110;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
	}

}

void checkRockCollision_13() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -345;
	float rockY = 350;
	float rockZ = -165;
	float rockWidth = 60; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 110;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock
	}

}

void checkRockCollision_14() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -355;
	float rockY = 400;
	float rockZ = -185;
	float rockWidth = 90; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 110;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock

	}

}

void checkRockCollision_15() {
	// Assuming rock position is at (-350, 170, 0)
	float rockX = -355;
	float rockY = 440;
	float rockZ = -200;
	float rockWidth = 90; // Assuming width of the rock

	// Assuming character's width and height
	float characterWidth = 30;
	float characterHeight = 110;

	if (knightX + characterWidth >= rockX && knightX <= rockX + rockWidth &&
		knightZ >= rockZ - rockWidth / 2 && knightZ <= rockZ + rockWidth / 2 &&
		knightY <= rockY + characterHeight) {
		knightY = rockY + characterHeight; // Place the character on top of the rock

	}

}









void updateCoins(int x_pos, int z_pos) {
	if (x_pos >= 245 && x_pos <= 255 && z_pos >= -6 && z_pos <= 4 && coin1) {
		coin1 = false;
		coinsCollected++;
		score++;
		std::cout << coinsCollected << std::endl;
	}
	else if (x_pos >= 245 && x_pos <= 255 && z_pos >= -305 && z_pos <= -295 && coin2) {
		coin2 = false;
		coinsCollected++;
		score++;
		std::cout << coinsCollected << std::endl;
	}
	else if (x_pos >= 245 && x_pos <= 255 && z_pos >= 295 && z_pos <= 305 && coin3) {
		coin3 = false;
		coinsCollected++;
		score++;
		std::cout << coinsCollected << std::endl;
	}
	else if (x_pos >= 365 && x_pos <= 375 && z_pos >= -185 && z_pos <= -175 && coin4) {
		coin4 = false;
		coinsCollected++;
		score++;
		std::cout << coinsCollected << std::endl;
	}
	else if (x_pos >= 365 && x_pos <= 375 && z_pos >= 75 && z_pos <= 85 && coin5) {
		coin5 = false;
		coinsCollected++;
		score++;
		std::cout << coinsCollected << std::endl;
	}
}

void updateSwords(int x_pos, int z_pos) {
	if (x_pos == -120 && z_pos == -90) {
		sword1 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -200 && z_pos == -90) {
		sword2 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -300 && z_pos == -90) {
		sword3 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -120 && z_pos == 0) {
		sword4 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -200 && z_pos == 0) {
		sword5 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -300 && z_pos == 0) {
		sword6 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -120 && z_pos == 110) {
		sword7 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -200 && z_pos == 110) {
		sword8 = false;
		swordsCollected++;
		score++;
	}
	else if (x_pos == -300 && z_pos == 110) {
		sword9 = false;
		swordsCollected++;
		score++;
	}
}

void renderInt(float x, float y, float z, void* font, int number) {
	glRasterPos3f(x, y, z);

	// Convert the integer to a string
	char buffer[50]; // Assuming the number won't exceed 50 characters
	snprintf(buffer, sizeof(buffer), "%d", number);

	// Loop through the characters in the string and render them
	for (char* c = buffer; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

void renderString(float x, float y, float z, void* font, const char* string) {
	glRasterPos3f(x, y, z);

	for (const char* c = string; *c != '\0'; ++c) {
		glutBitmapCharacter(font, *c);
	}
}




//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gluOrtho2D(0, WIDTH, 0, HEIGHT); // Set up an orthographic projection

	setupCamera();
	if (level1) {
		lightposition += 1.0f;
		if (lightposition >= 50) {
			lightposition = -50;
		}
		GLfloat lightPosition[] = { 0.0f, 100.0f,lightposition, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		updateLightingIntensity();
	}
	else {
		lightposition += 0.1f;
		if (lightposition >= 50) {
			lightposition = -50;
		}
		GLfloat lightPosition[] = { lightposition, 100.0f,0.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		updateLightingIntensity();

	}

	if (level1) {
		// Draw Ground
		glPushMatrix();
		glScalef(2, 2, 3);
		RenderGround();
		glPopMatrix();
	}
	else {
		// Draw Ground
		glPushMatrix();
		glScalef(2, 2, 3);
		RenderGround1();
		glPopMatrix();
	}

	if (!level1) {
		glPushMatrix();
		glTranslatef(7, 0, 0);
		glPushMatrix();
		glScalef(2, 2, 2);
		glTranslatef(2, 0, 0);
		// Draw house Model
		glPushMatrix();
		glScalef(1, 5, 1);
		glTranslatef(-10, 0, 0);
		glRotatef(90.f, 0, 1, 0);
		model_castle.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.02, 0.02, 0.02);
		glScalef(1, 3, 1);
		glTranslatef(-820, 30, -450);
		glRotatef(90.f, 0, 1, 0);
		model_tower.Draw();
		glPopMatrix();


		glPopMatrix();

		glPushMatrix();
		glScalef(0.04, 0.04, 0.04);
		glTranslatef(-688, 1000, -400);
		glRotatef(90, 0, 1, 0);
		model_girl.Draw();
		glPopMatrix();

		if (sword1) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-90, 100, -10);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword2) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-110, 150, -12);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword3) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-120, 200, -14);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword4) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-130, 250, -16);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword5) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-140, 300, -18);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword6) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-150, 350, -20);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword7) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-160, 400, -22);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword8) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-165, 450, -24);
			model_sword.Draw();
			glPopMatrix();
		}

		if (sword9) {
			glPushMatrix();
			glScalef(0.1, 0.05, 0.3);
			glTranslatef(-170, 500, -26);
			model_sword.Draw();
			glPopMatrix();
		}


		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-350, 170, 0);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();


		checkRockCollision_1();
		checkRockCollision_2();
		checkRockCollision_3();
		checkRockCollision_4();
		checkRockCollision_5();
		checkRockCollision_6();
		checkRockCollision_7();
		checkRockCollision_8();
		checkRockCollision_9();
		checkRockCollision_10();
		checkRockCollision_11();
		checkRockCollision_12();
		checkRockCollision_13();
		checkRockCollision_14();

		//std::cout << "Knight's Position - X: " << knightX << ", Y: " << knightY << ", Z: " << knightZ << std::endl;

		if (knightX == -350 && knightY == 510 && knightZ == -140) {
			knightX = -350;
			knightY = 700;
			knightZ = -170;
			knightR = -90;
			win = true;
		}

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-390, 300, -50);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-430, 430, -100);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-470, 560, -150);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-510, 690, -200);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-550, 820, -250);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-590, 950, -300);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-630, 1080, -350);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-670, 1210, -400);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-710, 1340, -450);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-750, 1470, -500);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-790, 1600, -550);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-830, 1730, -600);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-870, 1860, -650);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

		glPushMatrix();
		glScalef(0.03, 0.02, 0.02);
		glTranslatef(-910, 1990, -650);
		glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
		model_rock.Draw();
		glPopMatrix();

	}


	glPushMatrix();
	glScalef(0.08, 0.08, 0.08);
	glTranslatef(knightX, knightY, knightZ);
	glRotatef(90, 1, 0, 0);
	glRotatef(knightR, 0, 0, 1);
	model_knight.Draw();
	glPopMatrix();



	if (level1) {

		glPushMatrix();
		glScalef(0.05, 0.03, 0.06);
		glTranslatef(500, -10, 0);
		glRotatef(270.f, 0, 1, 0);
		model_maze.Draw();
		glPopMatrix();


		if (!haveKey) {
			glPushMatrix();
			glScalef(1, 1, 1);
			glTranslatef(30, 3, 0);
			model_key.Draw();
			glPopMatrix();
		}

		glPushMatrix();
		glTranslatef(25, 4, 0);
		glScalef(5, 5, 5);
		model_wall.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(25, 4, -50);
		glScalef(5, 5, 5);
		model_wall.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(25, 4, 50);
		glScalef(5, 5, 5);
		model_wall.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(50, 4, 35);
		glScalef(5, 5, 5);
		model_wall.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(50, 4, -35);
		glScalef(5, 5, 5);
		model_wall.Draw();
		glPopMatrix();

		if (coin1) {
			glPushMatrix();
			glTranslatef(20, 2, -1);
			model_coin.Draw();
			glPopMatrix();
		}

		if (coin2) {
			glPushMatrix();
			glTranslatef(20, 2, -25);
			model_coin.Draw();
			glPopMatrix();
		}

		if (coin3) {
			glPushMatrix();
			glTranslatef(20, 2, 25);
			model_coin.Draw();
			glPopMatrix();
		}

		if (coin4) {
			glPushMatrix();
			glTranslatef(30, 2, -15);
			model_coin.Draw();
			glPopMatrix();
		}

		if (coin5) {
			glPushMatrix();
			glTranslatef(30, 2, 5);
			model_coin.Draw();
			glPopMatrix();
		}
	}

	glPopMatrix();

	// Set up for text rendering
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT); // Set up an orthographic projection
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Render the score
	glColor3f(1.0f, 1.0f, 1.0f); // Set text color
	if (level1) {
		score = coinsCollected;
	}
	else {
		score = swordsCollected;
	}
	renderInt(10, HEIGHT - 20, 0, font, score); // Adjust the position as needed

	// Restore previous OpenGL state
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	if (level1Text) {
		// Set up for text rendering
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 0, HEIGHT); // Set up an orthographic projection
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Render the score
		glColor3f(1.0f, 1.0f, 1.0f); // Set text color
		renderString(50, HEIGHT - 20, 0, font, "YOU PASSED LEVEL 1"); // Adjust the position as needed

		// Restore previous OpenGL state
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		level1Text = false;
	}

	if (win) {
		// Set up for text rendering
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, WIDTH, 0, HEIGHT); // Set up an orthographic projection
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Render the score
		glColor3f(1.0f, 1.0f, 1.0f); // Set text color
		renderString(50, HEIGHT - 20, 0, font, "YOU WIN!!"); // Adjust the position as needed

		// Restore previous OpenGL state
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}



	//sky box
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


	glPopMatrix();



	glutSwapBuffers();
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	float d = 0.9;

	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'k':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 27:
		exit(0);
		break;
	case 't': //top
		camera.setTopView();
		break;
	case 'f': //front
		camera.setFrontView();
		break;
	case 's': //stairs
		knightYPrev = knightY;
		if (!jumpStairs) {
			jumpStairs = true;
		}

		break;
	case 'u':
		camera.moveY(d);
		if (!win) {
			if (knightZ <= -1050) {
				knightZ = -1050;
			}
			else if (knightX <= -50 && knightX >= -350 && knightZ == -150) {
				knightZ = -150;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 40 && knightZ >= -40) {
				knightZ = 70;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= -560 && knightZ >= -640) {
				knightZ = -530;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 640 && knightZ >= 560) {
				knightZ = 670;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= 540 && knightZ >= 460) {
				knightZ = 570;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= -460 && knightZ >= -540) {
				knightZ = -430;
			}
			else {
				knightZ -= 10;
			}
			if (knightX >= 350 && knightX <= 400 && knightZ >= -20 && knightZ <= 20) {
				haveKey = true;
			}
			knightR = 180;
			PlaySound("sound/slide.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
		break;
	case 'd':
		camera.moveY(-d);
		if (!win) {
			if (knightZ >= 1050) {
				knightZ = 1050;
			}
			else if (knightX <= -50 && knightX >= -350 && knightZ == 150) {
				knightZ = 150;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 40 && knightZ >= -40) {
				knightZ = -70;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= -560 && knightZ >= -640) {
				knightZ = -670;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 640 && knightZ >= 560) {
				knightZ = 530;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= 540 && knightZ >= 460) {
				knightZ = 430;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= -460 && knightZ >= -540) {
				knightZ = -570;
			}
			else {
				knightZ += 10;
			}
			if (knightX >= 350 && knightX <= 400 && knightZ >= -20 && knightZ <= 20) {
				haveKey = true;
			}
			knightR = 0;
			PlaySound("sound/slide.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
		break;
	case 'l':
		camera.moveX(d);
		if (!win) {
			if (knightX == -20 && !haveKey) {
				knightX = -10;
			}
			else if (knightZ <= 150 && knightZ >= -150 && knightX == -350) {
				knightX = -350;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 40 && knightZ >= -40) {
				knightX = 380;
				knightZ = 0;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= -560 && knightZ >= -640) {
				knightX = 350;
				knightZ = -600;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 640 && knightZ >= 560) {
				knightX = 350;
				knightZ = 600;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= 540 && knightZ >= 460) {
				knightX = 680;
				knightZ = 500;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= -460 && knightZ >= -540) {
				knightX = 680;
				knightZ = -500;
			}
			else {
				knightX -= 10;
			}
			if (knightX >= 350 && knightX <= 400 && knightZ >= -20 && knightZ <= 20) {
				haveKey = true;
			}
			if (knightX <= -20 && haveKey) {
				level1 = false;
				level1Text = true;
			}
			knightR = 90;
			PlaySound("sound/slide.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
		break;
	case 'r':
		camera.moveX(-d);
		if (!win) {
			if (knightX >= 670) {
				knightX = 670;
			}
			else if (knightZ <= 150 && knightZ >= -150 && knightX == -50) {
				knightX = -50;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 40 && knightZ >= -40) {
				knightX = 250;
				knightZ = 0;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= -560 && knightZ >= -640) {
				knightX = 250;
				knightZ = -600;
			}
			else if (knightX <= 350 && knightX >= 250 && knightZ <= 640 && knightZ >= 560) {
				knightX = 250;
				knightZ = 600;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= 540 && knightZ >= 460) {
				knightX = 580;
				knightZ = 500;
			}
			else if (knightX <= 680 && knightX >= 580 && knightZ <= -460 && knightZ >= -540) {
				knightX = 580;
				knightZ = -500;
			}
			else {
				knightX += 10;
			}
			if (knightX >= 350 && knightX <= 400 && knightZ >= -20 && knightZ <= 20) {
				haveKey = true;
			}
			knightR = -90;
		}
		PlaySound("sound/slide.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;
	case 'c':
		camera.moveZ(d);
		break;
	case 'b':
		camera.moveZ(-d);
		break;
	case 'j':
		if (!isJumping) {
			isJumping = true;
		}
		if (knightR == -90) {
			camera.moveX(-10);
		}
		else if (knightR == 90) {
			camera.moveX(10);
		}
		else if (knightR == 180) {
			camera.moveZ(10);
		}
		else {
			camera.moveZ(-10);
		}
		PlaySound("sound/jump.wav", NULL, SND_FILENAME | SND_ASYNC);
		break;
	default:
		break;
	}
	setupCamera();
	updateCoins(knightX, knightZ);
	updateSwords(knightX, knightZ);
	glutPostRedisplay();
}

void Special(int key, int x, int y) {
	float a = 2.0;

	switch (key) {
	case GLUT_KEY_UP:
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN:
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT:
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT:
		camera.rotateY(-a);
		break;
	}

	glutPostRedisplay();
}


//=======================================================================
// Motion Function
//=======================================================================
//void myMotion(int x, int y)
//{
//	y = HEIGHT - y;
//
//	if (cameraZoom - y > 0)
//	{
//		Eye.x += -0.1;
//		Eye.z += -0.1;
//	}
//	else
//	{
//		Eye.x += 0.1;
//		Eye.z += 0.1;
//	}
//
//	cameraZoom = y;
//
//	glLoadIdentity();	//Clear Model_View Matrix
//
//	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters
//
//	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
//	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
//
//	glutPostRedisplay();	//Re-draw scene 
//}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y) {
	float d = 0.5;

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			if (!isJumping) {
				isJumping = true;
			}
			PlaySound("sound/jump.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
		else if (button == GLUT_RIGHT_BUTTON) {
			if (cameraP) {
				camera.setFrontView();
				cameraP = false;

			}
			else {
				camera.setTopView();
				cameraP = true;
			}
		}
	}
}

//=======================================================================
// Reshape Function
//=======================================================================
//void myReshape(int w, int h)
//{
//	if (h == 0) {
//		h = 1;
//	}
//
//	WIDTH = w;
//	HEIGHT = h;
//
//	// set the drawable region of the window
//	glViewport(0, 0, w, h);
//
//	// set up the projection matrix 
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);
//
//	// go back to modelview matrix so we can move the objects about
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
//}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_castle.Load("Models/castle/Castle.3ds");
	model_tower.Load("Models/tower/tower.3ds");
	model_maze.Load("Models/maze/maze.3ds");
	model_knight.Load("Models/knight/Knight.3ds");
	model_girl.Load("Models/girl/girl.3ds");
	model_key.Load("Models/key/key.3ds");
	model_wall.Load("Models/wall/wall.3ds");
	model_coin.Load("Models/coin/coin.3ds");
	model_sword.Load("Models/sword/sword.3ds");
	model_rock.Load("Models/rock/rock.3ds");

	// Loading texture files
	tex_ground.Load("Textures/newgrass.bmp");
	tex_ground1.Load("Textures/sand.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

void update(int value) {
	updateKnight();
	checkRockCollision_1();
	glutPostRedisplay();
	glutTimerFunc(16, update, 0); // Adjust the timer interval as needed
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);
	glutSpecialFunc(Special);
	glutMouseFunc(myMouse);

	myInit();


	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
	glutTimerFunc(25, update, 0); // Start the update timer
	glutMainLoop();
}