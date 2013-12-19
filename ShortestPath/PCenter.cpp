#include "PCenter.h"

using namespace std;

PCenter::PCenter( UndirectedGraph &ug, unsigned pn, int mic )
: pnum( pn ), graph( ug ), closestCenter( ug.vertexAllocNum, ClosestCenterQueue() ),
tabu( ug.vertexAllocNum, vector<int>( ug.vertexAllocNum, 0 ) ), maxIterCount( mic )
{
    graph.getDistSeqTable();
}


PCenter::~PCenter()
{
}

void PCenter::solve( int tabuTenureBase, int tabuTenureAmplitude )
{
    ostringstream ss;
    ss << "perturb(RRGA)+tabu search(B=" << tabuTenureBase << "&A=" << tabuTenureAmplitude << ')';
    solvingAlgorithm = ss.str();

    TabuTenureCalculator getTabuTenure( tabuTenureBase, tabuTenureAmplitude );
    unsigned noImproveCount = 0;

    genInitSolution();

    RandSelect rs( 2 );
    for (int iterCount = 0; iterCount < maxIterCount; iterCount++) {
        bool isSwaped = false;
        CenterSwap centerSwap;
        TopologicalGraph::Distance minRadius = TopologicalGraph::MAX_DISTANCE;

        TopologicalGraph::Arc longestServeArc = findLongestServeArc( closestCenter );

        int longestEnd = longestServeArc.endVertex;
        TopologicalGraph::Distance longestDist = longestServeArc.dist;
        // try each vertex whose distance to longestEnd is shorter than longestDist
        for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
            int newCenter = graph.nthClosestVertex( longestEnd, i );
            if (graph.distance( longestEnd, newCenter ) < longestDist) {
                // find the best swap between center i and non-center vertices
                ClosestCenterTable tmpCCT( closestCenter );
                addCenter( newCenter, tmpCCT );
                TopologicalGraph::Distance radiusAfterAdd = tmpCCT[findFarthestVertex( tmpCCT )].dist[0];
                // calculate new radius for removing each center except the newly added one
                for (TopologicalGraph::VertexSet::iterator iter = center.begin(); iter != center.end(); iter++) {
                    // when *iter is removed
                    int removedCenter = *iter;
                    TopologicalGraph::Distance radiusAfterRemove = radiusAfterAdd;
                    for (int k = graph.minVertexIndex; k <= graph.maxVertexIndex; k++) {
                        if (tmpCCT[k].center[0] == removedCenter) {
                            TopologicalGraph::Distance newDist = tmpCCT[k].dist[1];
                            if (radiusAfterRemove < newDist) {
                                radiusAfterRemove = newDist;
                            }
                        }
                    }
                    // check if the swap between the candidate and the old is better
                    if (radiusAfterRemove < minRadius) {
                        if (radiusAfterRemove < bestSolution.serveRadius
                            || iterCount > tabu[removedCenter][newCenter]) {
                            centerSwap = CenterSwap( removedCenter, newCenter );
                            minRadius = radiusAfterRemove;
                            rs.reset( 2 );
                            isSwaped = true;
                        }
                    } else if (radiusAfterRemove == minRadius) {
                        if (rs.isSelected()) {
                            if (radiusAfterRemove < bestSolution.serveRadius
                                || iterCount > tabu[removedCenter][newCenter]) {
                                centerSwap = CenterSwap( removedCenter, newCenter );
                                isSwaped = true;
                            }
                        }
                    }
                }
            } else {
                break;
            }
        }

        if (!isSwaped || noImproveCount > (4 * graph.vertexNum)) {    // "Random Remove, Greedy Add" perturbation
            minRadius = perturbRRGA( pnum / 4 );
            getTabuTenure.reset();
            noImproveCount = 0;
        } else {    // commit the swap
            center.erase( centerSwap.oldCenter );
            center.insert( centerSwap.newCenter );
            addCenter( centerSwap.newCenter, closestCenter );
            removeCenter( centerSwap.oldCenter );
        }
        // record if it is the best solution
        if (minRadius < bestSolution.serveRadius) {
            timer.record();
            bestSolution.duration = timer.getDuration();
            bestSolution.iterCount = iterCount;
            bestSolution.serveRadius = minRadius;
            bestSolution.center = center;
            noImproveCount = 0;
        } else {
            ++noImproveCount;
        }
        // update tabu
        if (isSwaped) {
            tabu[centerSwap.oldCenter][centerSwap.newCenter] = getTabuTenure( iterCount, (noImproveCount == 0) );
        }
    }
}

