#include "OneLoneCoder/olcPixelGameEngine.h"
#include "units.h"

#ifndef P2_PATHFINDING
#define P2_PATHFINDING

struct sCell{
    enum eType{ NORMAL, FOREST, WATER, HILL, BLOCK };
    int type = NORMAL;
    olc::vi2d pos = {0,0};
    olc::vi2d tile = {0,0};
    bool bVisited = false;
    int nPath = -1;
    bool bAvailablePath = false;
    int nGlobalGoal;
    int nLocalGoal;
    sCell* parent;
    cUnit* currUnit = nullptr;

    explicit sCell(int nTYPE, olc::vi2d viPOS, olc::vi2d viTILE){
        type = nTYPE;
        pos = viPOS;
        tile = viTILE;
        bVisited = false;
        nPath = -1;
        bAvailablePath = false;
        nGlobalGoal = INT_MAX;
        nLocalGoal = INT_MAX;
        parent = nullptr;
        currUnit = nullptr;
    }
};

int TDIndex(olc::vi2d pos, int width){
    return pos.x+pos.y*width;
}

std::vector<olc::vi2d> FindPathBack(sCell* destinyCell){
    std::vector<olc::vi2d> returnPath;
    if (destinyCell != nullptr){
        sCell* p = destinyCell;

        olc::vi2d posParentDiff;
        olc::vi2d posChildDiff;
        olc::vi2d posChild;

        olc::vi2d posL = {1,0};
        olc::vi2d posR = {-1,0};
        olc::vi2d posU = {0,1};
        olc::vi2d posD = {0,-1};

        while (p->parent != nullptr){
            returnPath.emplace_back(p->pos);
            posParentDiff = p->pos - p->parent->pos; 
            if(p == destinyCell){
                if(posParentDiff == posL)
                    p->nPath = 11;
                else if(posParentDiff == posR)
                    p->nPath = 13;
                else if(posParentDiff == posU)
                    p->nPath = 12;
                else //if(posParentDiff == posD)
                    p->nPath = 10;
            }
            else{
                posChildDiff = p->pos - posChild;
                if(posParentDiff == posL){
                    if(posChildDiff == posD)
                        p->nPath = 8; // left-down
                    else if(posChildDiff == posR)
                        p->nPath = 5;
                    else //if(posChildDiff == posU)
                        p->nPath = 9; // left-up
                }
                else if(posParentDiff == posD){
                    if(posChildDiff == posR)
                        p->nPath = 7;
                    else if(posChildDiff == posU)
                        p->nPath = 4;
                    else //if(posChildDiff == posL)
                        p->nPath = 8;
                }
                else if(posParentDiff == posR){
                    if(posChildDiff == posU)
                        p->nPath = 6; // right-up
                    else if(posChildDiff == posL)
                        p->nPath = 5;
                    else //if(posChildDiff == posD)
                        p->nPath = 7; // left-down
                }
                else /*if(posParentDiff == posU)*/{
                    if(posChildDiff == posL)
                        p->nPath = 9; // up-left
                    else if(posChildDiff == posD)
                        p->nPath = 4;
                    else //if(posChildDiff == posR)
                        p->nPath = 6; // up-right
                }
            } 
            posChild = p->pos;
            p = p->parent;
        }
        returnPath.emplace_back(p->pos);
        posChildDiff = posChild - p->pos;
        if(posChildDiff == posL)
            p->nPath = 1;
        else if(posChildDiff == posR)
            p->nPath = 3;
        else if(posChildDiff == posU)
            p->nPath = 2;
        else //if(posChildDiff == posD)
            p->nPath = 0;
    }

    return returnPath;
}

std::array<int, 4> FindNeighbours(sCell* currentCell, int nGridW, int nGridH){
    std::array<int, 4> nNeigCell = {-1,-1,-1,-1};
    if(currentCell->pos.y > 0)
        nNeigCell[0] = currentCell->pos.x+((currentCell->pos.y-1)*nGridW);
    if(currentCell->pos.x > 0)
        nNeigCell[1] = (currentCell->pos.x-1)+(currentCell->pos.y*nGridW);
    if(currentCell->pos.y < nGridH-1)
        nNeigCell[2] = currentCell->pos.x+((currentCell->pos.y+1)*nGridW);
    if(currentCell->pos.x < nGridW-1)
        nNeigCell[3] = (currentCell->pos.x+1)+(currentCell->pos.y*nGridW);
    return nNeigCell;
} 

