/******************************************************************************
MIT License

Copyright (c) 2025 profile_2

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************/

#define OLC_PGE_APPLICATION
#define USE_CUBIC_LUT_3_0
#include "OneLoneCoder/olcPixelGameEngine.h"
//#include "nlohmann/json.hpp"
#include "profile_2/p2util.h"
#include "profile_2/p2LUT.h"
#include "pathfinding.h"
#include "animation.h"
#include "units.h"
#include "hud.h"

#define SCREEN_W    30*16
#define SCREEN_H    15*16
#define PIXEL_S     3
#define CELL_S      16
#define CELL_VFS    {16.0,16.0}

class ENGINE : public olc::PixelGameEngine{
private:
    enum UPDATE_STATE{
        S_DEFAULT_ENTRY,
        S_DEFAULT,
        S_UNIT_SELECTED_ENTRY,
        S_UNIT_SELECTED,
        S_UNIT_SEL_MOVE_ENTRY,
        S_UNIT_SEL_MOVE,
        S_UNIT_SEL_FIGHT_ENTRY,
        S_UNIT_SEL_FIGHT,
        S_UNIT_MOVING_ENTRY,
        S_UNIT_MOVING,
        S_UNIT_FIGHTING_ENTRY,
        S_UNIT_FIGHTING,
        S_UNIT_OPP_SELECTED_ENTRY,
        S_UNIT_OPP_SELECTED,
        S_GROUND_SELECTED_ENTRY,
        S_GROUND_SELECTED,
        S_TURN_TRANSITION_ENTRY,
        S_TURN_TRANSITION
    };

    int nMainLoopState = 0;

    enum FIGHTING_STATE{ FIGHT_S_SETUP, FIGHT_S_ATTACK, FIGHT_S_ATT_ANIM, FIGHT_S_AFTER_ATT, FIGHT_S_DEATH_ANIM, FIGHT_S_CLEANUP };
    enum TRANSITION_STATE{ TRAN_S_SETUP, TRAN_S_SCROLL_IN, TRAN_S_PAUSE, TRAN_S_SCROLL_OUT, TRAN_S_CLEANUP };

    olc::Sprite* sprTilemap = nullptr;
    olc::Decal* dclTilemap = nullptr;
    olc::Sprite* sprArrowmap = nullptr;
    olc::Decal* dclArrowmap = nullptr;
    olc::Sprite* sprCursor = nullptr;
    olc::Decal* dclCursor = nullptr;
    olc::Sprite* sprWindow = nullptr;
    olc::Decal* dclWindow = nullptr;
    olc::Sprite* sprFont = nullptr;
    olc::Decal* dclFont = nullptr;
    olc::Sprite* sprSpark = nullptr;
    olc::Decal* dclSpark = nullptr;
    olc::Sprite* sprMarquee = nullptr;
    olc::Decal* dclMarquee = nullptr;

    sFont fontFfs;

    std::vector<sCell*> vCells;
    sCell* originCell = nullptr;
    sCell* destinyCell = nullptr;
    sCell* currentCell = nullptr;
    const int nGridW = 30;
    const int nGridH = 30;

    const int nTilemapW = 28;//31
    // const int nTilemapH = 32;

    const float fKeyRepDelay = 0.15;
    float fKeyRepCurr = 0.0;
    int keyHeld = 0;

    olc::vi2d viCursor;
    const olc::vf2d vfCursorSprOffs = {-7.0,-7.0};
    const int nCursorMarginV = 32;
    const int nCursorMarginH = 32;
    
    cUnit* unitMovingUnit = nullptr;
    bool bUnitMoving = false;
    const float fMovingMaxT = 1.0f;
    const float fMovingMaxTpCell = 0.25;
    std::vector<olc::vi2d> viMovingPath;
    olc::vf2d vfMovingPos;

    float fPanX;
    float fPanY;
    olc::vf2d vfPan;
    olc::vf2d vfPanPrevious;
    olc::vf2d vfPanNew;
    olc::vf2d vfPanOrigin;
    const float fPanDuration = 0.66;
    float fPanT;
    bool bPanActive = false;

    int layerMain;
    int layerHud;
    int layerMap;
    int layerUnit;

    cAnim* animCursor = nullptr;
    cAnim* animSpark = nullptr;
    
