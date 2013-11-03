#include "GraphColoring.h"

using namespace std;

int main( int argc, char *argv[] )
{
    GraphColoring gc;               // step1

    //int colorNum;
    //printf("please input the color number: ");
    //scanf("%d", &colorNum);
    gc.setColorNum(12);       // step2

    if (argc == 2) {
        FILE *input = fopen(argv[argc-1], "r");
        if (input == NULL) {
            return -1;
        }
        gc.setupGraph(input);       // step3(method1)
        fclose(input);
    } else {
        gc.setupGraph(stdin);       // step3(method2)
    }

    gc.solve();                     // step4
    gc.check();                     // step5
    gc.printResult();               // step6

    return 0;
}