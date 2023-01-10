#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <deque>
using namespace std;
#include <GL/glut.h>
#include <GL/GLU.h>
#include <gl/GL.h>
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
int VECTOR_SIZE = 2;

#define UP     1
#define DOWN   2
#define LEFT   3
#define RIGHT  4

deque<pair<int, int>> snake_body;
cl_mem A_clmem ;
cl_mem B_clmem ;
cl_mem C_clmem ;
cl_mem D_clmem;
cl_mem E_clmem;


int* A = (int*)malloc(sizeof(int) * VECTOR_SIZE);
int* B = (int*)malloc(sizeof(int) * VECTOR_SIZE);
int* C = (int*)malloc(sizeof(int) * VECTOR_SIZE);
int* D = (int*)malloc(sizeof(int) * VECTOR_SIZE);
int* E = (int*)malloc(sizeof(int) * VECTOR_SIZE);

cl_command_queue command_queue;
cl_int clStatus;
cl_kernel kernel;
size_t global_size = 1024; // Process the entire lists
size_t local_size = 64;           // Process one item at a time

int direction = RIGHT;

int map_size = 20;    
//using namespace std;
//OpenCL kernel which is run for every work item created.
const char* saxpy_kernel =
"__kernel                                   \n"
"void saxpy_kernel(float alpha,     \n"
"                  __global float *A,       \n"
"                  __global float *B,       \n"
"                  __global float *C, __global float *D , __global float *E)       \n"
"{                                          \n"
"	       // delay(100);                       \n"
"    //Get the index of the work-item       \n"
"    int index = get_global_id(0);          \n"
"    C[0] = A[0] + B[0]; \n"
"    C[1] = D[0] + B[0]; \n"
"     	//glutTimerFunc(50, TimerFunc, 0);                     \n"

"}                                          \n";



void moveSnake(int dirc)
{
	direction = dirc;
	int delX = 0;
	int delY = 0;
	int mapEdge = 0;
	int snake_part_axis = 0;
	//delX = 1; // move by one 
	int temp3 = snake_body[0].first - 1;
	int temp4 = snake_body[0].second - 1;

	switch (direction)
	{
	case UP:
		delY = 1;
		break;
	case DOWN:
		delY = -1;
		break;
	case LEFT:
		delX = -1;
		break;
	case RIGHT:
		delX = 1;
		break;
	}
	int temp = snake_body[0].first + delX;
	int temp2 = snake_body[0].second + delY;
	if (temp >= map_size - 1 || temp <= 0 || temp2 >= map_size - 1 || temp2 <= 0) {
		exit(0);
	}
	snake_body.push_front({ temp ,snake_body[0].second + delY });
	snake_body.pop_back();
	glutPostRedisplay();
}


void keyboard(unsigned char key, int, int) {

	switch (key)
	{
	case 'W':
	case 'w':
	{
		if (direction == LEFT || direction == RIGHT) {
			moveSnake(UP);
		}

		break;
	}
	case 'S':
	case 's':
	{
		if (direction == LEFT || direction == RIGHT) {
			moveSnake(DOWN);
		}

		break;
	}

	case 'A':
	case 'a':
	{
		if (direction == UP || direction == DOWN) {
			moveSnake(LEFT);
		}
		break;
	}
	case 'D':
	case 'd':
	{
		if (direction == UP || direction == DOWN) {
			moveSnake(RIGHT);
		}
		break;
	}
	}
}

void TimerFunc(int val)
{
	moveSnake(direction);
	glutTimerFunc(100, TimerFunc, 0);
}

void displayBorderBlocks(int x, int y)
{
	glColor3f(0.87, 0, 0);
	glBegin(GL_POLYGON);
	glVertex3f(x + 0.95, y + 0.95, 1.0f);
	glVertex3f(x + 0.05, y + 0.95, 0.0f);
	glVertex3f(x + 0.05, y + 0.05, 0.0f);
	glVertex3f(x + 0.95, y + 0.05, 1.0f);
	glEnd();

	glLineWidth(2);
	glColor3f(1, 1, 1);
	glBegin(GL_LINES); 
	glVertex2f(x + 0.5, y + 1);
	glVertex2f(x + 0.5, y + 0);
	glEnd();
	glBegin(GL_LINES); 
	glVertex2f(x + 0, y + 0.5);
	glVertex2f(x + 1, y + 0.5);
	glEnd();
}
void displayBorders()
{

	for (int i = 0; i <= map_size; i++)
	{
		displayBorderBlocks(i, 0);          
		displayBorderBlocks(i, map_size - 1);   
		displayBorderBlocks(0, i);          
		displayBorderBlocks(map_size - 1, i);   

	}
}

void exKer(cl_int clStatus, cl_kernel kernel, size_t global_size, size_t local_size, int* C, cl_mem C_clmem, int* D, cl_mem D_clmem, cl_command_queue command_queue) {
	clStatus = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);

	// Read the cl memory C_clmem on device to the host variable C
	clStatus = clEnqueueReadBuffer(command_queue, C_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), C, 0, NULL, NULL);


	// Clean up and wait for all the comands to complete.
	clStatus = clFlush(command_queue);
	clStatus = clFinish(command_queue);
}

