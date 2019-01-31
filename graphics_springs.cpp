// CS 3600 Graphics Programming
// Spring, 2002
// Program #4 - SpringMan
// This program simulates various forces between particles, such as springs.

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream> 
#include <vector>
#include <map>
#include <sstream>
using namespace std;
#include "glut.h"
#include "graphics.h"
#include "particle.h"
#include "forces.h"


// Global Variables
// Some colors you can use, or make your own and add them
// here and in graphics.h
GLdouble redMaterial[] = {0.7, 0.1, 0.2, 1.0};
GLdouble greenMaterial[] = {0.1, 0.7, 0.4, 1.0};
GLdouble brightGreenMaterial[] = {0.1, 0.9, 0.1, 1.0};
GLdouble blueMaterial[] = {0.1, 0.2, 0.7, 1.0};
GLdouble whiteMaterial[] = {1.0, 1.0, 1.0, 1.0};

double screen_x = 700;
double screen_y = 500;

// The particle system.
ParticleSystem PS;
vector<string> my_particles;
vector<string> my_forces;

double my_df = 0;
string my_gravity;
// 
// Functions that draw basic primitives
//
void DrawCircle(double x1, double y1, double radius)
{
	glBegin(GL_POLYGON);
	for(int i=0; i<32; i++)
	{
		double theta = (double)i/32.0 * 2.0 * 3.1415926;
		double x = x1 + radius * cos(theta);
		double y = y1 + radius * sin(theta);
		glVertex2d(x, y);
	}
	glEnd();
}

void DrawLine(double x1, double y1, double x2, double y2)
{
	glBegin(GL_LINES);
	glVertex2d(x1,y1);
	glVertex2d(x2,y2);
	glEnd();
}

void DrawRectangle(double x1, double y1, double x2, double y2)
{
	glBegin(GL_QUADS);
	glVertex2d(x1,y1);
	glVertex2d(x2,y1);
	glVertex2d(x2,y2);
	glVertex2d(x1,y2);
	glEnd();
}

void DrawTriangle(double x1, double y1, double x2, double y2, double x3, double y3)
{
	glBegin(GL_TRIANGLES);
	glVertex2d(x1,y1);
	glVertex2d(x2,y2);
	glVertex2d(x3,y3);
	glEnd();
}

// Outputs a string of text at the specified location.
void text_output(double x, double y, char *string)
{
	void *font = GLUT_BITMAP_9_BY_15;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
	
	int len, i;
	glRasterPos2d(x, y);
	len = (int) strlen(string);
	for (i = 0; i < len; i++) 
	{
		glutBitmapCharacter(font, string[i]);
	}

    glDisable(GL_BLEND);
}


//
// GLUT callback functions
//

// This callback function gets called by the Glut
// system whenever it decides things need to be redrawn.
void display(void)
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3dv(whiteMaterial);
	
	//EulerStep(PS, DeltaT);
	//MidpointStep(PS, DeltaT);
	RungeKuttaStep(PS, PS.GetDeltaT());

	int N = PS.GetNumParticles();
	int NF = PS.GetNumForces();

	// Check Resulting particles for wall collisions
	for(i=0; i<N; i++)
	{
		Particle * p = PS.GetParticle(i);
		double radius = p->GetRadius();
		double x = p->GetPositionx();
		double y = p->GetPositiony();
		double xDir = p->GetDirectionx();
		double yDir = p->GetDirectiony();

		// bounce off left wall
		if(x - radius < 0)
		{
			p->SetPositionx(radius);
			p->SetDirectionx(fabs(xDir));
		}

		// bounce off right wall
		if(x + radius > screen_x)
		{
			p->SetPositionx(screen_x - radius);
			p->SetDirectionx(-fabs(xDir));
		}

		// bounce off bottom wall
		if(y - radius < 0)
		{
			p->SetPositiony(radius);
			p->SetDirectiony(fabs(yDir));
		}

		// bounce off top wall
		if(y + radius > screen_y)
		{
			p->SetPositiony(screen_y - radius);
			p->SetDirectiony(-fabs(yDir));
		}
	}


	// Draw Spring Forces as edges
	for(i=0; i<NF; i++)
	{
		Force * f = PS.GetForce(i);
		if(f->Type() == SPRING_FORCE)
		{
			SpringForce * sf = (SpringForce*)f;
			Particle * p1 = sf->GetParticle1();
			Particle * p2 = sf->GetParticle2();
			glColor3dv(greenMaterial);
			DrawLine(p1->GetPositionx(), p1->GetPositiony(),  p2->GetPositionx(), p2->GetPositiony());
		}
	}

	// Draw Particles
	for(i=0; i<N; i++)
	{
		Particle * p = PS.GetParticle(i);
		double radius = p->GetRadius();

		double thePos[DIM];
		p->GetPosition(thePos);
		if(p->GetAnchored())
			glColor3dv(redMaterial);
		else
			glColor3dv(whiteMaterial);
		DrawCircle(thePos[0], thePos[1], radius);
	}

	glutSwapBuffers();
	glutPostRedisplay();
}


