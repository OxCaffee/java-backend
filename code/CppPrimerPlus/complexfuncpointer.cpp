#include<iostream>

using namespace std;

const double* f1(const double arr[], int n);
const double* f2(const double [], int n);
const double* f3(const double *, int);

int main(){
	double av[3] = {1112.3, 1542.1, 2227.9};
	
	//pointer to a function
	const double* (*p1)(const double *, int) = f1;
	//c++11 automatic type deduction
	//eqauls to:
	//const double* (*p2)(const double *, int) = f2;
	auto p2 = f2;
	
	cout << "Using pointers to functions:\n";
	cout << "Address Value\n";
	cout << (*p1)(av, 3) << ": " << *(*p1)(av, 3) << endl;
	cout << p2(av, 3) << ": " << *p2(av, 3) << endl;
	
	//pa an array of pointers
	//auto doesn't work with list initialization
	const double* (pa[3])(const double *, int) = {f1, f2, f3};
	//but it does work for initialization to a single value
	//pb a pointer to first element of pa
	auto pb = pa;
	//pre-c++11 can use the following code instead
	//const double *(**pb)(const double *, int) = pa;
	cout << "\nUsing an array of pointers to functions:\n";
	cout << "Address  Value\n";
	for(int i = 0; i < 3; i++){
		cout << pa[i](av, 3) << ": " << *pa[i](av, 3) << endl;
	}
	cout << "\nUsing a pointer to a pointer to a function:\n";
	cout << "Address  Value" << endl;
	for(int i = 0; i < 3; i++){
		cout << pb[i](av, 3) << ": " << *pb[i](av, 3) << endl;
	}
	
	//whta about a pointer to an array of function pointers
	cout << "\nUsing pointers to an array of pointers:\n";
	cout << "Address  Value" << endl;
	auto pc = &pa;
	cout << (*pc)[0](av, 3) << ": " << *(*pc)[0](av, 3) << endl;
	const double* (*(*pd)[3])(const double *, int) = &pa;
	const double* pbd = (*pd)[1](av, 3);
	cout << pdb << ": " << *pdb << endl;
	cout << (*(*pd)[2])(av, 3) << ": " << *(*(*pd)[2])(av, 3) << endl;
	cin.get();
	return 0;
}

const double* f1(const double* ar, int n){
	return ar;
}

const double* f2(const double ar[], int n){
	return ar + 1;
}

const double* f3(const double ar[], int n){
	return ar * 2;
}
