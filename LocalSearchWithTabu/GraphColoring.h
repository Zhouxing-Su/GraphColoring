/**
*   usage:  use input file path as a command line argument.
*           if there is no command line argument, then input is standard input.
*           then input the color number you want to test if it is able to color the graph
*/

#ifndef GRAPH_COLORING_H
#define GRAPH_COLORING_H

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>

class GraphColoring
{
private:
    class AdjColorTable;
    class Graph;

public:
    typedef int Color;

    int TabuTenureBase;

    GraphColoring();
    ~GraphColoring();

    void setColorNum(int num) {    colorNum = num;    }
    int getColorNum() const {    return colorNum;    }

    void setupGraph(FILE *input);
    bool solve(const int timeOut);
    bool check() const;
    void printResult() const;

private:
    void genAdjColorTable();
    int evaluate() const;

    int minConflict;
    int colorNum;
    int iterCount;
    clock_t duration;

    Graph *graph;
    AdjColorTable *adjColorTable;
    AdjColorTable *tabuTable;
};


#endif