void PCenter::tabuSolve( int tabuTenureBase, int tabuTenureAmplitude )
{
    ostringstream ss;
    ss << "tabu search(B=" << tabuTenureBase << "&A=" << tabuTenureAmplitude << ')';
    solvingAlgorithm = ss.str();

    TabuTenureCalculator getTabuTenure( tabuTenureBase, tabuTenureAmplitude );
    genInitSolution();

    RandSelect rs( 2 );
    for (int iterCount = 0; iterCount < maxIterCount; iterCount++) {
        bool isSwaped = false;
        CenterSwap centerSwap;
        TopologicalGraph::Distance minRadius = TopologicalGraph::MAX_DISTANCE;

        TopologicalGraph::Arc longestServeArc = findLongestServeArc( closestCenter );

        int longestEnd = longestServeArc.endVertex;
        TopologicalGraph::Distance longestDist = longestServeArc.dist;
        // try each vertex whose distance to longestEnd is shorter than longestDist
        for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
            int newCenter = graph.nthClosestVertex( longestEnd, i );
            if (graph.distance( longestEnd, newCenter ) < longestDist) {
                // find the best swap between center i and non-center vertices
                ClosestCenterTable tmpCCT( closestCenter );
                addCenter( newCenter, tmpCCT );
                TopologicalGraph::Distance radiusAfterAdd = tmpCCT[findFarthestVertex( tmpCCT )].dist[0];
                // calculate new radius for removing each center except the newly added one
                for (TopologicalGraph::VertexSet::iterator iter = center.begin(); iter != center.end(); iter++) {
                    // when *iter is removed
                    int removedCenter = *iter;
                    TopologicalGraph::Distance radiusAfterRemove = radiusAfterAdd;
                    for (int k = graph.minVertexIndex; k <= graph.maxVertexIndex; k++) {
                        if (tmpCCT[k].center[0] == removedCenter) {
                            TopologicalGraph::Distance newDist = tmpCCT[k].dist[1];
                            if (radiusAfterRemove < newDist) {
                                radiusAfterRemove = newDist;
                            }
                        }
                    }
                    // check if the swap between the candidate and the old is better
                    if (radiusAfterRemove < minRadius) {
                        if (radiusAfterRemove < bestSolution.serveRadius
                            || iterCount > tabu[removedCenter][newCenter]) {
                            centerSwap = CenterSwap( removedCenter, newCenter );
                            minRadius = radiusAfterRemove;
                            rs.reset( 2 );
                            isSwaped = true;
                        }
                    } else if (radiusAfterRemove == minRadius) {
                        if (rs.isSelected()) {
                            if (radiusAfterRemove < bestSolution.serveRadius
                                || iterCount > tabu[removedCenter][newCenter]) {
                                centerSwap = CenterSwap( removedCenter, newCenter );
                                isSwaped = true;
                            }
                        }
                    }
                }
            } else {
                break;
            }
        }

        if (!isSwaped) {    // do strong perturbation
            centerSwap = getRandSwap();
            //do strong perturbation !!!
        }
        // commit the swap
        center.erase( centerSwap.oldCenter );
        center.insert( centerSwap.newCenter );
        addCenter( centerSwap.newCenter, closestCenter );
        removeCenter( centerSwap.oldCenter );
        if (!isSwaped) {    // update minRadius
            minRadius = closestCenter[findFarthestVertex( closestCenter )].dist[0];
        }
        // record if it is the best solution
        if (minRadius < bestSolution.serveRadius) {
            timer.record();
            bestSolution.duration = timer.getDuration();
            bestSolution.iterCount = iterCount;
            bestSolution.serveRadius = minRadius;
            bestSolution.center = center;
            tabu[centerSwap.oldCenter][centerSwap.newCenter] = getTabuTenure( iterCount, true );
        } else {
            tabu[centerSwap.oldCenter][centerSwap.newCenter] = getTabuTenure( iterCount, false );
        }
    }
}