void displaySnake(int* A, int* B, int* , cl_int clStatus, cl_kernel kernel, size_t global_size, size_t local_size, int* C, cl_mem C_clmem, cl_command_queue command_queue, cl_mem A_clmem, cl_mem B_clmem)
{

	int hx = snake_body[0].first;  
	int hy = snake_body[0].second;

	glColor3f(0, 0, 0);

	int len = snake_body.size();
	for (int i = 0; i < len; i++)
	{
		// ----------- Updates Snake with new cordinates -----------
		int x = snake_body[i].first;
		int y = snake_body[i].second;
		A[0] = x;
		B[0] = 1;
		E[0] = 1;
		C[0] = 0;
		C[1] = 0;
		D[0] = y;
		clStatus = clEnqueueWriteBuffer(command_queue, A_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), A, 0, NULL, NULL);
		clStatus = clEnqueueWriteBuffer(command_queue, B_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), B, 0, NULL, NULL);
		clStatus = clEnqueueWriteBuffer(command_queue, D_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), D, 0, NULL, NULL);

		clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&A_clmem);
		clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&B_clmem);
		clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&C_clmem);
		clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&D_clmem);
		clStatus = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&E_clmem);


		exKer(clStatus, kernel, global_size, local_size, C, C_clmem,D, D_clmem, command_queue);

		// Display the result to the screen
		//for (i = 0; i < VECTOR_SIZE; i++)
		printf("%d + %d = %d   |||  %d + %d = %d \n",A[0], B[0], C[0], D[0], B[0], C[1]);




		glColor3f(0, 0, 0);
		glBegin(GL_POLYGON); 
		glVertex2i(x, y);
		glVertex2i(x, C[1]);
		glVertex2i(C[0], C[1]);
		glVertex2i(C[0], y);
		glEnd();



	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	gluOrtho2D(0, map_size, 0, map_size);
	displaySnake( A,  B, C,  clStatus,  kernel,  global_size,  local_size,  C,  C_clmem,  command_queue,  A_clmem,  B_clmem);
	displayBorders();
	glutSwapBuffers();
}
int main(int argc, char ** argv) {
	int luck = 5;          
	int initialLives = 5;
	int maxDifficulty = 20; 
    int i;

	int alpha = 2.0;
	A[0] = 0;
	B[0] = 1;
	C[0] = 0;
	D[0] = 0;
	// Get platform and device information
	cl_platform_id* platforms = NULL;
	cl_uint     num_platforms;
	//Set up the Platform
	clStatus = clGetPlatformIDs(0, NULL, &num_platforms); //setting up platforms in order to set up the host for processing and multiple devices forkernel execution
	platforms = (cl_platform_id*)
		malloc(sizeof(cl_platform_id) * num_platforms);
	clStatus = clGetPlatformIDs(num_platforms, platforms, NULL);

	//Get the devices list and choose the device you want to run on
	cl_device_id* device_list = NULL;
	cl_uint           num_devices;

	clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_DEFAULT, 0, NULL, &num_devices); // will return list of devices available to to choose devices
	device_list = (cl_device_id*)
		malloc(sizeof(cl_device_id) * num_devices);
	clStatus = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_devices, device_list, NULL);

	// Create one OpenCL context for each device in the platform
	cl_context context;
	context = clCreateContext(NULL, num_devices, device_list, NULL, NULL, &clStatus);

	// Create a command queue
	command_queue = clCreateCommandQueue(context, device_list[0], 0, &clStatus);

	// Create memory buffers on the device for each vector
	A_clmem = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(float), NULL, &clStatus);
	B_clmem = clCreateBuffer(context, CL_MEM_READ_ONLY, VECTOR_SIZE * sizeof(float), NULL, &clStatus);
	C_clmem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(float), NULL, &clStatus);
	D_clmem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(float), NULL, &clStatus);
	E_clmem = clCreateBuffer(context, CL_MEM_WRITE_ONLY, VECTOR_SIZE * sizeof(float), NULL, &clStatus);


	// Copy the Buffer A and B to the device
	clStatus = clEnqueueWriteBuffer(command_queue, A_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), A, 0, NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, B_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), B, 0, NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, D_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), D, 0, NULL, NULL);
	clStatus = clEnqueueWriteBuffer(command_queue, D_clmem, CL_TRUE, 0, VECTOR_SIZE * sizeof(float), E, 0, NULL, NULL);

	// Create a program from the kernel source
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&saxpy_kernel, NULL, &clStatus);

	// Build the program
	clStatus = clBuildProgram(program, 1, device_list, NULL, NULL, NULL);

	clock_t start, end;
	double cpu_time_used;

	start = clock();            //start timer for CPU usage

	// Create the OpenCL kernel
	kernel = clCreateKernel(program, "saxpy_kernel", &clStatus);

	// Set the arguments of the kernel
	clStatus = clSetKernelArg(kernel, 0, sizeof(float), (void*)&alpha);
	clStatus = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&A_clmem);
	clStatus = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void*)&B_clmem);
	clStatus = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&C_clmem);
	clStatus = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&D_clmem);
	clStatus = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&E_clmem);




	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(map_size * 30, map_size * 30);
	glutInitWindowPosition(300, 100);
	glutCreateWindow("Snake Game");
	glutDisplayFunc(display);
	glutReshapeWindow(500, 500);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(50, TimerFunc, 0);
	//exKer(clStatus, kernel, global_size, local_size, C, C_clmem, command_queue);
	glClearColor(1, 1, 1, 0);
	snake_body.clear();
	snake_body.push_front({ 2,4 });
	snake_body.push_front({ 3,4 });
	snake_body.push_front({ 4,4 });
	// Allocate space for vectors A, B and C

	glutMainLoop();

	clStatus = clReleaseKernel(kernel);
	clStatus = clReleaseProgram(program);
	clStatus = clReleaseMemObject(A_clmem);
	clStatus = clReleaseMemObject(B_clmem);
	clStatus = clReleaseMemObject(C_clmem);
	clStatus = clReleaseCommandQueue(command_queue);
	clStatus = clReleaseContext(context);
	free(A);
	free(B);
	free(C);
	free(platforms);
	free(device_list);





    // Finally release all OpenCL allocated objects and host buffers.
    return 0;
}