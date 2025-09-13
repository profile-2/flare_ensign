#include "OneLoneCoder/olcPixelGameEngine.h"

#ifndef P2_HUD
#define P2_HUD

void DrawHudWin(olc::PixelGameEngine& pge, olc::Decal* decal, olc::vf2d pos, const olc::vf2d& size, const olc::vi2d& anchor, 
        olc::vf2d sourceSize, const int& leftMargin, const int& rightMargin, const int& topMargin, const int& bottomMargin, const olc::Pixel color = olc::WHITE){
    olc::vf2d source_pos(0,0);
    //olc::vf2d source_size(26,25);
    const float middle_width = (sourceSize.x-leftMargin)-rightMargin;
    const float middle_height = (sourceSize.y-topMargin)-bottomMargin;
    const float width = leftMargin+rightMargin+middle_width*size.x;
    const float height = topMargin+bottomMargin+middle_height*size.y;

    if(anchor.x == 1) pos.x -= width/2;
    else if(anchor.x == 2) pos.x -= width;
    if(anchor.y == 1) pos.y -= height/2;
    else if(anchor.y == 2) pos.y -= height;

    pge.DrawPartialDecal(pos, decal, {0,0}, {leftMargin,topMargin}, olc::vf2d(1,1), color);
    for(int x = 0; x < size.x; x++)
        pge.DrawPartialDecal(pos+olc::vf2d(x*middle_width+leftMargin,0), decal, {leftMargin,0}, {middle_width,topMargin}, olc::vf2d(1,1), color);
    pge.DrawPartialDecal(pos+olc::vf2d(size.x*middle_width+leftMargin,0), decal, {leftMargin+middle_width,0}, {rightMargin,topMargin}, olc::vf2d(1,1), color);

    for(int y = 0; y < size.y; y++){
        pge.DrawPartialDecal(pos+olc::vf2d(0,y*middle_height+topMargin), decal, {0,topMargin}, {leftMargin,middle_height}, olc::vf2d(1,1), color);
        for(int x = 0; x < size.x; x++)
            pge.DrawPartialDecal(pos+olc::vf2d(x*middle_width+leftMargin,y*middle_height+topMargin), decal, {leftMargin,topMargin}, {middle_width,middle_height}, olc::vf2d(1,1), color);
        pge.DrawPartialDecal(pos+olc::vf2d(size.x*middle_width+leftMargin,y*middle_height+topMargin), decal, {leftMargin+middle_width,topMargin}, {rightMargin,middle_height}, olc::vf2d(1,1), color);
    }

    pge.DrawPartialDecal(pos+olc::vf2d(0,size.y*middle_height+topMargin), decal, {0,topMargin+middle_height}, {leftMargin,bottomMargin}, olc::vf2d(1,1), color);
    for(int x = 0; x < size.x; x++)
        pge.DrawPartialDecal(pos+olc::vf2d(x*middle_width+leftMargin,size.y*middle_height+topMargin), decal, {leftMargin,bottomMargin+middle_height}, {middle_width,bottomMargin}, olc::vf2d(1,1), color);
    pge.DrawPartialDecal(pos+olc::vf2d(size.x*middle_width+leftMargin,size.y*middle_height+topMargin), decal, {leftMargin+middle_width,bottomMargin+middle_height}, {rightMargin,bottomMargin}, olc::vf2d(1,1), color);
}

// simplified version to use with window_frame_blue.png
void DrawHudWin(olc::PixelGameEngine& pge, olc::Decal* decal, olc::vf2d pos, const olc::vf2d& size, const olc::vi2d& anchor = olc::vi2d(), const olc::Pixel color = olc::WHITE){
    DrawHudWin(pge, decal, pos, size, anchor,{26,26}, 5, 5, 5, 5, color);
}

struct sFont{
    std::string map; // List of characters in the font tileset in the order they appear
    olc::vi2d grid_size;
    olc::vi2d cell_size;
};

void DrawHudStr(olc::PixelGameEngine& pge, olc::Decal* decal, const sFont& font, const std::string& text, olc::vf2d pos, const int length, const olc::vi2d& anchor = olc::vi2d(0,0), const olc::Pixel tint = olc::WHITE){
    float width = length*font.cell_size.x;
    if(text.size() < length) width = text.size()*font.cell_size.x;
    const float height = font.cell_size.y;

    if(anchor.x == 1) pos.x -= width/2;
    else if(anchor.x == 2) pos.x -= width;
    if(anchor.y == 1) pos.y -= height/2;
    else if(anchor.y == 2) pos.y -= height;

    for(int f = 0; f < text.length() && f < length; f++){
        size_t charPos = font.map.find(text[f]);
        olc::vf2d letter = olc::vf2d(charPos%font.grid_size.x,charPos/font.grid_size.x)*font.cell_size;
        pge.DrawPartialDecal(pos+olc::vf2d(f*font.cell_size.x,0), decal, letter, font.cell_size, {1,1}, tint);
    }
}