void PCenter::basicSolve()
{
    solvingAlgorithm = "basic local search";
    genInitSolution();

    RandSelect rs( 2 );
    for (int iterCount = 0; iterCount < maxIterCount; iterCount++) {
        CenterSwap centerSwap;
        TopologicalGraph::Distance minRadius = TopologicalGraph::MAX_DISTANCE;

        TopologicalGraph::Arc longestServeArc = findLongestServeArc( closestCenter );

        int longestEnd = longestServeArc.endVertex;
        TopologicalGraph::Distance longestDist = longestServeArc.dist;
        // try each vertex whose distance to longestEnd is shorter than longestDist
        for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
            int newCenter = graph.nthClosestVertex( longestEnd, i );
            if (graph.distance( longestEnd, newCenter ) < longestDist) {
                // find the best swap between center i and non-center vertices
                ClosestCenterTable tmpCCT( closestCenter );
                addCenter( newCenter, tmpCCT );
                TopologicalGraph::Distance radiusAfterAdd = tmpCCT[findFarthestVertex( tmpCCT )].dist[0];
                // calculate new radius for removing each center except the newly added one
                for (TopologicalGraph::VertexSet::iterator iter = center.begin(); iter != center.end(); iter++) {
                    // when *iter is removed
                    int removedCenter = *iter;
                    TopologicalGraph::Distance radiusAfterRemove = radiusAfterAdd;
                    for (int k = graph.minVertexIndex; k <= graph.maxVertexIndex; k++) {
                        if (tmpCCT[k].center[0] == removedCenter) {
                            TopologicalGraph::Distance newDist = tmpCCT[k].dist[1];
                            if (radiusAfterRemove < newDist) {
                                radiusAfterRemove = newDist;
                            }
                        }
                    }
                    // check if the swap between the candidate and the old is better
                    if (radiusAfterRemove < minRadius) {
                        centerSwap = CenterSwap( removedCenter, newCenter );
                        minRadius = radiusAfterRemove;
                        rs.reset( 2 );
                    } else if (radiusAfterRemove == minRadius) {
                        if (rs.isSelected()) {
                            centerSwap = CenterSwap( removedCenter, newCenter );
                        }
                    }
                }
            } else {
                break;
            }
        }
        // commit the swap
        center.erase( centerSwap.oldCenter );
        center.insert( centerSwap.newCenter );
        addCenter( centerSwap.newCenter, closestCenter );
        removeCenter( centerSwap.oldCenter );
        // record if it is the best solution
        if (minRadius < bestSolution.serveRadius) {
            timer.record();
            bestSolution.duration = timer.getDuration();
            bestSolution.iterCount = iterCount;
            bestSolution.serveRadius = minRadius;
            bestSolution.center = center;
        }
    }
}

void PCenter::greedyBasicSolve()
{
    solvingAlgorithm = "greedy local search";
    genInitSolution();

    RandSelect rs( 2 );
    for (int iterCount = 0; iterCount < maxIterCount; iterCount++) {
        CenterSwap centerSwap;
        TopologicalGraph::Distance minRadius = TopologicalGraph::MAX_DISTANCE;

        TopologicalGraph::ArcSet lsa = findLongestServeArcs( closestCenter );

        for (TopologicalGraph::ArcSet::iterator iter = lsa.begin(); iter != lsa.end(); iter++) {
            int longestEnd = iter->endVertex;
            TopologicalGraph::Distance longestDist = iter->dist;
            // try each vertex whose distance to longestEnd is shorter than longestDist
            for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
                int newCenter = graph.nthClosestVertex( longestEnd, i );
                if (graph.distance( longestEnd, newCenter ) < longestDist) {
                    // find the best swap between center i and non-center vertices
                    ClosestCenterTable tmpCCT( closestCenter );
                    addCenter( newCenter, tmpCCT );
                    TopologicalGraph::Distance radiusAfterAdd = tmpCCT[findFarthestVertex( tmpCCT )].dist[0];
                    // calculate new radius for removing each center
                    for (TopologicalGraph::VertexSet::iterator iter = center.begin(); iter != center.end(); iter++) {
                        // when *iter is removed
                        int removedCenter = *iter;
                        TopologicalGraph::Distance radiusAfterRemove = radiusAfterAdd;
                        for (int k = graph.minVertexIndex; k <= graph.maxVertexIndex; k++) {
                            if (tmpCCT[k].center[0] == removedCenter) {
                                TopologicalGraph::Distance newDist = tmpCCT[k].dist[1];
                                if (radiusAfterRemove < newDist) {
                                    radiusAfterRemove = newDist;
                                }
                            }
                        }
                        // check if the swap between the candidate and the old is better
                        if (radiusAfterRemove < minRadius) {
                            centerSwap = CenterSwap( removedCenter, newCenter );
                            minRadius = radiusAfterRemove;
                            rs.reset( 2 );
                        } else if (radiusAfterRemove == minRadius) {
                            if (rs.isSelected()) {
                                centerSwap = CenterSwap( removedCenter, newCenter );
                            }
                        }
                    }
                } else {
                    break;
                }
            }
        }

        // commit the swap
        center.erase( centerSwap.oldCenter );
        center.insert( centerSwap.newCenter );
        addCenter( centerSwap.newCenter, closestCenter );   // add first may pop the being removed center out of the
        removeCenter( centerSwap.oldCenter );   // closestCenter queue which will left less work to removeCenter()
        // record if it is the best solution
        if (minRadius < bestSolution.serveRadius) {
            timer.record();
            bestSolution.duration = timer.getDuration();
            bestSolution.iterCount = iterCount;
            bestSolution.serveRadius = minRadius;
            bestSolution.center = center;
        }
    }
}

