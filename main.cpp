#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <cstring>

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

using namespace std;

//booleans to determine different attributes
bool wireForm = true;
bool solidForm = false;
bool flatShading=true;
bool drawDouble=false;
bool quadStrip=false;
string alg="circles";

//determine quad or tirangle positions
int pos[4][2] = {{0,0}, {0,-1}, {-1,-1}, {-1,0}};

float heightMap[1000][1000];
int xSize, zSize;
float minHeight, maxHeight;
int numIterations;

float camPos[] = {100, 130, 100};

//different size terrains
char randArray[6] = {'a','b','c','d','e','f'};

void resetHeightMap()
{
	memset(heightMap, 0, sizeof(heightMap[0][0]) * 1000 * 1000);
}

void calcMinMax()			//get min and max heights from height map in order to compute gradients!
{
	maxHeight=0;
	minHeight=heightMap[0][0];
	for(int x=1; x<xSize; x++)
	{
		for(int z=1;z<zSize; z++)
		{
			if(heightMap[x][z]>maxHeight)
			{
				maxHeight=heightMap[x][z];
			}
			if(heightMap[x][z]<minHeight)
			{
				minHeight=heightMap[x][z];
			}
		}
	}
}

float midpoint(float v1, float v2) //determine loc
{
	return ((v1 + v2)/2);
}

float average2(float v1, float v2) //determine heights
{
	return ((v1 + v2)/2);
}

float average4(float v1, float v2, float v3, float v4) //determine height for center
{
	return ((v1 + v2 + v3 + v4)/4);
}

float jitter(float v) {  //randomize heights a bit
	v += rand()%5;
}

void initCorners()            //MDP Alg --- initailize heights of four corners randomly
{
	heightMap[0][xSize]=rand()%10;
	heightMap[0][zSize]=rand()%10;
	heightMap[xSize][0]=rand()%10;
	heightMap[zSize][0]=rand()%10;
}

void MPDAlgorithmDisplace(int lx,int rx,int by,int ty)  //get heights for points in midpoints
{
	int cx = lx + rx/2;
	int cy = by + ty/2;
	
	float bl = heightMap[lx][by]; //bottom left point height
	float tl = heightMap[lx][ty]; //top left point height
	float br = heightMap[rx][by]; //bottom right point height
	float tr = heightMap[rx][ty]; //top right point height

	float top = average2(tl,tr);  
	float left = average2(bl,tl);
	float right = average2(br,tr);
	float bottom = average2(br,bl);
	float center = average4(top,left,bottom,right);

	heightMap[cx][by]=jitter(bottom); //determine h for midpoints 
	heightMap[cx][ty]=jitter(top);
	heightMap[lx][cy]=jitter(left);
	heightMap[rx][cy]=jitter(right);
	heightMap[cx][cy]=jitter(center);
}

void MPDAlgorithm()  //entry point to MDP Alg
{	int exp=0;
	while(pow(2,exp)<=xSize)       //get num iterations based on size
	{
		exp++;
	}
	initCorners();   
	for(int i=0; i<exp; i++) {
		int chunks=pow(2,i);
		int chunkWidth = (xSize/chunks);
		for(int x=0; x<chunks;x++) {
			for(int z=0;z<chunks;z++) {
				float leftX=(chunkWidth*x);
				float rightX=(leftX+chunkWidth);
				float bottomY=(chunkWidth*z);
				float topY=(bottomY*chunkWidth);
				MPDAlgorithmDisplace(leftX,rightX,bottomY,topY);  //entry to algorithm
			}
		}
	}
	calcMinMax();
}

void faultAlgorithm(int iterations) 
{
	for(int i=0; i<iterations; i++){
        float v = static_cast <float> (rand());          
        float a = sin(v);                             
        float b = cos(v);                                 
        float d = sqrt(pow(xSize,2) + pow(zSize,2));   
        float c = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * d - d/2;
        for(int x = 0; x<xSize; x++){
            for(int z=0; z<zSize; z++){
                if (a*x + b*z - c > 0){                   
                    heightMap[x][z] += 1;
                }
                else{
                    heightMap[x][z] -= 1;                
                }
            }
        }
    }
	calcMinMax();
}