// This callback function gets called by the Glut
// system whenever a key is pressed.
void keyboard(unsigned char c, int x, int y)
{
	switch (c) 
	{
		case 27: // escape character means to quit the program
			exit(0);
			break;
		case 'b':
			// do something when 'b' character is hit.
			break;
		default:
			return; // if we don't care, return without glutPostRedisplay()
	}

	glutPostRedisplay();
}


// This callback function gets called by the Glut
// system whenever the window is resized by the user.
void reshape(int w, int h)
{
	screen_x = w;
	screen_y = h;

	// Set the pixel resolution of the final picture (Screen coordinates).
	glViewport(0, 0, w, h);

	// go into 2D mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the world coordinates.
	gluOrtho2D(0, w, 0, h);
	glMatrixMode(GL_MODELVIEW);

}

// This callback function gets called by the Glut
// system whenever any mouse button goes up or down.
void mouse(int mouse_button, int state, int x, int y)
{
	if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) 
	{
	}
	if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_UP) 
	{
	}
	if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) 
	{
	}
	if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_UP) 
	{
	}
	glutPostRedisplay();
}
/*Creating the particles*/
void InputParticles()
{
	
	//now my_paricles, my_forces and my_gravity have a string that needs to be split and inserted into the correct functions.
	map<string, Particle *> assigned_p;

	//set up test for after splitting string to make sure you have the minimum required arguments
	for (unsigned int i = 0; i < my_particles.size(); i++){
		//split the string and make a new particle with the values
		//stod is a great built in way for converting strings to doubles.
		//place the string into the map using key, value something like map<p1, new Particle(100,200,4.0,-1.0,10,false)>
		string temp_s = my_particles[i];
		//stringstream ss(temp_s);
		//double x, double y, double xDir, double yDir, double r_temp, bool anchored_temp
		string temp_name;
		double temp_x, temp_y, temp_xDir, temp_yDir, temp_r_temp;
		bool temp_anchored_temp;
		
		stringstream ss(temp_s);
		vector<string> result;

		while( ss.good() )
		{
			string substr;
			getline( ss, substr, ',' );
			result.push_back( substr );
		}
		temp_name = result[0];
		temp_x = stod(result[1]);
		temp_y = stod(result[2]);
		temp_xDir = stod(result[3]);
		temp_yDir = stod(result[4]);
		temp_r_temp = stod(result[5]);
		
		if (result[6].compare("false")==0){
			temp_anchored_temp = false;
		} else if (result[6].compare("true")==0){
			temp_anchored_temp = true;
		} else {
			cout << "broken statement in particles... now exiing" << endl;
			exit(0);
			
		}
		Particle *p = new Particle(temp_x, temp_y, temp_xDir, temp_yDir, temp_r_temp, temp_anchored_temp);
		PS.AddParticle(p);
		assigned_p[temp_name] = p;
	}

	
	for (unsigned int f = 0; f < my_forces.size(); f++){
		string point1, point2;
		double f_spring_constant, f_damping_constant, f_rest_length;

		string temp_f = my_forces[f];
		stringstream ss(temp_f);
		vector<string> result;

		while( ss.good() )
		{
			string substr;
			getline( ss, substr, ',' );
			result.push_back( substr );
		}
		point1 = result[1];
		point2 = result[2];
		f_spring_constant = stod(result[3]);
		f_damping_constant = stod(result[4]);
		if (result.size()>5){
			f_rest_length = stod(result[5]);
			Force * F = new SpringForce(assigned_p[point1], assigned_p[point2], f_spring_constant, f_damping_constant, f_rest_length);
			PS.AddForce(F);
		} else {
			Force * F = new SpringForce(assigned_p[point1], assigned_p[point2], f_spring_constant, f_damping_constant);
			PS.AddForce(F);
		}
		
	}
	
	if (my_df != 0){
		Force * DF = new DragForce(my_df, &PS);
		PS.AddForce(DF);
	}

	double point1, point2;
		
	string temp_g = my_gravity;
	stringstream ss(temp_g);
	vector<string> result;
	while( ss.good() )
	{
		string substr;
		getline( ss, substr, ',' );
		result.push_back( substr );
	}
	point1 = stod(result[1]);
	point2 = stod(result[2]);
	if (result[0].compare("gravity")==0){
		cout << "gravity good..." << endl;
	}
	double gravity[DIM] = {point1, point2};
	Force * F2 = new GravityForce(gravity, &PS);
	PS.AddForce(F2);	

}


