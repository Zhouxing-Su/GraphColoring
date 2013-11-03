#include "GraphColoring.h"

using namespace std;

class GraphColoring::Graph
{
    class Vertex;

public:
    Graph(FILE *input);
    Graph(const Graph &g);
    ~Graph(void);

    void setRandColorToVertices(int colorNum);
    inline int getVertexNum() const {    return vertexNum;    }
    inline Vertex& getVertex(int index);

private:
    Vertex* adjacencyList;
    int vertexNum;
};

class GraphColoring::Graph::Vertex
{
public:
    Vertex();
    ~Vertex();

    void addAdjVertex(int adjacentVertexIndex);
    inline int getAdjVertex(int index) {    return adjacentVertices[index];    }
    inline int getAdjVertexNum() {    return adjacentVertices.size();    }

    GraphColoring::Color color;

private:
    std::vector<int> adjacentVertices;
};

class GraphColoring::AdjColorTable
{
public:
    AdjColorTable(int vertexNum, int colorNum);
    AdjColorTable(const AdjColorTable &act);
    ~AdjColorTable();

    int& at(int vertex, int color) {    return *(table + (vertex * colorNum) + color);    }

private:
    int *table;
    const int colorNum;
    const int vertexNum;
};







GraphColoring::Graph::Vertex::Vertex()
{
}


GraphColoring::Graph::Vertex::~Vertex()
{
}

void GraphColoring::Graph::Vertex::addAdjVertex(int adjacentVertexIndex)
{
    adjacentVertices.push_back(adjacentVertexIndex);
}







GraphColoring::Graph::Graph(FILE *input)
{
    int v1, v2;
    char buf[256];

    // read the comments
    while ((fgets(buf, 255, input) != NULL) && (buf[0] != 'p')) {}
    // get vertex number
    sscanf(buf, "%*s%*s%d", &vertexNum);
    adjacencyList = new Vertex[vertexNum];
    // read the edges
    while (fgets(buf, 255, input) != NULL) {
        sscanf(buf, "%*s%d%d", &v1, &v2);
        adjacencyList[v1-1].addAdjVertex(v2-1);
        adjacencyList[v2-1].addAdjVertex(v1-1);
    }
}

GraphColoring::Graph::Graph(const Graph &g) : vertexNum(g.vertexNum)
{
    adjacencyList = new Vertex[vertexNum];
    memcpy(adjacencyList, g.adjacencyList, (vertexNum * sizeof(Vertex)));
}

GraphColoring::Graph::~Graph(void)
{
    delete [] adjacencyList;
}


void GraphColoring::Graph::setRandColorToVertices(int colorNum)
{
    srand(time(NULL));
    for (int i = 0; i < vertexNum; ++i) {
        adjacencyList[i].color = (rand() % colorNum);
    }
}

inline GraphColoring::Graph::Vertex& GraphColoring::Graph::getVertex(int index)
{
    return adjacencyList[index];
}








GraphColoring::AdjColorTable::AdjColorTable(int vn, int cn)
    : table(new int[vn * cn]), colorNum(cn), vertexNum(vn)
{
    memset(table, 0, vertexNum * colorNum * sizeof(int));
}


GraphColoring::AdjColorTable::AdjColorTable(const AdjColorTable &act)
    : table(new int[act.vertexNum * act.colorNum]), colorNum(act.colorNum), vertexNum(act.vertexNum)
{
    memcpy(table, act.table, vertexNum * colorNum * sizeof(int));
}

GraphColoring::AdjColorTable::~AdjColorTable()
{
    delete [] table;
}







GraphColoring::GraphColoring() 
    : iterCount(0), graph(NULL), adjColorTable(NULL), tabuTable(NULL)
{
}

GraphColoring::GraphColoring(const GraphColoring &gc, bool copyConfig)
    : iterCount(gc.iterCount), graph(NULL), adjColorTable(NULL), tabuTable(NULL)
{
    if (copyConfig == true) {
        minConflict = gc.minConflict;
        colorNum = gc.colorNum;
        graph = new Graph(*(gc.graph));
        adjColorTable = new AdjColorTable(*(gc.adjColorTable));
        tabuTable = new AdjColorTable(*(gc.tabuTable));
    }
}

GraphColoring::~GraphColoring()
{
    delete adjColorTable;
    adjColorTable = NULL;
    delete tabuTable;
    tabuTable = NULL;
    delete graph;
    graph = NULL;
}


void GraphColoring::setupGraph( FILE *input )
{
    delete graph;   // drop previous graph
    graph = new Graph(input);
}

void GraphColoring::initConfig(int cn)
{
    colorNum = cn;
    graph->setRandColorToVertices(colorNum);
    genAdjColorTable();
    minConflict = evaluate();

    TabuTenureBase = 0;
    //TabuTenureBase = colorNum/2 + 10;
}

