#include <iostream>
#include <fstream>
#include <string>

using namespace std;


int main(int argc, char* argv[]){

    if(argc < 3){
        cerr << "Usage :" << argv[0] << "<FullName> <PhoneNumber> \n";
        return 1;
    }


    // Construct the entry as "Full Name, PhoneNumber\n" format
    string name = argv[1];
    string phone_number = argv[2];


    //open the phone book file
    ofstream phone_book("phoneBook.txt", ios::app);

    if(!phone_book.is_open()){
        cerr << "Failed in open phoneBook.txt file";
        return 1;
    }

    //Write the details into the file
    phone_book << name << "," << phone_number << "\n";
    phone_book.close();

    cout << "Added data :" << name << "," << phone_number << endl;

    return 0;
}