bool PCenter::check() const
{
    if (bestSolution.center.size() != pnum) {
        return false;
    }

    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        TopologicalGraph::Distance minRadius = TopologicalGraph::MAX_DISTANCE;
        for (TopologicalGraph::VertexSet::iterator iter = bestSolution.center.begin(); iter != bestSolution.center.end(); iter++) {
            if (minRadius > graph.distance( i, *iter )) {
                minRadius = graph.distance( i, *iter );
            }
        }

        if (minRadius > bestSolution.serveRadius) {
            return false;
        }
    }

    return true;
}

void PCenter::printResult( ostream &os ) const
{
    os << "The max serving radius is : " << bestSolution.serveRadius << endl;
    os << "The indexes of the vertices which are chosed as centers are :\n";
    for (TopologicalGraph::VertexSet::iterator iter = bestSolution.center.begin(); iter != bestSolution.center.end(); iter++) {
        os << *iter << "|";
    }
    os << endl;
}

void PCenter::initResultSheet( std::ofstream &csvFile )
{
    csvFile << "Date, Instance, Algorithm, TotalIter, Duration, IterCount, ServingRadius, Centers" << endl;
}

void PCenter::appendResultToSheet( const string &instanceFileName, ofstream &csvFile ) const
{
    csvFile << Timer::getLocalTime() << ", " << solvingAlgorithm << ", " << instanceFileName << ", " << maxIterCount << ", "
        << bestSolution.duration << ", " << bestSolution.iterCount << ", " << bestSolution.serveRadius << ", ";
    for (TopologicalGraph::VertexSet::iterator iter = bestSolution.center.begin(); iter != bestSolution.center.end(); iter++) {
        csvFile << *iter << "|";
    }
    csvFile << endl;
}




void PCenter::genInitSolution()
{
    // select a vertex as center randomly
    RangeRand viRand( graph.minVertexIndex, graph.maxVertexIndex );
    int firstCenter = viRand();
    center.insert( firstCenter );

    // select one of the longest arc with length longestDist randomly
    RandSelect rs( 2 );
    int longestEnd = graph.nthClosestVertex( firstCenter, graph.maxVertexIndex );
    TopologicalGraph::Distance longestDist = graph.distance( firstCenter, longestEnd );
    for (int i = (graph.maxVertexIndex - 1); i >= graph.minVertexIndex; i--) {
        if (graph.distance( firstCenter, graph.nthClosestVertex( firstCenter, i ) ) == longestDist) {
            if (rs.isSelected()) {
                longestEnd = graph.nthClosestVertex( firstCenter, i );
            }
        } else {
            break;
        }
    }

    // select one of the vertex whose distance to longestEnd is shorter than longestDist randomly
    int secondCenter = graph.findVertexWithinRadius( longestEnd, longestDist );
    center.insert( secondCenter );

    // select other (pnum-2) centers
    initClosestCenter( firstCenter, secondCenter );
    for (int i = pnum - 2; i > 0; i--) {
        int fv = findFarthestVertex( closestCenter );
        int newCenter = graph.findVertexWithinRadius( fv, closestCenter[fv].dist[0] );
        center.insert( newCenter );
        addCenter( newCenter, closestCenter );
    }

    // init the min max min serve radius
    timer.record();
    bestSolution.serveRadius = closestCenter[findFarthestVertex( closestCenter )].dist[0];
    bestSolution.center = center;
    bestSolution.duration = timer.getDuration();
    bestSolution.iterCount = 0;
}

void PCenter::initClosestCenter( int firstCenter, int secondCenter )
{
    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        TopologicalGraph::Distance d1 = graph.distance( firstCenter, i );
        TopologicalGraph::Distance d2 = graph.distance( secondCenter, i );
        if (d1 < d2) {
            closestCenter[i] = ClosestCenterQueue( firstCenter, d1, secondCenter, d2 );
        } else {
            closestCenter[i] = ClosestCenterQueue( secondCenter, d2, firstCenter, d1 );
        }
    }
}

int PCenter::findFarthestVertex( ClosestCenterTable &cct ) const
{
    TopologicalGraph::Distance maxDist = TopologicalGraph::MIN_DISTANCE;
    int farthestVertex;
    RandSelect rs( 2 );
    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        if (maxDist < cct[i].dist[0]) {
            maxDist = cct[i].dist[0];
            farthestVertex = i;
            rs.reset( 2 );
        } else if (maxDist == cct[i].dist[0]) {
            if (rs.isSelected()) {
                farthestVertex = i;
            }
        }
    }

    return farthestVertex;
}

