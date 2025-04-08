#include <iostream>
#include <string>
#include <memory>
#include <time.h>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace std::chrono;

class Person {
    string first_name;
    string second_name;
    int age;
    public:
        Person(string f = "null", string s = "null", int a = 0) {
            first_name = f;
            second_name = s;
            age = a;
        }
        void set_f_name(string f) {first_name = f;}
        void set_s_name(string s) {second_name = s;}
        void set_age(int a) {age = a;}
        string get_f_name() {return first_name;}
        string get_s_name() {return second_name;}
        int get_age() {return age;}
};

int main() {
    auto totalStart = high_resolution_clock::now();

    auto start = high_resolution_clock::now();
    Person* ptr = new Person();
    auto end = high_resolution_clock::now();
    cout << "new Person: " << duration_cast<microseconds>(end - start).count() << " mcs\n";
    delete ptr;

    return 0;
}