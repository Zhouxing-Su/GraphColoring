/**
*   usage:  use input file path as a command line argument.
*           if there is no command line argument, then input is standard input.
*           then input the color number you want to test if it is able to color the graph
*/

#include "GraphColoring.h"

using namespace std;

int main( int argc, char *argv[] )
{
    GraphColoring gc;               // step1

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

    int colorNum = 12;
    //printf("please input the color number: ");
    //scanf("%d", &colorNum);

    gc.initConfig(colorNum);        // step3
    gc.solve(0x7fffffff);           // step4
    gc.printResult();               // step5

    // copy constructor usage:

    //GraphColoring gc1(gc, true);
    //gc1.solve(0x7fffffff);
    //gc1.printResult();

    //GraphColoring gc2(gc, false);
    //gc2.initConfig(colorNum);
    //gc2.solve(0x7fffffff);
    //gc2.printResult();

    return 0;
}