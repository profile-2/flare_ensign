#include "OneLoneCoder/olcPixelGameEngine.h"
#include "nlohmann/json.hpp"

#ifndef P2_CANIM
#define P2_CANIM

enum enumFlag{  ANIM_NONE        = 0,
                ANIM_SINGLE      = 1,
                ANIM_SWING       = 2,
                ANIM_LOOP        = 4,
                ANIM_COMPLETE    = 8,
                ANIM_REVERSE     = 16,
                ANIM_DISABLE_IF_COMPLETE = 32//,
                //FLAG6       = 64,
                //FLAG7       = 128
                }; 

struct sAnimDesc{               // animation description
    std::vector<float> times;   // time in seconds of each frame of animation
    UCHAR flag;                 // use enumFlag
    sAnimDesc(std::vector<float> t, UCHAR f = ANIM_SWING): times(t), flag(f){};

    explicit sAnimDesc(const nlohmann::json& data) {    // required to acquire data from the jobs JSON
        times = data.at("times").get<std::vector<float>>();
        flag = data.at("flag").get<int>();
    }
};


class cAnim {
private:
    olc::Decal* decal;      // the decal organized in rows, each row a series of frames of animation
    olc::vi2d viGridSize;   // of the animation tile set
    olc::vf2d vfCellSize;
    olc::vf2d vfDclOffset;
    std::vector<sAnimDesc> vaRowDesc;
    int nCurrFrame;
    int nCurrRow;           
    float fCurrTime;
    int nFacing;
    bool bEnabled;

public:
    cAnim(olc::Decal* DECAL, const olc::vi2d& viGRIDSIZE, const olc::vf2d& vfCELLSIZE, const olc::vf2d& vfOFFSET, 
        const std::vector<sAnimDesc>& vANIMDESC, int nFACING = 1,bool bENABLE = true)
        : decal(DECAL), viGridSize(viGRIDSIZE), vfCellSize(vfCELLSIZE), vfDclOffset(vfOFFSET), nFacing(nFACING), bEnabled(bENABLE) {
        if (!DECAL || vANIMDESC.empty() || viGRIDSIZE.y != static_cast<int>(vANIMDESC.size()) ||
            viGRIDSIZE.x <= 0 || viGRIDSIZE.y <= 0 || vfCELLSIZE.x <= 0.0f || vfCELLSIZE.y <= 0.0f) {
            vaRowDesc.clear();
            nCurrFrame = 0;
            nCurrRow = 0;
            fCurrTime = 0.0f;
            bEnabled = false;
            return;
        }
        for (const auto& desc : vANIMDESC) {
            if (desc.times.empty() || desc.times.size() > static_cast<size_t>(viGRIDSIZE.x)) {
                vaRowDesc.clear();
                nCurrFrame = 0;
                nCurrRow = 0;
                fCurrTime = 0.0f;
                bEnabled = false;
                return;
            }
        }
        vaRowDesc.reserve(vANIMDESC.size());
        vaRowDesc = vANIMDESC;
        nCurrFrame = 0;
        nCurrRow = 0;
        fCurrTime = 0.0f;
    }

    // for decals with a single row of animation
    cAnim(olc::Decal* DECAL, int nFRAMES, const olc::vf2d& vfCELLSIZE, const olc::vf2d& vfOFFSET, const sAnimDesc& adDESC, int nFACING = 1, 
        bool bENABLE = true)
        : decal(DECAL), vfCellSize(vfCELLSIZE), vfDclOffset(vfOFFSET), nFacing(nFACING), bEnabled(bENABLE) {
        if (!DECAL || nFRAMES <= 0 || vfCELLSIZE.x <= 0.0f || vfCELLSIZE.y <= 0.0f) {
            vaRowDesc.clear();
            nCurrFrame = 0;
            nCurrRow = 0;
            fCurrTime = 0.0f;
            bEnabled = false;
            return;
        }
        if (adDESC.times.empty() || adDESC.times.size() > static_cast<size_t>(nFRAMES)) {
            vaRowDesc.clear();
            nCurrFrame = 0;
            nCurrRow = 0;
            fCurrTime = 0.0f;
            bEnabled = false;
            return;
        }
        viGridSize = {nFRAMES, 1};
        vaRowDesc.reserve(1);
        vaRowDesc.emplace_back(adDESC);
        nCurrFrame = 0;
        nCurrRow = 0;
        fCurrTime = 0.0f;
    }
    