bool GraphColoring::solve(const int timeOut)
{
    clock_t start = clock();

    typedef struct ReduceInfo {
        ReduceInfo(int r=0x80000000, int v=0, int d=0) : reduce(r), vertex(v), desColor(d) {}
        ReduceInfo(const ReduceInfo &ri) : reduce(ri.reduce), vertex(ri.vertex), desColor(ri.desColor) {}
        int reduce;
        int vertex;
        int desColor;
    } ReduceInfo;

    ReduceInfo maxReduce;
    int conflict = minConflict;

    for (; (conflict > 0) && (iterCount < timeOut); ++iterCount) {
        maxReduce = ReduceInfo();
        int bestMoveCount = 2;
        for (int i = 0; i < graph->getVertexNum(); ++i) {   // for each vertex that has conflicts
            int color = graph->getVertex(i).color;
            if ( adjColorTable->at(i, color) > 0) {
                for (int j = 0; j < colorNum; ++j) {        // for each destination color
                    if (color != j) {                       // test if it is the max reduce
                        int reduce = (adjColorTable->at(i, color) - adjColorTable->at(i,j));
                        if ((conflict - reduce) < minConflict) {
                            if (reduce > maxReduce.reduce) { // only way for tabu move to be chosen
                                maxReduce = ReduceInfo(reduce,i,j);
                                bestMoveCount = 2;
                            } else if ((reduce == maxReduce.reduce) && (tabuTable->at(i, j) < iterCount)) {
                                if ((rand() % bestMoveCount) == 0) {
                                    maxReduce = ReduceInfo(reduce,i,j);
                                }
                                ++bestMoveCount;
                            }
                        } else if (tabuTable->at(i, j) < iterCount) {
                            if (reduce > maxReduce.reduce) {
                                maxReduce = ReduceInfo(reduce,i,j);
                                bestMoveCount = 2;
                            } else if (reduce == maxReduce.reduce) {
                                if ((rand() % bestMoveCount) == 0) {
                                    maxReduce = ReduceInfo(reduce,i,j);
                                }
                                ++bestMoveCount;
                            }
                        }
                    }
                }
            }
        }
        
        conflict -= maxReduce.reduce;
        minConflict = (minConflict > conflict)?(conflict):(minConflict);
        int srcColor = graph->getVertex(maxReduce.vertex).color;
        tabuTable->at(maxReduce.vertex, srcColor) = (iterCount + conflict + TabuTenureBase + (rand() % 10));
        // update the color of the vertex
        graph->getVertex(maxReduce.vertex).color = maxReduce.desColor;
        // update the infomation of the adjacent vertices in the adjacent color table
        for (int k = graph->getVertex(maxReduce.vertex).getAdjVertexNum(); --k >= 0;) {
            int vertex = graph->getVertex(maxReduce.vertex).getAdjVertex(k);
            ++(adjColorTable->at(vertex, maxReduce.desColor));
            --(adjColorTable->at(vertex, srcColor));
        }
    }

    duration = (clock() - start);
    return (conflict == 0);
}

bool GraphColoring::check() const
{
    if (evaluate() > 0) {
        printf("The answer is wrong according to adjacent color table!\n");
    } else {
        printf("The answer is right according to adjacent color table!\n");
    }


    for (int i = 0; i < graph->getVertexNum(); ++i) {
        int color = graph->getVertex(i).color;
        for (int j = graph->getVertex(i).getAdjVertexNum(); --j >= 0;) {
            if (color == graph->getVertex(graph->getVertex(i).getAdjVertex(j)).color) {
                printf("The answer is wrong according to the original graph!\n");
                return false;
            }
        }
    }

    printf("The answer is right according to the original graph!\n");
    return true;
}


void GraphColoring::printResult() const
{
    printf("\n    Vertex    Color\n");
    for (int i = 0; i < graph->getVertexNum(); ++i) {
        printf("%10d%9d\n", i, graph->getVertex(i).color);
    }
    printf("\nduration: %lf\niteration: %d\n", (duration / 1000.0), iterCount);
    
    FILE *flog = fopen("log.csv", "a");
    if (check() == true) {
        fprintf(flog, "\n%d,%d,%lf,%d", graph->getVertexNum(), colorNum, (duration / 1000.0), iterCount);
    } else {
        fprintf(flog, "\n%d,%d,%lf,TIME_OUT", graph->getVertexNum(), colorNum, (duration / 1000.0));
    }
    fclose(flog);
}

void GraphColoring::genAdjColorTable()
{
    tabuTable = new AdjColorTable(graph->getVertexNum(), colorNum);

    adjColorTable = new AdjColorTable(graph->getVertexNum(), colorNum);
    for (int i = 0; i < graph->getVertexNum(); ++i) {
        for (int j = graph->getVertex(i).getAdjVertexNum(); --j >= 0;) {
            ++(adjColorTable->at(i, graph->getVertex(graph->getVertex(i).getAdjVertex(j)).color));
        }
    }
}

int GraphColoring::evaluate() const
{
    int conflict = 0;
    for (int i = 0; i < graph->getVertexNum(); ++i) {
        conflict += adjColorTable->at(i, graph->getVertex(i).color);
    }
    return (conflict / 2);
}
