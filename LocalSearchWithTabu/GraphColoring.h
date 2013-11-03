/**
*   usage:  step1: construct a GraphColoring object.
*           step2: call setupGraph() to read the graph from input file.
*           step3, call initConfig() to set the color number to be test and randomly coloring the graph.
*           step4, call solve() to search for the result
*           step5, call printResult to check if it is solved correctly, print result and log to "log.csv".
*           
*           if you use copy constructor to the GraphColoring object with (copyConfig == true),
*           then you can go to step4:solve() directly to continue the original search.
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

    GraphColoring(volatile bool *pIsSolved=NULL);
    GraphColoring(const GraphColoring &gc, bool copyConfig);
    ~GraphColoring();

    int getColorNum() const {    return colorNum;    }

    void setupGraph(FILE *input);
    void initConfig(int colorNum);
    bool solve(const int timeOut);
    void printResult() const;

private:
    void genAdjColorTable();
    int evaluate() const;
    bool check() const;
    void printVertexAndColor() const;

    volatile bool *pIsSolved; // used for mutiple thread support

    int colorNum;       // must be set before init()
    int iterCount;
    int minConflict;    // no need to be set in the constructor
    clock_t duration;   // no need to be set in the constructor

    Graph *graph;
    AdjColorTable *adjColorTable;
    AdjColorTable *tabuTable;

    // abandoned methods
    GraphColoring(const GraphColoring &gc) {}
    const GraphColoring& operator=(const GraphColoring &gc) {}
};


#endif
