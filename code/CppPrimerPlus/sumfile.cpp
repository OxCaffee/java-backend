#include<iostream>
#include<fstream>
#include<cstdlib>

using namespace std;

const int SIZE = 60;

int main(){
	char filename[SIZE];
	
	ifstream inFile;
	cout << "Enter name of data file: ";
	cin.getline(filename, SIZE);
	inFile.open(filename);
	if(!inFile.is_open()){
		cout << "Could not open the file " << filename << endl;
		cout << "Program terminating.\n";
		exit(EXIT_FAILURE);
	}
	double value;
	double sum = 0.0;
	int count = 0;
	
	inFile >> value;
	while(inFile.good()){	//check if the file is ok
		++count;
		sum += value;
		inFile >> value;
	}
	if(inFile.eof()){	//if reached the file end
		cout << "End of file reached.\n";
	}else if(inFile.fail()){	//check both eof and file failure
		cout << "Input terminated by data mismatch.\n";
	}
	if(count == 0){
		cout << "No data processed.\n";
	}else{
		cout << "Item read: " << count << endl;
		cout << "Sum: " << sum << endl;
		cout << "Average: " << sum / count << endl; 
	}
	inFile.close();
	return 0;
}