class cCombatInfoWin{
private:
    olc::vf2d vfPos;
    olc::vi2d viSize;
    float fAnimationT;
    float fCurrT;
    int nMaxHp;
    int nCurrHp;
    int nLastHp;
    int nNewHp;
    int nDisplayedHp;
    int nFaction;
    std::string sUnitName;
    int nUnitAttacks;
    int nUnitDamage;
    int nSide;
    bool bFinished;
    olc::Sprite* spr;
    olc::Decal* dcl;
    bool bEnabled;

public:
    cCombatInfoWin(olc::vf2d vfPOS, int nSIDE): vfPos(vfPOS), nSide(nSIDE) {
        fAnimationT = 1.0;
        fCurrT = 0.0;
        bFinished = false;
        viSize = olc::vi2d(8,1);
        spr = new olc::Sprite("./assets/bars.png");
        dcl = new olc::Decal(spr);
        bEnabled = false;
    }

    ~cCombatInfoWin(){
        delete dcl;
        delete spr;
    }
    
    void SetUp(int nMAX_HP, int nCURR_HP, int nFACTION, std::string sNAME, int nATTACKS, int nDAMAGE){
        nDisplayedHp = 10.0 * (float)nCURR_HP / (float)nMAX_HP;
        nMaxHp = nMAX_HP;
        nCurrHp = nCURR_HP;
        nFaction = nFACTION;
        sUnitName = sNAME;
        nUnitAttacks = nATTACKS;
        nUnitDamage = nDAMAGE;
    }

    void ChangeHp(int nNEW_HP){
        nLastHp = nCurrHp;
        nNewHp = nNEW_HP;
        fCurrT = 0.0;
        nDisplayedHp = 10.0 * (float)nCurrHp / (float)nMaxHp;
    }

    bool AdvanceTime(float fTIME){
        fCurrT += fTIME;
        if(fCurrT < 0.45){
            nCurrHp = nLastHp - (nLastHp - nNewHp) * fCurrT / 0.45;
            nDisplayedHp = 10.0 * (float)nCurrHp / (float)nMaxHp;
            return false;
        }
        else{
            nCurrHp = nNewHp;
            nDisplayedHp = 10.0 * (float)nCurrHp / (float)nMaxHp;
            return true;
        }
    }

    bool Draw(olc::PixelGameEngine& pge, olc::Decal* decalWin, olc::Decal* decalFont, sFont font){
        // 0,0:6,8 6,0:4,8 10,0:5,8 // 45,0:6:8 51,0:4:8 55,0:5,8
        if(bEnabled){
            olc::Pixel winColor = nFaction ? olc::Pixel(255,64,64) : olc::WHITE;
            olc::vf2d line1Offs = olc::vf2d(5,5);
            olc::vf2d line2Offs = olc::vf2d(7,14);
            olc::vf2d line3Offs = olc::vf2d(5,25);
            DrawHudWin(pge, decalWin, vfPos, viSize, olc::vi2d(0,0), winColor);
            pge.DrawPartialDecal(vfPos+line2Offs+olc::vf2d(41,0), dcl, olc::vf2d(0,0), olc::vf2d(6,8));
            if(nDisplayedHp > 0) pge.DrawPartialDecal(vfPos+line2Offs+olc::vf2d(41,0), dcl, olc::vf2d(45,0), olc::vf2d(6,8));
            for(int x = 0; x < 8; x++){
                pge.DrawPartialDecal(vfPos+line2Offs+olc::vf2d(41+6+4*x,0), dcl, olc::vf2d(6,0), olc::vf2d(4,8));
                if(nDisplayedHp > x+1) pge.DrawPartialDecal(vfPos+line2Offs+olc::vf2d(41+6+4*x,0), dcl, olc::vf2d(51,0), olc::vf2d(4,8));
            }
            pge.DrawPartialDecal(vfPos+line2Offs+olc::vf2d(41+6+4*8,0), dcl, olc::vf2d(10,0), olc::vf2d(5,8));
            if(nDisplayedHp == 10) pge.DrawPartialDecal(vfPos+line2Offs+olc::vf2d(41+6+4*8,0), dcl, olc::vf2d(55,0), olc::vf2d(5,8));
            
            DrawHudStr(pge, decalFont, font, sUnitName, vfPos+line1Offs, 16);
            DrawHudStr(pge, decalFont, font, std::string("AT:").append(std::to_string(nUnitDamage)), vfPos+line2Offs, 5);
            DrawHudStr(pge, decalFont, font, std::to_string(nCurrHp).append("/").append(std::to_string(nMaxHp)), vfPos+line2Offs+olc::vf2d(43+41+1,0), 5);
        }
        return true;
    }

    void Enabled(bool enable = true) { bEnabled = enable; }
};
#endif