/**
*   usage:  use input file path as a command line argument.
*           if there is no command line argument, then input is standard input.
*           then input the color number you want to test if it is able to color the graph
*   e.g. :  use following command to start the program: 
*                   ./LocalSearchWithTabu.exe  "./TestCases/DSJC500.1.col"
*           then input:
*                   12
*/

#include "GraphColoring.h"
#include <windows.h>
#include <process.h>

using namespace std;

void findSolution(void*);


volatile bool isSolved = false;
GraphColoring gc(&isSolved);        // step1
volatile int gColorNum;

int main(int argc, char *argv[])
{

    if (argc == 2) {
        FILE *input = fopen(argv[argc-1], "r");
        if (input == NULL) {
            return -1;
        }
        gc.setupGraph(input);       // step2(method1)
        fclose(input);
    } else {
        gc.setupGraph(stdin);       // step2(method2)
    }

    printf("please input the color number: ");
    scanf("%d", &gColorNum);

    //gc.initConfig(gColorNum);       // step3
    //gc.solve(0x7fffffff);           // step4
    //gc.printResult();               // step5

    _beginthread(findSolution, 0, NULL);
    _beginthread(findSolution, 0, NULL);
    _beginthread(findSolution, 0, NULL);
    _beginthread(findSolution, 0, NULL);

    while (isSolved == false) {
        Sleep(10000);
    }
    
    system("pause");
    return 0;

    // copy constructor usage:

    //GraphColoring gc1(gc, true);
    //gc1.solve(0x7fffffff);
    //gc1.printResult();

    //GraphColoring gc2(gc, false);
    //gc2.initConfig(colorNum);
    //gc2.solve(0x7fffffff);
    //gc2.printResult();
}

void findSolution(void*)
{
    GraphColoring gcT(gc, false);

    gcT.initConfig(gColorNum);
    gcT.solve(0x7fffffff);
    gcT.printResult();

    _endthread();
}