    std::vector<olc::Sprite*> vJobSprites;
    std::vector<olc::Decal*> vJobDecals;
    std::vector<cAnim*> vJobAnims;
    std::vector<cJob*> vJobs;
    std::vector<cUnit*> vUnits;

    olc::vf2d vfHudWinAlign;
    olc::vf2d vfHudWinPos;
    bool bHudUnitMenu = false;
    bool bHudGralMenu = false;
    int nMenuCursor = 0;

    cCombatInfoWin* hudCombatWinL;
    cCombatInfoWin* hudCombatWinR;
    int nCombatState = 0;
    int nCombatAttackerTurns;
    int nCombatOpponentTurns;
    int nCombatActiveUnit = 0;

    int nScreenTransState;
    float fScreenTransTime;
    olc::vf2d vfMarqueePos;

public:
    ENGINE(){
        sAppName = "FLARE ENSIGN!";
    }

protected:
    void ResetCells(bool bResetAvailable, bool bResetPath){
        for(int i= 0; i < (nGridH*nGridW); i++){
            if(bResetAvailable) vCells[i]->bAvailablePath = false;
            if(bResetPath) vCells[i]->nPath = -1;
        }
    }

    int GetDirHeld(float& fTime, float& fElapsedTime){
        int keyHeld = 0;
        if(GetKey(olc::Key::UP).bHeld) keyHeld += 1;
        if(GetKey(olc::Key::DOWN).bHeld) keyHeld += 2;
        if(GetKey(olc::Key::LEFT).bHeld) keyHeld += 4;
        if(GetKey(olc::Key::RIGHT).bHeld) keyHeld += 8;
        if(keyHeld){
            if(fTime == 0.0){
                fTime += fElapsedTime;
                return keyHeld;
            }
            else{
                fTime += fElapsedTime;
                if(fTime >= fKeyRepDelay){
                    fTime = 0.0;
                }
                return 0;
            }
        }
        else{
            fTime = 0.0;
            return 0;
        }
    }

    void UpdateCursor(const int& keyHeld, olc::vi2d& viCursor){
        if((keyHeld & 1) && viCursor.y > 0) { viCursor = viCursor + olc::vi2d(0,-1); }
        if((keyHeld & 2) && viCursor.y < nGridH - 1) { viCursor = viCursor + olc::vi2d(0,+1); }
        if((keyHeld & 4) && viCursor.x > 0) { viCursor = viCursor + olc::vi2d(-1,0); }
        if((keyHeld & 8) && viCursor.x < nGridW - 1) {  viCursor = viCursor + olc::vi2d(+1,0); }
    }

    int InitLayer(int layer = -1){
        int newLayer;
        if(layer == -1) { newLayer = CreateLayer(); }
        else { newLayer = layer; }
        EnableLayer(newLayer, true);
        SetDrawTarget(newLayer);
        Clear(olc::BLANK);
        return newLayer;
    }

