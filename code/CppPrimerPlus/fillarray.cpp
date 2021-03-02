#include<iostream>

using namespace std;

int fill_array(double arr[], int limit);
void display_array(const double arr[], const int limit);


int main(){
	double arr[10];
	fill_array(arr, 10);
	display_array(arr, 10);
	return 0;
}

int fill_array(double arr[], int limit){
	double tmp;
	int i;
	for(i = 0; i < limit; i++){
		cout << "Enter value #" << (i + 1) << ": ";
		cin >> tmp;
		if(!cin){
			cin.clear();
			while(cin.get() != '\n')
				continue;
			cout << "Bad input; input process terminated.\n";
			break;
		}else if(tmp < 0){
			break;
		}
		arr[i] = tmp;
	}
}

void display_array(const double arr[], const int limit){
	for(int i = 0; i < limit; i++){
		cout << arr[i] << " ";
	} 
	cout << endl;
}