    bool AdvanceTime(float fTime){
        if (!bEnabled || vaRowDesc.empty() || nCurrRow < 0 || nCurrRow >= viGridSize.y || vaRowDesc[nCurrRow].times.empty()) {
            return bEnabled;
        }
        fCurrTime += fTime;
        if(fCurrTime >= vaRowDesc[nCurrRow].times[nCurrFrame]){
            fCurrTime -= vaRowDesc[nCurrRow].times[nCurrFrame];
            if(vaRowDesc[nCurrRow].flag&ANIM_LOOP){
                nCurrFrame++;
                if(nCurrFrame >= vaRowDesc[nCurrRow].times.size()){
                    nCurrFrame = 0;
                }
            }
            else if(vaRowDesc[nCurrRow].flag&ANIM_SINGLE && !(vaRowDesc[nCurrRow].flag&ANIM_COMPLETE)){
                nCurrFrame++;
                if(nCurrFrame >= vaRowDesc[nCurrRow].times.size()){
                    nCurrFrame = vaRowDesc[nCurrRow].times.size() - 1;
                    vaRowDesc[nCurrRow].flag |= ANIM_COMPLETE;
                    if(vaRowDesc[nCurrRow].flag&ANIM_DISABLE_IF_COMPLETE)
                    bEnabled = false;
                }
            }
            else if(vaRowDesc[nCurrRow].flag&ANIM_SWING){
                if(vaRowDesc[nCurrRow].flag&ANIM_REVERSE)
                nCurrFrame--;
                else 
                nCurrFrame++;
                if(nCurrFrame >= vaRowDesc[nCurrRow].times.size() && !(vaRowDesc[nCurrRow].flag&ANIM_REVERSE)){
                    nCurrFrame = vaRowDesc[nCurrRow].times.size() - 1;
                    vaRowDesc[nCurrRow].flag |= ANIM_REVERSE;
                }
                else if(nCurrFrame <= 0 && vaRowDesc[nCurrRow].flag&ANIM_REVERSE){
                    nCurrFrame = 0;
                    vaRowDesc[nCurrRow].flag &= ~ANIM_REVERSE;
                }
            }
        }
        return bEnabled;
    }
    
    void Draw(olc::PixelGameEngine& pge, olc::vf2d vfPos, olc::Pixel pColor = olc::WHITE) {
        if (decal && bEnabled) {
            if(nFacing<0) vfPos = {vfPos.x+vfCellSize.x,vfPos.y};
            int facing = nFacing<0 ? -1 : 1;
            pge.DrawPartialDecal(vfPos+vfDclOffset, decal, {nCurrFrame*vfCellSize.x,nCurrRow*vfCellSize.y},vfCellSize,{facing,1}, pColor);
        }
    }
    
    void Draw(olc::PixelGameEngine& pge, int nLayer, olc::vf2d vfPos, olc::Pixel pColor = olc::WHITE) {
        if (decal && bEnabled) {
            if(nFacing<0) vfPos = {vfPos.x+vfCellSize.x,vfPos.y};
            int facing = nFacing<0 ? -1 : 1;
            pge.SetDrawTarget(nLayer);
            pge.DrawPartialDecal(vfPos+vfDclOffset, decal, {nCurrFrame*vfCellSize.x,nCurrRow*vfCellSize.y},vfCellSize,{facing,1}, pColor);
        }
    }
    
    void SetDecal(olc::Decal* Decal){
        decal = Decal;
    }
    
    bool SetRow(int nRow){ // change animation
        if(nRow >= 0 && nRow < viGridSize.y && !vaRowDesc.empty()){
            nCurrRow = nRow;
            nCurrFrame = 0;
            fCurrTime = 0.0;
            return true;
        }
        return false;
    }
    int GetRow() { return nCurrRow; }
    int GetMaxRows() { return viGridSize.y; }

    void SetFacing(int facing) { nFacing = facing; }
    int GetFacing() { return nFacing; }

    float GetCurrTime() { return fCurrTime; }

    void Enable(bool enable = true){ 
        bEnabled = enable; 
        if(bEnabled){
            nCurrFrame = 0;
            fCurrTime = 0.0;
            vaRowDesc[nCurrRow].flag &= ~ANIM_COMPLETE;
        }
    }
    bool IsEnabled(){ return bEnabled; }
};

#endif