void InitParticles1()
{
	Particle *p1 = new Particle(100, 200,  4.0, -1.0,  10, false);
	PS.AddParticle(p1);

	Particle *p2 = new Particle(200, 200,  4.0, 0.0,  5, false);
	PS.AddParticle(p2);

	Particle *p3 = new Particle(20, 405,  5.0, 1.0,  5, false);
	PS.AddParticle(p3);

	Particle *p4 = new Particle(40, 410,  -1.0, -2.0,  5, false);
	PS.AddParticle(p4);

	Particle *p5 = new Particle(420, 405,  5.0, 1.0,  5, true);
	PS.AddParticle(p5);

	Particle *p6 = new Particle(440, 410,  -1.0, -2.0,  5, false);
	PS.AddParticle(p6);

	Particle *p7 = new Particle(490, 410,  -1.0, -2.0,  5, false);
	PS.AddParticle(p7);

	Force * F = new SpringForce(p1, p2, 4, 0.5, 80);
	PS.AddForce(F);

	Force * s2 = new SpringForce(p3, p4, .5, 0.1, 100);
	PS.AddForce(s2);

	Force * s3 = new SpringForce(p5, p6, .5, 0.1, 100);
	PS.AddForce(s3);

	Force * s4 = new SpringForce(p7, p6, .5, 0.1, 100);
	PS.AddForce(s4);

	Force * DF = new DragForce(.001, &PS);
	PS.AddForce(DF);

	double gravity[DIM] = {0.0, -0.0050};
	Force * F2 = new GravityForce(gravity, &PS);
	PS.AddForce(F2);

}