    bool OnUserCreate(){
        sprTilemap = new olc::Sprite("./assets/tileset.png");
        dclTilemap = new olc::Decal(sprTilemap);
        sprArrowmap = new olc::Sprite("./assets/arrow.png");
        dclArrowmap = new olc::Decal(sprArrowmap);
        sprCursor = new olc::Sprite("./assets/cursor.png");
        dclCursor = new olc::Decal(sprCursor);
        sprWindow = new olc::Sprite("./assets/window_frame_blue.png");
        dclWindow = new olc::Decal(sprWindow);
        sprFont = new olc::Sprite("./assets/ffs_font_tile.png");
        dclFont = new olc::Decal(sprFont);
        sprSpark = new olc::Sprite("./assets/spark.png");
        dclSpark = new olc::Decal(sprSpark);
        sprMarquee = new olc::Sprite("./assets/turn_marquee.png");
        dclMarquee = new olc::Decal(sprMarquee);

        fontFfs.map = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?/:\"'-.,_;#+()%~*  =·>";
        fontFfs.cell_size = {8,8};
        fontFfs.grid_size = {16,6};

        animCursor = new cAnim(dclCursor, 4, {30.0,30.0}, {0,0}, sAnimDesc({0.1,0.1,0.1,0.1},ANIM_SWING));
        animSpark = new cAnim(dclSpark, olc::vi2d(3,2), olc::vf2d(20,19), olc::vf2d(), {sAnimDesc({0.15,0.15,0.15},ANIM_SINGLE|ANIM_DISABLE_IF_COMPLETE),
                                                                                        sAnimDesc({0.15,0.15,0.15},ANIM_SINGLE|ANIM_DISABLE_IF_COMPLETE)});
        animSpark->Enable(false);
        
        std::vector<int> viTileValues = p2util::ValuesFromCSV("./assets/map_1.csv");
        std::vector<int> viTileTypes = p2util::ValuesFromCSV("./assets/tileset_types.csv");

        hudCombatWinL = new cCombatInfoWin(olc::vf2d(ScreenWidth()/2-138,ScreenHeight()-26), 0);
        hudCombatWinR = new cCombatInfoWin(olc::vf2d(ScreenWidth()/2,ScreenHeight()-26), 0);
        
        
        for(int y = 0; y < nGridH; y++){
            for(int x = 0; x < nGridW; x++){
                int index = x+y*nGridW;
                vCells.emplace_back(new sCell(viTileTypes[viTileValues[index]], olc::vi2d(x,y), 
                                        olc::vi2d(viTileValues[index]%nTilemapW,viTileValues[index]/nTilemapW)*CELL_S));
            }
        }
        
        ParseJobsJSON("./assets/data_files/jobs_1.json", vJobs, vJobSprites, vJobDecals, vJobAnims);
        ParseUnitsJSON("./assets/data_files/units_1.json", vUnits, vJobs);

        for(cUnit* unit: vUnits) { 
            vCells[TDIndex(unit->GetPos(),nGridW)]->currUnit = unit; 
        }

        originCell = vCells[0];

        fPanX = 0.0f;
        fPanY = 0.0f;
        vfPanPrevious = {0.0f,0.0f};

        viCursor = olc::vf2d(ScreenWidth(),ScreenHeight()) / 16 / 2;

        InitLayer(0);
        layerHud = InitLayer();
        // layerUnit = InitLayer();
        layerMain = InitLayer();
        layerMap = InitLayer();

        vfHudWinAlign = {0,0};
        vfHudWinPos = {0,0};
        return true;
    }
    
