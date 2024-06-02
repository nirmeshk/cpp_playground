#include <iostream>
#include <vector>
using namespace std;


/**
 *  Usefull links
 *  - https://www.cprogramming.com/tutorial/java/syntax-differences-java-c++.html
 *  - https://www.thegeekstuff.com/2016/02/c-plus-plus-11/
 *  - https://www.programiz.com/cpp-programming/variables-literals (Quality tutorial)
 * 
 * Data Structures
 *  - Vector https://www.programiz.com/cpp-programming/vectors
 *  - 
 * 
 *  Some details about memmory management
 * - Overview of new and delete
 * - why you should never have to use new - https://stackoverflow.com/questions/1695042/is-garbage-collection-automatic-in-standard-c
 * - https://www.stroustrup.com/bs_faq2.html#memory-leaks
 * - another explanation of the same https://stackoverflow.com/questions/8839943/why-does-the-use-of-new-cause-memory-leaks/8840302#8840302
 * - Some explanations about gc -> https://stackoverflow.com/questions/147130/why-doesnt-c-have-a-garbage-collector
 *  
 * 
 * 
 *  Free Books about idiomatic c++
 *  - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms
 *  - http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines  (github -> https://github.com/isocpp/CppCoreGuidelines)
 * 
 * 
 * **/

int main() {
    std::cout << "Hello World Nirmesh!" << endl;    
    std::cout << "Heyo" << endl; 
    
    int n = 23;
    float f = 234343.454545;
    double d = 122323.1212312312;
    char c = 'd';
    bool b = true;

    printf("some random float: %f double: %lf char: %c \n", f, d, c);

    int number = 5;

    if(number<6) {
        cout << "Number is less than 6" << endl;
    } else {
        cout <<" number is greater than 6" << endl;
    }

    int sum_of_first_five_numbers = 0;

    for(int i=0; i<5; i++) {
        sum_of_first_five_numbers += i;
    }

    cout << "sum_of_first_five_numbers: " << sum_of_first_five_numbers <<endl;

    int num_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for(int n: num_array) {
        cout << n << " ";
    }

    cout << endl;

    int i = 1; 

    cout << "while loop from 1 to 5 -> " << endl;
    while (i <= 5) {
        cout << i << " ";
        ++i;
    }

    cout << endl;


    /*
     * Vectors (Arraylist equivalent)
     */
    cout<<endl<< "** Vectors ** " << endl;


    vector<int> vector_number;
    vector_number.push_back(1);
    vector_number.push_back(2);
    vector_number.push_back(3);

    for(int x: vector_number) {
        cout << x << " ";
    }

    cout << endl;

    cout << "Element at Index 2: " << vector_number.at(2) << endl;

    vector_number.at(2) = 23;

    cout << "Element at Index 2: " << vector_number.at(2) << endl;

    // use iterator with for loop
    for (vector<int>::iterator iter = vector_number.begin(); iter != vector_number.end(); ++iter) {
        cout << *iter << "  ";
    }

    cout<<endl<< "** Pointers ** " << endl;


    /**
     * Pointers 
     * **/

    int var = 5;

    // declare pointer variable
    int* pointVar;

    // store address of var
    pointVar = &var;

    // print value of var
    cout << "var = " << var << endl;

    // print address of var
    cout << "Address of var (&var) = " << &var << endl
         << endl;

    // print pointer pointVar
    cout << "pointVar = " << pointVar << endl;

    // print the content of the address pointVar points to
    cout << "Content of the address pointed to by pointVar (*pointVar) = " << *pointVar << endl;
    
    return 0;


}