void InitParticles2()
{
	bool hanging = false;

	Particle *p1 = new Particle(100,300,0,0,2,hanging);
	PS.AddParticle(p1);

	Particle *p2 = new Particle(220,360,0,0,2,hanging);
	PS.AddParticle(p2);

	Particle *p3 = new Particle(215,300,0,0,2,false);
	PS.AddParticle(p3);

	Particle *p4 = new Particle(200,200,0,0,2,false);
	PS.AddParticle(p4);

	Particle *p5 = new Particle(200,105,0,0,2,false);
	PS.AddParticle(p5);

	Particle *p6 = new Particle(233,330,0,0,2,false);
	PS.AddParticle(p6);

	Particle *p7 = new Particle(250,380,0,0,2,hanging);
	PS.AddParticle(p7);

	Particle *p8 = new Particle(266,330,0,0,2,false);
	PS.AddParticle(p8);

	Particle *p9 = new Particle(280,360,0,0,2,hanging);
	PS.AddParticle(p9);

	Particle *p10 = new Particle(285,300,0,0,2,false);
	PS.AddParticle(p10);

	Particle *p11 = new Particle(300,200,0,0,2,false);
	PS.AddParticle(p11);

	Particle *p12 = new Particle(300,100,0,0,2,false);
	PS.AddParticle(p12);

	Particle *p13 = new Particle(400,300,0,0,2,hanging);
	PS.AddParticle(p13);

	double kSpringConstant = .2;
	double kDampingConstant = 1.5;

	PS.AddForce(new SpringForce(p1, p3, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p4, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p5, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p6, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p6, p2, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p2, p7, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p7, p9, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p9, p8, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p8, p10, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p10, p11, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p11, p12, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p10, p13, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p10, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p11, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p6, p8, kSpringConstant, kDampingConstant));

	PS.AddForce(new SpringForce(p1, p4, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p11, p13, kSpringConstant, kDampingConstant));
	for(int i=0;i<5;i++)
	{
	PS.AddForce(new SpringForce(p5, p12, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p5, p11, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p12, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p11, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p10, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p6, p10, kSpringConstant*2, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p8, kSpringConstant*2, kDampingConstant));
	PS.AddForce(new SpringForce(p2, p9, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p6, p7, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p7, p8, kSpringConstant, kDampingConstant));
	}

	double gravity[DIM] = {0.0, -.10};
	Force * F2 = new GravityForce(gravity, &PS);
	PS.AddForce(F2);
}

void InitParticles3()
{
	Particle *p1 = new Particle(200,150,0,0,2,false);
	PS.AddParticle(p1);

	Particle *p2 = new Particle(200,300,0,0,2,false);
	PS.AddParticle(p2);

	Particle *p3 = new Particle(300,300,0,0,2,false);
	PS.AddParticle(p3);

	Particle *p4 = new Particle(300,200,0,0,2,false);
	PS.AddParticle(p4);

	double kSpringConstant = 1;
	double kDampingConstant = 1.5;

	PS.AddForce(new SpringForce(p1, p2, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p2, p3, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p4, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p1, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p1, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p2, kSpringConstant, kDampingConstant));

	double gravity[DIM] = {0.0, -.30};
	Force * F2 = new GravityForce(gravity, &PS);
	PS.AddForce(F2);
}

void InitParticles4()
{
	Particle *p1 = new Particle(200,200,0,0,4,false);
	PS.AddParticle(p1);

	Particle *p2 = new Particle(200,300,0,0,4,true);
	PS.AddParticle(p2);

	Particle *p3 = new Particle(300,300,0,0,4,true);
	PS.AddParticle(p3);

	Particle *p4 = new Particle(300,200,0,0,4,false);
	PS.AddParticle(p4);

	double kSpringConstant = .8;
	double kDampingConstant = 0.1;

	PS.AddForce(new SpringForce(p1, p2, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p2, p3, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p4, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p4, p1, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p3, p1, kSpringConstant, kDampingConstant));
	PS.AddForce(new SpringForce(p2, p4, kSpringConstant, kDampingConstant));

	double gravity[DIM] = {0.0, -4.30};
	Force * F2 = new GravityForce(gravity, &PS);
	PS.AddForce(F2);

	Force * DF = new DragForce(.01, &PS);
	PS.AddForce(DF);
}


// Your initialization code goes here.
void InitializeMyStuff()
{
	InputParticles();
	
}