std::vector<olc::vi2d> Solve_AStar(std::vector<sCell*> cells, sCell* originCell, sCell* destinyCell, int nGridW, int nGridH){
    for(int x = 0; x < nGridW; x++){
        for(int y = 0; y < nGridH; y++){
            cells[x+y*nGridW]->bVisited = false;
            cells[x+y*nGridW]->nPath = -1;
            cells[x+y*nGridW]->nGlobalGoal = INT_MAX;
            cells[x+y*nGridW]->nLocalGoal = INT_MAX;
            cells[x+y*nGridW]->parent = nullptr;
        }
    }

    auto distance = [](sCell* a, sCell* b){
        return std::abs(a->pos.x - b->pos.x) + std::abs(a->pos.y - b->pos.y);
    };
    auto heuristic = [distance](sCell* a, sCell* b){ // solo por claridad, remover despues
        return distance(a, b);
    };

    sCell* currentCell = originCell;
    originCell->nLocalGoal = 0;
    originCell->nGlobalGoal = heuristic(originCell, destinyCell);

    std::list<sCell*> listNotTestedCells;
    listNotTestedCells.push_back(originCell);

    while(!listNotTestedCells.empty() && currentCell != destinyCell){
        listNotTestedCells.sort([](const sCell* lhs, const sCell* rhs){ return lhs->nGlobalGoal < rhs->nGlobalGoal; });
        while(!listNotTestedCells.empty() && listNotTestedCells.front()->bVisited)
            listNotTestedCells.pop_front();

        if(listNotTestedCells.empty())
            break;
        
        currentCell = listNotTestedCells.front();
        currentCell->bVisited = true;

        std::array<int,4> nNeigCell = FindNeighbours(currentCell, nGridW, nGridH);
        for(int i = 0; i < 4; i++){
            if(nNeigCell[i] != -1){
                sCell* n = cells[nNeigCell[i]];
                if(!n->bVisited && n->bAvailablePath && n->type != sCell::BLOCK && n->type != sCell::WATER)
                    listNotTestedCells.push_back(n);
                int nPossiblyLowerGoal = currentCell->nLocalGoal + distance(currentCell, n);
                if(currentCell->type == sCell::FOREST || currentCell->type == sCell::HILL)
                    nPossiblyLowerGoal++; // a mejorar
                if(nPossiblyLowerGoal < n->nLocalGoal){
                    n->parent = currentCell;
                    n->nLocalGoal = nPossiblyLowerGoal;
                    n->nGlobalGoal = n->nLocalGoal + heuristic(n, destinyCell);
                }
                
            }
        }        
    }
    for(int x = 0; x < nGridW; x++){
        for(int y = 0; y < nGridH; y++){
            cells[x+y*nGridW]->nPath = -1;
        }
    }
    return FindPathBack(destinyCell);
}

bool FindAvailablePath(std::vector<sCell*> cells, sCell* currentCell, int steps, int nGridW, int nGridH, const int& nFaction = -1){
    if(steps >= 0 && (nFaction == -1 || (currentCell->type != sCell::BLOCK && currentCell->type != sCell::WATER && (currentCell->currUnit == nullptr || currentCell->currUnit->GetFaction() == nFaction)))){
        currentCell->bAvailablePath = true;
        if((currentCell->type == sCell::FOREST || currentCell->type == sCell::HILL) && nFaction != -1)
            steps--;
        
        std::array<int,4> nNeigCell = FindNeighbours(currentCell, nGridW, nGridH);
        for(int i = 0; i < 4; i++){
            if(nNeigCell[i] != -1){
                FindAvailablePath(cells, cells[nNeigCell[i]], steps - 1, nGridW, nGridH, nFaction);
            }
        }  
    }
    return true;
}



#endif