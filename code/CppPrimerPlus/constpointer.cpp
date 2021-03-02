#include<iostream>

using namespace std;

int main(){
	//test case1---> valid test case
	const int a = 1;
	const int* b = &a;
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;

	//test case2---> invalid test case: can not convert const int* to int
	const int a = 1;
	int* b = &a;	
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;

	//test case3---> valid test case
	int a = 1;
	int* b = &a;
	const int* c = b;
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;
	cout << "value of c: " << *c << ", address of c: " << c << endl;
	*b = 2;
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;
	cout << "value of c: " << *c << ", address of c: " << c << endl;
	a = 3;
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;
	cout << "value of c: " << *c << ", address of c: " << c << endl;

	//test case4(can we change the pointer point at?)
	//in this case we find we can change the pointer point at, but can we change the value of pointer point at?
	int a = 1;
	int b = 2;
	const int* c = &a;
	c = &b;
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << b << ", address of b: " << &b << endl;
	cout << "value of c: " << *c << ", address of c: " << c << endl;

	//test case5(can we change the value the pointer point at?)
	int a = 1;
	const int* b = &a;
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;
	*b = 2;		//after compiling, the compiler throw a error, obviously we can not change the value the pointer point at
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;

	//test case6(how can we make the pointer unchangeable?)
	int a = 1;
	const int* b = &a;
	int* const c = b;	//invalid operation, can not convert const int* to int*
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;
	cout << "value of c: " << *c << ", address of c: " << c << endl;

	//test case7(how to use int* const properly?)
	int a = 1;
	int* const b = &a;
	int c = 2;
	b = &c;	//invalid, after assign of the pointer we can not make this pointer point at other place
	cout << "value of a: " << a << ", address of a: " << &a << endl;
	cout << "value of b: " << *b << ", address of b: " << b << endl;	
	
	return 0;
}