void circlesAlgorithm(int iterations)
{
	for(int i=0; i<iterations; i++)
	{
		int ccx = rand()%xSize;                       
		int ccz = rand()%zSize;                         
		int radius = (rand()%20)+8;                          
		float disp = rand()%10;                                 
		for(int x = 0; x<xSize; x++){                      
		    for(int z = 0; z<zSize; z++){                     
			float distance = sqrtf(pow(x-ccx,2) + pow(z-ccz,2));  
			float pd = (distance*2)/radius;      
			if (fabs(pd) <= 1.0){                       
			    heightMap[x][z] += (disp/2.0) + (cos(pd*3.14)*(disp/2.0)); 
			}
		    }
		}
	}
	calcMinMax();
}
/*Actually draws quads or triangle and determines colors
 */
void drawTerrain()
{
	int newX,newZ;
	for(int x=0; x<xSize; x++)
	{
		for(int z=0; z<zSize; z++)
		{
			int storeX1=x-pos[2][0];  //store vars used to get last two verts
			int storeZ1=z-pos[2][1];
			int storeX2=x-pos[3][0];
			int storeZ2=z-pos[3][1];
			if (quadStrip) {
	    			glBegin(GL_QUAD_STRIP);
			} else {
	    			glBegin(GL_TRIANGLE_STRIP);
			}
				for(int i=3;i>=0;i--)
				{
					if(i==0) {
						newX=x-pos[i][0];
						newZ=z-pos[i][1];
						storeX1=newX;
						storeZ1=newZ;
					}
					else if(i==1) {
						newX=x-pos[i][0];
						newZ=z-pos[i][1];
						storeX2=newX;
						storeZ2=newZ;
					} else if(i==2) {
						newX=storeX2;
						newZ=storeZ2;
					} else {
						newX=storeX1;
						newZ=storeZ1;
					}
					float newY=heightMap[newX][newZ];
					float gradient = (newY - minHeight)/(maxHeight-minHeight);
					if(gradient<=0.3) {
						glColor3f(0, gradient, 0); //green
					} 
					else if(gradient<=0.5) {
						glColor3f(1, gradient, 0); //orange
					} 
					else if(gradient<=0.8) {
						glColor3f(gradient, 0, 0);  //red
					} 
					else {
						glColor3f(1,1,1); //white
					}
					glVertex3f(newX, newY, newZ); //draw
				}
			glEnd();
            	}
        }

	if(drawDouble) //to get wire frame for wire+solid mode
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for(int x=0; x<xSize; x++)
		{
			for(int z=0; z<zSize; z++)
			{
				int storeX1=x-pos[2][0];
				int storeZ1=z-pos[2][1];
				int storeX2=x-pos[3][0];
				int storeZ2=z-pos[3][1];
				if (quadStrip) {
					glBegin(GL_QUAD_STRIP);
				} else {
					glBegin(GL_TRIANGLE_STRIP);
				}
				for(int i=3;i>=0;i--)
				{
					if(i==0) {
						newX=x-pos[i][0];
						newZ=z-pos[i][1];
						storeX1=newX;
						storeZ1=newZ;
					}
					else if(i==1) {
						newX=x-pos[i][0];
						newZ=z-pos[i][1];
						storeX2=newX;
						storeZ2=newZ;
					} else if(i==2) {
						newX=storeX2;
						newZ=storeZ2;
					} else {
						newX=storeX1;
						newZ=storeZ1;
					}
					float newY=heightMap[newX][newZ];
					glColor3f(0, 0, 0);
					glVertex3f(newX, newY, newZ);
				}
				glEnd();
			}
		}
	}
	drawDouble=false;
}

/*
 *Control Camera
 */
void special(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_LEFT:
            camPos[2] +=  2;
            break;
        case GLUT_KEY_RIGHT:
            camPos[2] -= 2;
            break;
        case GLUT_KEY_UP:
            camPos[1] += 2;
            break;
        case GLUT_KEY_DOWN:
            camPos[1] -= 2;
            break;
   }
    glutPostRedisplay();
}