TopologicalGraph::Arc PCenter::findLongestServeArc( ClosestCenterTable &cct ) const
{
    int fv = findFarthestVertex( cct );

    return TopologicalGraph::Arc( cct[fv].center[0], fv, cct[fv].dist[0] );
}

TopologicalGraph::VertexSet PCenter::findFarthestVertices( ClosestCenterTable &cct ) const
{
    TopologicalGraph::Distance maxDist = TopologicalGraph::MIN_DISTANCE;
    TopologicalGraph::VertexSet farthestVertices;
    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        if (maxDist < cct[i].dist[0]) {
            maxDist = cct[i].dist[0];
            farthestVertices.clear();
            farthestVertices.insert( i );
        } else if (maxDist == cct[i].dist[0]) {
            farthestVertices.insert( i );
        }
    }

    return farthestVertices;
}

TopologicalGraph::ArcSet PCenter::findLongestServeArcs( ClosestCenterTable &cct ) const
{
    TopologicalGraph::VertexSet fvs = findFarthestVertices( cct );
    TopologicalGraph::ArcSet longestServeArcs;

    for (TopologicalGraph::VertexSet::iterator iter = fvs.begin(); iter != fvs.end(); iter++) {
        longestServeArcs.insert( TopologicalGraph::Arc( cct[*iter].center[0], *iter, cct[*iter].dist[0] ) );
    }

    return longestServeArcs;
}

void PCenter::addCenter( int newCenter, ClosestCenterTable &cct )
{
    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        TopologicalGraph::Distance newDist = graph.distance( newCenter, i );
        if (newDist < cct[i].dist[0]) {
            cct[i].center[1] = cct[i].center[0];
            cct[i].dist[1] = cct[i].dist[0];
            cct[i].center[0] = newCenter;
            cct[i].dist[0] = newDist;
        } else if (newDist < cct[i].dist[1]) {
            cct[i].center[1] = newCenter;
            cct[i].dist[1] = newDist;
        }
    }
}

void PCenter::removeCenter( int oldCenter )
{
    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        bool changed = false;
        if (closestCenter[i].center[0] == oldCenter) {
            closestCenter[i].center[0] = closestCenter[i].center[1];
            closestCenter[i].dist[0] = closestCenter[i].dist[1];
            changed = true;
        }

        if (changed || closestCenter[i].center[1] == oldCenter) {
            // locate the closest center and add it to the queue
            for (int k = graph.minVertexIndex; k <= graph.maxVertexIndex; k++) {
                TopologicalGraph::VertexSet::iterator iter = center.find( graph.nthClosestVertex( i, k ) );
                if ((iter != center.end()) && (*iter != closestCenter[i].center[0]) && (*iter != oldCenter)) {
                    closestCenter[i].center[1] = *iter;
                    closestCenter[i].dist[1] = graph.distance( *iter, i );
                    break;
                }
            }
        }
    }
}

PCenter::CenterSwap PCenter::getRandSwap() const
{
    RandSelect selectOld( 1 );
    RandSelect selectNew( 1 );

    CenterSwap cs;
    vector<bool> isCenter( graph.vertexAllocNum, false );

    for (TopologicalGraph::VertexSet::iterator iter = center.begin(); iter != center.end(); iter++) {
        isCenter[*iter] = true;
    }

    for (int i = graph.minVertexIndex; i <= graph.maxVertexIndex; i++) {
        if (isCenter[i]) {
            if (selectOld.isSelected()) {
                cs.oldCenter = i;
            }
        } else {
            if (selectNew.isSelected()) {
                cs.newCenter = i;
            }
        }
    }

    return cs;
}

TopologicalGraph::Distance PCenter::perturbRRGA( int perturbStrength )
{
    // remove some centers
    for (int i = perturbStrength; i > 0; i--) {
        RangeRand rr( 0, center.size() - 1 );
        TopologicalGraph::VertexSet::iterator iter = center.begin();
        advance( iter, rr() );
        removeCenter( *iter );
        center.erase( iter );
    }
    // select some new centers
    for (int i = perturbStrength; i > 0; i--) {
        int fv = findFarthestVertex( closestCenter );
        int newCenter = graph.findVertexWithinRadius( fv, closestCenter[fv].dist[0] );
        center.insert( newCenter );
        addCenter( newCenter, closestCenter );
    }
    // recalculate the min radius
    return closestCenter[findFarthestVertex( closestCenter )].dist[0];
}