int main(int argc, char **argv)
{
	/* asking the user for an input data file*/
	char str[256];
	cout << '\n' << "Please insert file name and press enter to continue.. \n";
	std::cin.get (str,256);
	std::cout << str << std::endl;
	std::ifstream is(str);     // open file
	if (!is.is_open())
	{
		std::cout << "error in opening file " << str << std::endl;
		exit(0);
	}
	string c;
	/*parse through that file and create the particles and forces as the file directs*/
	while(std::getline(is, c))
	{
		std::string beg = c;
		if (beg.compare(0,1,"p") == 0)
		{
			std::cout << "reading p commands" << std::endl;
			my_particles.push_back(beg);
			std::cout << c << std::endl;
			cout << "true" << endl;
		} 
		if (beg.compare(0,2,"SF") == 0) {
			std::cout << "reading SF commands" << std::endl;
			my_forces.push_back(beg);
			std::cout << c << std::endl;
			cout << "true" << endl;
		}
		if (beg.compare(0,2,"DF") == 0) {
			std::cout << "reading DF commands" << std::endl;
			string amount = beg.substr(3,10);
			if (stod(amount)!=0){
				my_df = stod(amount);
			}
			std::cout << my_df << std::endl;
			cout << "true" << endl;
		}
		if (beg.compare(0,7,"gravity") == 0) {
			std::cout << "reading gravity" << std::endl;
			my_gravity = c;
			cout << my_gravity << endl;
			
		}
		if (beg.compare(0,6,"deltaT") == 0) {
			std::cout << "deltaT" << std::endl;
			string amount = beg.substr(7,10);
			if (stod(amount)!=0.1){
				PS.SetDeltaT(stod(amount));
			}
		}
		if (beg.compare(0,3,"red") == 0) {
			string red = beg.substr(4,100);
			//break apart the new string and set it to redmaterial
			stringstream ss(red);
			vector<string> result;
			while( ss.good() )
			{
				string substr;
				getline( ss, substr, ',' );
				result.push_back( substr );
			}
			redMaterial[0] = stod(result[0]);
			redMaterial[1] = stod(result[1]);
			redMaterial[2] = stod(result[2]);
			redMaterial[3] = stod(result[3]);
			std::cout << "testing red " << result[0] << " " << result[1] << " " << result[2] << " " << result[3] << endl;

		}
		if (beg.compare(0,3,"gre") == 0) {
			string green = beg.substr(4,100);
			//break apart the new string and set it to greenmaterial
			stringstream ss(green);
			vector<string> result;
			while( ss.good() )
			{
				string substr;
				getline( ss, substr, ',' );
				result.push_back( substr );
			}
			greenMaterial[0] = stod(result[0]);
			greenMaterial[1] = stod(result[1]);
			greenMaterial[2] = stod(result[2]);
			greenMaterial[3] = stod(result[3]);
		}
		if (beg.compare(0,3,"bri") == 0) {
			string bright = beg.substr(4,100);
			//break apart the new string and set it to brightmaterial
			stringstream ss(bright);
			vector<string> result;
			while( ss.good() )
			{
				string substr;
				getline( ss, substr, ',' );
				result.push_back( substr );
			}
			brightGreenMaterial[0] = stod(result[0]);
			brightGreenMaterial[1] = stod(result[1]);
			brightGreenMaterial[2] = stod(result[2]);
			brightGreenMaterial[3] = stod(result[3]);
		}
		if (beg.compare(0,3,"blu") == 0) {
			string blue = beg.substr(4,100);
			//break apart the new string and set it to bluematerial
			stringstream ss(blue);
			vector<string> result;
			while( ss.good() )
			{
				string substr;
				getline( ss, substr, ',' );
				result.push_back( substr );
			}
			blueMaterial[0] = stod(result[0]);
			blueMaterial[1] = stod(result[1]);
			blueMaterial[2] = stod(result[2]);
			blueMaterial[3] = stod(result[3]);
		}
		if (beg.compare(0,3,"whi") == 0) {
			string white = beg.substr(4,100);
			//break apart the new string and set it to whitematerial
			stringstream ss(white);
			vector<string> result;
			while( ss.good() )
			{
				string substr;
				getline( ss, substr, ',' );
				result.push_back( substr );
			}
			whiteMaterial[0] = stod(result[0]);
			whiteMaterial[1] = stod(result[1]);
			whiteMaterial[2] = stod(result[2]);
			whiteMaterial[3] = stod(result[3]);
		}
	}
	is.close();
	
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(screen_x, screen_y);
	glutInitWindowPosition(50, 50);

	int fullscreen = 0;
	if (fullscreen) 
	{
		glutGameModeString("800x600:32");
		glutEnterGameMode();
	} 
	else 
	{
		glutCreateWindow("Shapes");
	}

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);

	glClearColor(.3,.3,.3,0);	
	InitializeMyStuff();

	glutMainLoop();

	return 0;
}
