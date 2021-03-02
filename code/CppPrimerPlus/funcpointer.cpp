#include<iostream>

double func_a(int);
double func_b(int);
void func_c(int lines, double (*pf)(int));

using namespace std;

int main(){
	int code;
	cout << "How many lines of code do you need? ";
	cin >> code;
	cout << "Here is func_a's result: ";
	func_c(code, func_a);
	cout << "Here is func_b's result: ";
	func_c(code, func_b);
	return 0;
} 

double func_a(int code){
	return 0.5 * code;
}

double func_b(int code){
	return 0.1 * code;
}

void func_c(int lines, double (*pf)(int)){
	cout << lines << " lines will take ";
	cout << (*pf)(lines) << " hour(s)" << endl;
}