void menu(int key) 
{
    switch(key) 
    {
        case 'a':
            xSize = 50;					 
            zSize = 50;
            break;
        case 'b':
            xSize = 100;
            zSize = 100;
            break;
        case 'c':
            xSize = 150;
            zSize = 150;
            break;
        case 'd':
            xSize = 200;
            zSize = 200;
            break;
        case 'e':
            xSize = 250;
            zSize = 250;
            break;
        case 'f':
            xSize = 300;
            zSize = 300;
            break;
    }
    //redisplays terrain and determines alg used
    resetHeightMap();
    numIterations = (xSize+zSize)*2;
    if(alg=="circles") { 
    	circlesAlgorithm(numIterations);  
    } else if(alg=="fault") {
	faultAlgorithm(numIterations);
    } else if(alg=="MPD") {
	MPDAlgorithm();
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'q':
            exit (0);
	    break;
        case 'w':
	    if (wireForm and !solidForm) 
	    {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		wireForm=false;
		solidForm=true;
	    } else if(!wireForm and solidForm) {
		wireForm=true;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	    } else if(wireForm and solidForm) {
		drawDouble=true;
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		wireForm=true;
		solidForm=false;
	    }
	    break;
	case 's':
	    if(flatShading)
	    {
	      glShadeModel(GL_FLAT);
	      flatShading=false;
	    } else {
	      glShadeModel(GL_SMOOTH);
	      flatShading=true;
	    }
	    break;
	case 'r':
            menu(randArray[rand()%6]);                       
	    break;
	case 'y':
	    quadStrip=true;
	    break;
	case 't':
	    quadStrip=false;
	    break;
	case 'a':
	    if(alg=="circles") {
		alg="fault";	
	    } else if(alg=="fault") {
		alg="MPD";
	    } else if(alg=="MPD") {
		alg="circles";
            }
    }
    glutPostRedisplay();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camPos[0], camPos[1], camPos[2], 0,0,0, 0,1,0);
    drawTerrain();
    glutSwapBuffers();
}

void init(void) 
{
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 400);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
}

void CallBackInit()
{
    glutDisplayFunc(display);	
    glutKeyboardFunc(keyboard); 
    glutSpecialFunc(special);  
}

void instr()
{
	cout << "==============================WELCOME TO OPENGL TERRAIN!===============================" << endl;
	cout << "--------BEGIN BY RIGHT CLICKING TO ACCESS THE MENU!------------------------------------" << endl;
	cout << "Press a for different terrain algorithm: (rebuild terrain to see difference):----------" << endl;
	cout << "1) CirclesAlgorithm (default)----------------------------------------------------------" << endl;
	cout << "2) FaultAlgorithm ---------------------------------------------------------------------" << endl;
	cout << "2) MPDAlgorithm -----------------------------------------------------------------------" << endl;
	cout << "Press w to toggle wireframe mode:------------------------------------------------------" << endl;
	cout << "1) Solid frame ------------------------------------------------------------------------" << endl;
	cout << "2) Wireframe --------------------------------------------------------------------------" << endl;
	cout << "3) Solid and Wireframe mode-------------------------------------------------------------" << endl;
	//cout << "Press l to toggle lighting:------------------------------------------------------------" << endl;
	cout << "Press r to get random terrain:---------------------------------------------------------" << endl;
	cout << "Press s to toggle shading:-------------------------------------------------------------" << endl;
	cout << "1) Flat shading -----------------------------------------------------------------------" << endl;
	cout << "2) Gouraud shading --------------------------------------------------------------------" << endl;
	cout << "Press y to draw in quad strips:--------------------------------------------------------" << endl;
	cout << "Press t to draw in triangle strips:----------------------------------------------------" << endl;

}

int main(int argc, char ** argv) 
{
    glutInit(&argc, argv);		
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800); 
    glutInitWindowPosition(100, 100); 
    glutCreateWindow("Terrain");	
    CallBackInit();
    init();                         
    glutCreateMenu(menu);
    instr();
    glutAddMenuEntry("50 x 50", 'a');
    glutAddMenuEntry("100 x 100", 'b');
    glutAddMenuEntry("150 x 150", 'c');
    glutAddMenuEntry("200 x 200", 'd');
    glutAddMenuEntry("250 x 250", 'e');
    glutAddMenuEntry("300 x 300", 'f');
    glutAttachMenu(GLUT_RIGHT_BUTTON); 
    glutMainLoop();			
    return(0);					
}