    bool OnUserUpdate(float fElapsedTime){
        
        keyHeld = GetDirHeld(fKeyRepCurr, fElapsedTime);

        switch(nMainLoopState){
            case S_DEFAULT_ENTRY:{
                bHudUnitMenu = false;
                nMainLoopState = S_DEFAULT;
                //break;
            }

            case S_DEFAULT:{
                if(GetKey(olc::Key::ENTER).bPressed){
                    ResetCells(true,true);
                    if(currentCell->currUnit != nullptr){ 
                        if(currentCell->currUnit->GetFaction() == 0){
                            if(!currentCell->currUnit->Attacked())
                                nMainLoopState = S_UNIT_SELECTED_ENTRY;
                        }
                        else{
                            nMainLoopState = S_UNIT_OPP_SELECTED_ENTRY;
                        }
                    }
                    else{
                        nMainLoopState = S_GROUND_SELECTED_ENTRY;
                    }
                }

                if(keyHeld){
                    UpdateCursor(keyHeld, viCursor);
                    currentCell = vCells[TDIndex(viCursor,nGridW)];
                }

                break;
            }

            case S_UNIT_SELECTED_ENTRY:{
                ResetCells(true,true);
                originCell = currentCell;
                destinyCell = nullptr;
                bHudUnitMenu = true;
                nMenuCursor = 0;
                currentCell->currUnit->AnimSetRow(2);

                nMainLoopState = S_UNIT_SELECTED;
                //break;
            }

            case S_UNIT_SELECTED:{
                if(GetKey(olc::Key::ENTER).bPressed){
                    switch(nMenuCursor){
                        case 0: {
                            if(!originCell->currUnit->Moved())
                                nMainLoopState = S_UNIT_SEL_MOVE_ENTRY;
                            break;
                        }
                        case 1: {
                            nMainLoopState = S_UNIT_SEL_FIGHT_ENTRY; 
                            break;
                        }
                        case 2: {
                            originCell->currUnit->AnimSetRow(0);
                            nMainLoopState = S_DEFAULT_ENTRY;
                            break;
                        }
                    }
                }
                if(GetKey(olc::Key::ESCAPE).bPressed){
                    originCell->currUnit->AnimSetRow(0);
                    bUnitMoving = false;
                    nMainLoopState = S_DEFAULT_ENTRY;
                }
                if(GetKey(olc::Key::UP).bPressed){
                    nMenuCursor--;
                    if(nMenuCursor < 0) nMenuCursor = 2;
                }
                if(GetKey(olc::Key::DOWN).bPressed){
                    nMenuCursor++;
                    if(nMenuCursor > 2) nMenuCursor = 0;
                }

                break;
            }

            case S_UNIT_SEL_MOVE_ENTRY:{
                FindAvailablePath(vCells, originCell, originCell->currUnit->GetMov(), nGridW, nGridH, originCell->currUnit->GetFaction());
                unitMovingUnit = originCell->currUnit;
                bHudUnitMenu = false;

                nMainLoopState = S_UNIT_SEL_MOVE;
                //break;
            }

            case S_UNIT_SEL_MOVE:{
                if(GetKey(olc::Key::ENTER).bPressed && currentCell->bAvailablePath && currentCell->currUnit == nullptr){
                    ResetCells(true,false);
                    viCursor = destinyCell->pos;
                    nMainLoopState = S_UNIT_MOVING_ENTRY;
                }

                if(GetKey(olc::Key::ESCAPE).bPressed){
                    ResetCells(true,true);
                    viCursor = originCell->pos;
                    currentCell = vCells[TDIndex(viCursor,nGridW)];
                    originCell->currUnit->AnimSetRow(0);
                    nMainLoopState = S_UNIT_SELECTED_ENTRY;
                }

                if(keyHeld){
                    UpdateCursor(keyHeld, viCursor);
                    currentCell = vCells[TDIndex(viCursor,nGridW)];
                    ResetCells(false, true);
                    destinyCell = currentCell;
                    if(destinyCell != originCell && destinyCell->currUnit == nullptr && destinyCell->bAvailablePath == true)
                        viMovingPath = Solve_AStar(vCells, originCell, destinyCell, nGridW, nGridH);
                }

                break;
            }

            case S_UNIT_SEL_FIGHT_ENTRY:{
                FindAvailablePath(vCells, originCell, 1, nGridW, nGridH, -1);
                nCombatState = 0;
                unitMovingUnit = originCell->currUnit;
                hudCombatWinL->SetUp(unitMovingUnit->GetMaxHp(), unitMovingUnit->GetHp(), unitMovingUnit->GetFaction(), unitMovingUnit->GetName(), 1, originCell->currUnit->GetDamage());
                hudCombatWinL->Enabled(true);
                bHudUnitMenu = false;
                
                nMainLoopState = S_UNIT_SEL_FIGHT;
                //break;
            }

            case S_UNIT_SEL_FIGHT:{
                if(GetKey(olc::Key::ENTER).bPressed && currentCell->bAvailablePath && currentCell->currUnit != nullptr && currentCell->currUnit->GetFaction() != originCell->currUnit->GetFaction()){
                    ResetCells(true,true);
                        destinyCell = currentCell;
                        viCursor = originCell->pos;
                        originCell->currUnit->AnimSetRow(0);
                        nMainLoopState = S_UNIT_FIGHTING_ENTRY;
                }

                if(GetKey(olc::Key::ESCAPE).bPressed){
                    ResetCells(true,true);
                    hudCombatWinL->Enabled(false);
                    hudCombatWinR->Enabled(false);
                    viCursor = originCell->pos;
                    currentCell = vCells[TDIndex(viCursor,nGridW)];
                    originCell->currUnit->AnimSetRow(0);
                    nMainLoopState = S_UNIT_SELECTED_ENTRY;
                }

                if(keyHeld){
                    UpdateCursor(keyHeld, viCursor);
                    currentCell = vCells[TDIndex(viCursor,nGridW)];
                    if(currentCell->bAvailablePath == true && currentCell->currUnit != nullptr && originCell->currUnit->GetFaction() != currentCell->currUnit->GetFaction()){
                        hudCombatWinR->SetUp(currentCell->currUnit->GetMaxHp(), currentCell->currUnit->GetHp(), currentCell->currUnit->GetFaction(), currentCell->currUnit->GetName(), 1, currentCell->currUnit->GetDamage());
                        hudCombatWinR->Enabled(true);
                    }
                    else{
                        hudCombatWinR->Enabled(false);
                    }
                }

                break;
            }

            case S_UNIT_MOVING_ENTRY:{
                nMainLoopState = S_UNIT_MOVING;
                //break;
            }

            case S_UNIT_MOVING:{
                originCell->currUnit = nullptr;
                bUnitMoving = unitMovingUnit->Move(viMovingPath, vfMovingPos, fMovingMaxT, fMovingMaxTpCell, fElapsedTime, CELL_S);
                if(!bUnitMoving){
                    viMovingPath.clear();
                    destinyCell->currUnit = unitMovingUnit;
                    ResetCells(false, true);
                    nMainLoopState = S_DEFAULT_ENTRY;
                }

                break;
            }

            case S_UNIT_FIGHTING_ENTRY:{
                nMainLoopState = S_UNIT_FIGHTING;
                //break;
            }

            case S_UNIT_FIGHTING:{
                switch(nCombatState){
                    case FIGHT_S_SETUP: {
                        nCombatAttackerTurns = originCell->currUnit->GetAttacks();
                        nCombatOpponentTurns = destinyCell->currUnit->GetAttacks();
                        nCombatActiveUnit = 0;
                        nCombatState = FIGHT_S_ATTACK;
                        break;
                    }
                    case FIGHT_S_ATTACK: {
                        if(nCombatActiveUnit == 0){
                            originCell->currUnit->Attack(destinyCell->currUnit);
                            hudCombatWinR->ChangeHp(destinyCell->currUnit->GetHp());
                        }
                        else{
                            destinyCell->currUnit->Attack(originCell->currUnit);
                            hudCombatWinL->ChangeHp(originCell->currUnit->GetHp());
                        }
                        animSpark->Enable(true);
                        nCombatState = FIGHT_S_ATT_ANIM;
                        break;
                    }
                    case FIGHT_S_ATT_ANIM: { 
                        olc::vf2d sparkPos;
                        if(nCombatActiveUnit == 0){
                            sparkPos = destinyCell->pos * CELL_S;
                            hudCombatWinR->AdvanceTime(fElapsedTime);
                        }
                        else{
                            sparkPos = originCell->pos * CELL_S;
                            hudCombatWinL->AdvanceTime(fElapsedTime);
                        }
                        
                        if(animSpark->AdvanceTime(fElapsedTime)){
                            animSpark->Draw(*this, layerHud, sparkPos);
                        }
                        else{
                            nCombatState = FIGHT_S_AFTER_ATT;
                        }
                        break;
                    }
                    case FIGHT_S_AFTER_ATT: {
                        if(nCombatActiveUnit == 0){
                            nCombatAttackerTurns--;
                            if(destinyCell->currUnit->GetHp() == 0){
                                nCombatState = FIGHT_S_DEATH_ANIM;
                            }
                            else {
                                if(nCombatOpponentTurns > 0){
                                    nCombatActiveUnit = 1;
                                    nCombatState = FIGHT_S_ATTACK;
                                }
                                else if(nCombatAttackerTurns > 0){
                                    nCombatState = FIGHT_S_ATTACK;
                                }
                                else{
                                    nCombatState = FIGHT_S_CLEANUP;
                                }
                            }
                        }
                        else{
                            nCombatOpponentTurns--;
                            if(originCell->currUnit->GetHp() == 0){
                                nCombatState = FIGHT_S_DEATH_ANIM;
                            }
                            else {
                                if(nCombatAttackerTurns > 0){
                                    nCombatActiveUnit = 0;
                                    nCombatState = FIGHT_S_ATTACK;
                                }
                                else if(nCombatOpponentTurns > 0){
                                    nCombatState = FIGHT_S_ATTACK;
                                }
                                else{
                                    nCombatState = FIGHT_S_CLEANUP;
                                }
                            }
                        }
                    }
                    case FIGHT_S_DEATH_ANIM: { 
                        if(nCombatActiveUnit == 0){
                            if(!destinyCell->currUnit->Die(fElapsedTime)){
                                destinyCell->currUnit == nullptr;
                                vCells[TDIndex(destinyCell->pos,nGridW)]->currUnit = nullptr;
                                nCombatState = FIGHT_S_CLEANUP;
                            }
                        }
                        else{
                            if(!originCell->currUnit->Die(fElapsedTime)){
                                originCell->currUnit == nullptr;
                                vCells[TDIndex(originCell->pos,nGridW)]->currUnit = nullptr;
                                nCombatState = FIGHT_S_CLEANUP;
                            }
                        }
                        break;
                    }
                    case FIGHT_S_CLEANUP: { 
                        hudCombatWinL->Enabled(false);
                        hudCombatWinR->Enabled(false);
                        nMainLoopState = S_DEFAULT_ENTRY;
                        break;
                    }
                }

                break;
            }

            case S_UNIT_OPP_SELECTED_ENTRY:{
                FindAvailablePath(vCells, currentCell, currentCell->currUnit->GetMov(), nGridW, nGridH, currentCell->currUnit->GetFaction());
                nMainLoopState = S_DEFAULT;
                break;
            }

            case S_UNIT_OPP_SELECTED:{
                // unused
                break;
            }

            case S_GROUND_SELECTED_ENTRY:{
                bHudGralMenu = true;
                nMenuCursor = 0;
                nMainLoopState = S_GROUND_SELECTED;
                //break;
            }

            case S_GROUND_SELECTED:{
                if(GetKey(olc::Key::ENTER).bPressed){
                    switch(nMenuCursor){
                        case 0: { // end turn
                            nMainLoopState = S_TURN_TRANSITION_ENTRY;
                            break;
                        }
                        case 1: { // dummy
                            break;
                        }
                        case 2: { // return
                            bHudGralMenu = false;
                            nMainLoopState = S_DEFAULT_ENTRY;
                            break;
                        }
                    }
                }
                if(GetKey(olc::Key::ESCAPE).bPressed){
                    bHudGralMenu = false;
                    nMainLoopState = S_DEFAULT_ENTRY;
                }
                if(GetKey(olc::Key::UP).bPressed){
                    nMenuCursor--;
                    if(nMenuCursor < 0) nMenuCursor = 2;
                }
                if(GetKey(olc::Key::DOWN).bPressed){
                    nMenuCursor++;
                    if(nMenuCursor > 2) nMenuCursor = 0;
                }

                break;
            }

            case S_TURN_TRANSITION_ENTRY:{
                nScreenTransState = 0;
                bHudGralMenu = false;
                for(auto& unit: vUnits){
                    unit->ResetTurn();
                }

                nMainLoopState = S_TURN_TRANSITION;
                //break;
            }

            case S_TURN_TRANSITION:{
                switch(nScreenTransState){
                    case TRAN_S_SETUP:{
                        fScreenTransTime = 0.0;
                        nScreenTransState = TRAN_S_SCROLL_IN;
                        break;
                    }
                    case TRAN_S_SCROLL_IN:{
                        SetDrawTarget(layerHud);
                        Clear(olc::Pixel(16,16,16,128));
                        fScreenTransTime += fElapsedTime;
                        SetDrawTarget(0,true);
                        vfMarqueePos.x = SCREEN_W - (SCREEN_W - (SCREEN_W/2 - 37)) * cubic_lut_3_0.at((int)(100*fScreenTransTime/1.0));
                        vfMarqueePos.y = SCREEN_H/2 - 8;
                        DrawPartialDecal(vfMarqueePos, dclMarquee, olc::vf2d(), olc::vf2d(74,17));
                        if (fScreenTransTime > 1.0){
                            fScreenTransTime = 0.0;
                            nScreenTransState = TRAN_S_PAUSE;
                        }
                        break;
                    }
                    case TRAN_S_PAUSE:{
                        SetDrawTarget(0,true);
                        DrawPartialDecal(vfMarqueePos, dclMarquee, olc::vf2d(), olc::vf2d(74,17));
                        fScreenTransTime += fElapsedTime;
                        if (fScreenTransTime > 1.0){
                            fScreenTransTime = 0.0;
                            nScreenTransState = TRAN_S_SCROLL_OUT;
                        }
                        break;
                    }
                    case TRAN_S_SCROLL_OUT:{
                        SetDrawTarget(layerHud);
                        Clear(olc::Pixel(16,16,16,128));
                        fScreenTransTime += fElapsedTime;
                        SetDrawTarget(0,true);
                        vfMarqueePos.x = -74 + ((SCREEN_W/2 - 37) + 74) * cubic_lut_3_0.at((int)(100*(1-fScreenTransTime/1.0)));
                        vfMarqueePos.y = SCREEN_H/2 - 8;
                        DrawPartialDecal(vfMarqueePos, dclMarquee, olc::vf2d(), olc::vf2d(74,17));
                        if (fScreenTransTime > 1.0){
                            fScreenTransTime = 0.0;
                            nScreenTransState = TRAN_S_CLEANUP;
                        }
                        break;
                    }
                    case TRAN_S_CLEANUP:{
                        SetDrawTarget(layerHud);
                        Clear(olc::BLANK);
                        nMainLoopState = S_DEFAULT;
                        break;
                    }
                }
            }
        }
        

        if((viCursor.y*16+fPanY) >= (SCREEN_H - nCursorMarginH)) { fPanY = SCREEN_H - nCursorMarginH - 16 - viCursor.y*16; }
        if((viCursor.y*16+fPanY) <= nCursorMarginH) { fPanY = nCursorMarginH - viCursor.y*16; }
        if((viCursor.x*16+fPanX) >= (SCREEN_W - nCursorMarginV)) { fPanX = SCREEN_W - nCursorMarginV - 16 - viCursor.x*16; }
        if((viCursor.x*16+fPanX) <= nCursorMarginV) { fPanX = nCursorMarginV - viCursor.x*16; }
        if(fPanX > 0.0f) fPanX = 0.0f;
        if(fPanY > 0.0f) fPanY = 0.0f;
        if(fPanX < (SCREEN_W  - nGridW*CELL_S)) fPanX = (SCREEN_W  - nGridW*CELL_S);
        if(fPanY < (SCREEN_H  - nGridH*CELL_S)) fPanY = (SCREEN_H  - nGridH*CELL_S);
        vfPanNew = {fPanX,fPanY};

        if(vfPanNew != vfPanPrevious){
            if(bPanActive)
                vfPanOrigin = vfPan;
            else
                vfPanOrigin = vfPanPrevious;
            vfPanPrevious = vfPanNew;
            bPanActive = true;
            fPanT = fPanDuration;
        }
        else{
            vfPan = vfPanNew;
        }

        if(bPanActive){
            if(fPanT <= 0.0){
                vfPan = vfPanNew;
                bPanActive = false;
            }
            else{
                fPanT -= fElapsedTime;
                vfPan = ((vfPanNew - vfPanOrigin) * cubic_lut_3_0.at((int)((fPanDuration - fPanT) * 100.0 / fPanDuration))) + vfPanOrigin;
            }
        }
    

        // Update animations //
        
        for(cUnit* unit: vUnits) { unit->AnimAdvance(fElapsedTime); }
        animCursor->AdvanceTime(fElapsedTime);

        // Drawing to screen //
        for(int x = 0; x < nGridW; x++){
            for(int y = 0; y < nGridH; y++){
                olc::vf2d pos = olc::vf2d(x,y)*CELL_S;
                pos += vfPan;
                int index = x+y*nGridW;

                SetDrawTarget(layerMap);
                DrawPartialDecal(pos, CELL_VFS, dclTilemap, (vCells[index]->tile), CELL_VFS);
                
                if(vCells[index]->bAvailablePath && vCells[index] != originCell){
                    DrawPartialDecal(pos, CELL_VFS, dclTilemap, {16.0,16.0}, CELL_VFS, olc::Pixel(255,64,64,128));
                }
                
                if(vCells[index]->nPath != -1){
                    DrawPartialDecal(pos, CELL_VFS, dclArrowmap, {vCells[index]->nPath*16.0,0.0}, CELL_VFS);
                }

                if(vCells[index]->currUnit != nullptr) {
                    olc::Pixel c(255,255,255);
                    if(vCells[index]->currUnit->Attacked() && vCells[index]->currUnit->GetFaction() == 0 && nMainLoopState != S_UNIT_FIGHTING) c = olc::Pixel(128,128,128);
                    vCells[index]->currUnit->Draw(*this, layerMain, pos, c);
                }
            }
        }

        if(bUnitMoving) unitMovingUnit->Draw(*this, layerMain, vfMovingPos+vfPan);
        animCursor->Draw(*this, layerMain, viCursor*CELL_S+vfPan+vfCursorSprOffs);
        
        // HUD //
        const int windowWidth = 3;
        const int windowHeight = 4;
        const olc::Pixel textColor(208,214,224);
        const olc::Pixel textColorAlt(64,64,64);
        const olc::Pixel redWinColor(255,64,64);
        
        if((viCursor.y*16+fPanY) < ScreenHeight()/3) { vfHudWinPos.y = ScreenHeight() - windowHeight*16 - 10; }
        else if((viCursor.y*16+fPanY) > ScreenHeight()*2/3){ vfHudWinPos.y = 0; }

        if((viCursor.x*16+fPanX) < ScreenWidth()/3) { vfHudWinPos.x = ScreenWidth() - windowWidth*16 - 10; }
        else if((viCursor.x*16+fPanX) > ScreenWidth()*2/3){ vfHudWinPos.x = 0; }
        
        DrawHudWin(*this, dclWindow, vfHudWinPos, {windowWidth,windowHeight}, vfHudWinAlign);
        SetDrawTarget(layerHud);
        std::string textCurrent;

        olc::vf2d textPos = vfHudWinPos + olc::vf2d(5,5);
        if(bHudUnitMenu){
            DrawHudStr(*this, dclFont, fontFfs, ">", textPos+olc::vi2d(-1,nMenuCursor * 10), 1, olc::vi2d(), textColor);
            textPos.x += 8;
            DrawHudStr(*this, dclFont, fontFfs, "MOVE", textPos, 5, olc::vi2d(), currentCell->currUnit->Moved() ? textColorAlt : textColor);
            textPos.y += 10;
            DrawHudStr(*this, dclFont, fontFfs, "FIGHT", textPos, 5, olc::vi2d(), textColor);
            textPos.y += 10;
            DrawHudStr(*this, dclFont, fontFfs, "BACK", textPos, 5, olc::vi2d(), textColor);
        }
        else if (bHudGralMenu){
            DrawHudStr(*this, dclFont, fontFfs, ">", textPos+olc::vi2d(-1,nMenuCursor * 10), 1, olc::vi2d(), textColor);
            textPos.x += 8;
            DrawHudStr(*this, dclFont, fontFfs, "END", textPos, 5, olc::vi2d(), textColor);
            textPos.y += 10;
            DrawHudStr(*this, dclFont, fontFfs, "STATS", textPos, 5, olc::vi2d(), textColorAlt);
            textPos.y += 10;
            DrawHudStr(*this, dclFont, fontFfs, "BACK", textPos, 5, olc::vi2d(), textColor);
        }
        else {
            int cursorPos = TDIndex(viCursor,nGridW);
            textCurrent = std::to_string(viCursor.x).append(",").append(std::to_string(viCursor.y));
            DrawHudStr(*this, dclFont, fontFfs, textCurrent, textPos, 6, {0,0}, textColor);
            textPos.y += 10;
            textCurrent = std::to_string((vCells[cursorPos]->tile.x+vCells[cursorPos]->tile.y*nTilemapW)/16);
            DrawHudStr(*this, dclFont, fontFfs, textCurrent, textPos+olc::vf2d(48,0), 6, {2,0}, textColor);
            textPos.y += 10;
            DrawPartialDecal(textPos, CELL_VFS, dclTilemap, (vCells[cursorPos]->tile), CELL_VFS);
            //textPos.y += 2;
            if(vCells[cursorPos]->currUnit != nullptr){
                vCells[cursorPos]->currUnit->Draw(*this, textPos+olc::vf2d(32,0));
                textPos.y += 18;
                DrawHudStr(*this, dclFont, fontFfs, vCells[cursorPos]->currUnit->GetName(), textPos, 6, {0,0}, textColor);
                textPos.y += 10;
                textPos.x += 3*16;
                DrawHudStr(*this, dclFont, fontFfs, std::to_string(vCells[cursorPos]->currUnit->GetHp()).append("/").append(std::to_string(vCells[cursorPos]->currUnit->GetMaxHp())), textPos, 6, {2,0}, textColor);
            }
        }

        hudCombatWinL->Draw(*this, dclWindow, dclFont, fontFfs);
        hudCombatWinR->Draw(*this, dclWindow, dclFont, fontFfs);

        return true;
    }
};

int main(){
    ENGINE oEngine;
    if(oEngine.Construct(SCREEN_W, SCREEN_H, PIXEL_S, PIXEL_S))
        oEngine.Start();
    
    return 0;
}