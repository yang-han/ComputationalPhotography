#include "solver.h"
#include <iostream>
using namespace std;

int main(){
    Solver4251* solver = new Solver4251();
    cout << solver->solve(nullptr, nullptr) << endl;
    return 0;
}