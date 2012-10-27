/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */
#include <cstdlib>
#include <iostream>
#include <modules>

extern "C"
{
    void HelloWorld_start();
}


using namespace std;

class HW {
private:
	int id;
public:
	virtual void setId( int id );
	virtual void printHello( void );
	virtual ~HW();

};

void helloWorld(void* binary, void* arg2, void* arg3, void* arg4)
{
	HW* obj = new HW;
	HW* obj2 = new HW;
	obj->setId(1);
	obj->printHello();
	delete obj;
	
	obj2->setId(2);
	obj2->printHello();
	delete obj2;

	printf("Hello world from ExecKernel hook. Binary located at 0x%X\n", binary);
	getchar();
}

void HelloWorld_start()
{
	//printf("Hooking 'ExecKernel'\n");
	register_hook_callback("ExecKernel", &helloWorld);
	register_hook_callback("Kernel Start", &helloWorld);
}

void HW::printHello()
{
	//cout << "[" << id << "] HelloWorld from a c++ function\n";
	printf("[%d] HelloWorld from a c++ function\n", id);
}

void HW::setId(int id)
{
	this->id = id;
}

HW::~HW()
{
}
