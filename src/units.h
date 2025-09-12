#include "OneLoneCoder/olcPixelGameEngine.h"
#include "nlohmann/json.hpp"
#include "animation.h"

#ifndef P2_UNITS
#define P2_UNITS

struct sCell;

std::map<std::string, int> mJobs {
    {"FIGHTER", 0},
    {"ROGUE", 1},
    {"MAGE", 2},
};

class cJob{
protected:
    std::string sJobName;
    int nMovement;
    int nMovementType;
    int nMaxHp;
    int nAttackType;
    int nStr;
    int nDex;
    int nInt;
    int nSpr;
    cAnim* animation;
    int nFactionAnimRows;

public:

    cJob(std::string sJOB_NAME, int nMOV, int nMOV_TYPE, int nHP, int nATT_TYPE, int nSTR, int nDEX, int nINT, int nSPR, cAnim* ANIM = nullptr): 
        sJobName(sJOB_NAME), nMovement(nMOV), nMovementType(nMOV_TYPE), nMaxHp(nHP), nAttackType(nATT_TYPE), nStr(nSTR), nDex(nDEX), nInt(nINT), nSpr(nSPR) {
            if(ANIM) { 
                animation = new cAnim(*ANIM); 
                nFactionAnimRows = animation->GetMaxRows()/2;
            }
            else { 
                animation = nullptr; 
                nFactionAnimRows = 0;
            }
    }

    ~cJob(){
        delete animation;
    }

    std::string GetName() const { return sJobName; }
    int GetMov() const { return nMovement; }
    int GetMovType() const { return nMovementType; }
    int GetHp() const { return nMaxHp; }
    int GetAtt() const { return nAttackType; }
    int GetStr() const { return nStr; }
    int GetDex() const { return nDex; }
    int GetInt() const { return nInt; }
    int GetSpr() const { return nSpr; }
    cAnim* GetAnim() const { return animation; }
};

class cUnit: public cJob{
private:
    std::string sName;
    int nLevel;
    int nFaction;
    olc::vi2d viPos;
    int nCurrHp;
    int nAttPerTurn;
    int nDamage;

    bool bMoving;
    std::vector<olc::vi2d>::iterator iMovingIndex;
    olc::vf2d vfMovingDestiny;
    olc::vf2d vfMovingOrigin;
    float fMovingTimer;
    float fMovingTimerCell;
    int nCounter;

    bool bMoved;
    bool bAttacked;

public:
    cUnit(const std::string& sNAME, const cJob& JOB, int nLEVEL, int nFACTION, const olc::vi2d& viPOS): 
    cJob(JOB.GetName(), JOB.GetMov(), JOB.GetMovType(), JOB.GetHp(), JOB.GetAtt(),
    JOB.GetStr(), JOB.GetDex(), JOB.GetInt(), JOB.GetSpr(), JOB.GetAnim()), sName(sNAME), nLevel(nLEVEL), nFaction(nFACTION), viPos(viPOS){
        this->AnimSetRow(0); // hack to load the correct animation for the faction
        bMoving = false;
        nCurrHp = nMaxHp;

        if(nAttackType == 2) nAttPerTurn = 2;
        else nAttPerTurn = 1;

        if(nAttackType == 10) nDamage = nInt;
        else nDamage = nStr;
        
        nCounter = 10;

        bMoved = false;
        bAttacked = false;
    }

    std::string GetName() { return sName; }
    void SetFaction(int faction) { nFaction = faction; }
    int GetFaction() { return nFaction; }
    olc::vi2d GetPos() { return viPos; }
    std::string GetJobName() { return sJobName; }
    int GetMaxHp() { return nMaxHp; }

    bool SetHp(int nHP) {
        if(nHP > nMaxHp) { 
            nCurrHp = nMaxHp; 
        }
        else { 
            nCurrHp = nHP; 
        }

        if(nCurrHp <= 0){
            nCurrHp = 0;
            return false;
        }
        else {
            return true;
        }
    }

    int GetHp() { return nCurrHp; }
    int GetAttacks() { return nAttPerTurn; }
    int GetDamage() { return nDamage; }

    bool AnimSetRow(int nRow) { return animation->SetRow(nRow+(nFaction*nFactionAnimRows)); }
    int AnimGetRow() { return animation->GetRow(); }
    bool AnimAdvance(float fTime) { return animation->AdvanceTime(fTime); }
    void Draw(olc::PixelGameEngine& pge, olc::vf2d vfPos, olc::Pixel pColor = olc::WHITE) { animation->Draw(pge, vfPos, pColor); }
    void Draw(olc::PixelGameEngine& pge, int nLayer, olc::vf2d vfPos, olc::Pixel pColor = olc::WHITE) { animation->Draw(pge, nLayer, vfPos, pColor); }
    void AnimSetDecal(olc::Decal* Decal) { animation->SetDecal(Decal); }
    void AnimSetFacing(int nFacing) { animation->SetFacing(nFacing); }
    int AnimGetFacing() { return animation->GetFacing(); }
    void AnimEnable(bool bEnable = true) { animation->Enable(bEnable); }
    bool AnimIsEnable() { return animation->IsEnabled(); }

    bool Move(std::vector<olc::vi2d>& viMovingPath, olc::vf2d& vfMovingPos, const float& fMovingTimerMax, const float& fMovingTimerCellMax, const float& fElapsedTime, float fCellSize){
        if(!bMoving){
            iMovingIndex = viMovingPath.end() - 1;
            vfMovingOrigin = *iMovingIndex;
            vfMovingDestiny = *(iMovingIndex-1);
            
            vfMovingPos = vfMovingOrigin*fCellSize;
            AnimSetRow(1);

            fMovingTimer = 0.0;
            fMovingTimerCell = fMovingTimerMax / viMovingPath.size();
            if(fMovingTimerCell > fMovingTimerCellMax) fMovingTimerCell = fMovingTimerCellMax;
            bMoving = true;
        }

        if(iMovingIndex != viMovingPath.begin()){
            fMovingTimer += fElapsedTime;
            if(fMovingTimer < fMovingTimerCell){
                vfMovingPos = fCellSize*vfMovingOrigin.lerp(vfMovingDestiny, fMovingTimer/fMovingTimerCell);
                if(vfMovingDestiny.x > vfMovingOrigin.x) AnimSetFacing(1);
                else if(vfMovingDestiny.x < vfMovingOrigin.x) AnimSetFacing(-1);
            }
            else{
                iMovingIndex--;
                vfMovingPos = vfMovingDestiny*fCellSize;
                vfMovingOrigin = vfMovingDestiny;
                vfMovingDestiny = *(iMovingIndex-1);
                fMovingTimer = 0.0;
            }

            return true;
        }
        else{
            bMoving = false;
            AnimSetRow(0);
            AnimSetFacing(1);
            bMoved = true;
            return false;
        }
    }

    bool Attack(cUnit* target) {
        bAttacked = true;
        bMoved = true;
        return target->SetHp(target->GetHp() - nDamage);
    }

    bool Moved() { return bMoved; }
    bool Attacked()  {return bAttacked; }

    void ResetTurn() {
        bMoved = false;
        bAttacked = false;
    }

    bool Die(float Time) {
        fMovingTimer += Time; // reusing fMovingtimer
        if(fMovingTimer >= 0.1){
            if(animation->IsEnabled()) animation->Enable(false);
            else animation->Enable(true);
            fMovingTimer = 0.0;
            nCounter--;
        }

        if(nCounter <= 0) 
            return false;
        else
            return true;
    }
};

struct sjJob {
    std::string name;
    std::string sprite_path;
    int grid_size_x;
    int grid_size_y;
    int cell_size_x;
    int cell_size_y;
    int offset_x;
    int offset_y;
    std::vector<sAnimDesc> anim_desc;
    int movement;
    int mobility_type;
    int hp;
    int attack_type;
    int str;
    int dex;
    int int_;
    int spr;

    explicit sjJob(const nlohmann::json& data) {
        name = data.at("name").get<std::string>();
        sprite_path = data.at("sprite_path").get<std::string>();
        grid_size_x = data.at("grid_size_x").get<int>();
        grid_size_y = data.at("grid_size_y").get<int>();
        cell_size_x = data.at("cell_size_x").get<int>();
        cell_size_y = data.at("cell_size_y").get<int>();
        offset_x = data.value("offset_x", 0);
        offset_y = data.value("offset_y", 0);
        for (const auto& item : data.at("anim_desc")) {
            anim_desc.emplace_back(item);
        }
        movement = data.at("movement").get<int>();
        mobility_type = data.at("mobility_type").get<int>();
        hp = data.at("hp").get<int>();
        attack_type = data.at("attack_type").get<int>();
        str = data.at("str").get<int>();
        dex = data.at("dex").get<int>();
        int_ = data.at("int").get<int>();
        spr = data.at("spr").get<int>();
    }
};

int ParseJobsJSON(const std::string& sFile, std::vector<cJob*>& vJobs, 
        std::vector<olc::Sprite*>& vSprites, std::vector<olc::Decal*>& vDecals,
        std::vector<cAnim*> vAnim){
    try{
        std::ifstream file(sFile);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }

        nlohmann::json data;
        file >> data;
        std::vector<sjJob> jjobs;
        for(const auto& item: data.at("jobs")){
            jjobs.emplace_back(item);
        }

        for(const auto& item: jjobs){
            vSprites.emplace_back(new olc::Sprite(item.sprite_path));
            vDecals.emplace_back(new olc::Decal(vSprites.back()));
            std::vector<sAnimDesc> animDesc;
            for(const auto& anim: item.anim_desc){ // anim_desc rows for faction 0
                animDesc.emplace_back(anim);
            }
            for(const auto& anim: item.anim_desc){ // anim_desc rows for faction 1
                animDesc.emplace_back(anim);
            }
            
            vAnim.emplace_back(new cAnim(vDecals.back(), olc::vi2d(item.grid_size_x,item.grid_size_y*2), // grid_size_y*2 to add faction 1 rows
                olc::vf2d(item.cell_size_x,item.cell_size_y), olc::vf2d(item.offset_x,item.offset_y),
                animDesc));
            
            vJobs.emplace_back(new cJob(item.name, item.movement,item.mobility_type,item.hp,item.attack_type,item.str,item.dex,item.int_,item.spr,vAnim.back()));
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON error: " << e.what() << "\n";
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return -1;
    }
    return vJobs.size();
}

struct sjUnit {
    std::string name;
    std::string job;
    int level;
    int pos_x;
    int pos_y;
    int faction;

    explicit sjUnit(const nlohmann::json& data){
        name = data.at("name").get<std::string>();
        job = data.at("job").get<std::string>();
        level = data.at("level").get<int>();
        pos_x = data.at("pos_x").get<int>();
        pos_y = data.at("pos_y").get<int>();
        faction = data.at("faction").get<int>();
    }
};

int ParseUnitsJSON(const std::string& sFile, std::vector<cUnit*>& vUnits, std::vector<cJob*>& vJobs){
    
    try{
        std::ifstream file(sFile);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
        nlohmann::json data;
        file >> data;
        std::vector<sjUnit> junits;
        for(const auto& item: data.at("units")){
            junits.emplace_back(item);
        }
        
        for (const auto& unit: junits){
            vUnits.emplace_back(new cUnit(unit.name, *(vJobs[mJobs.at(unit.job)]),unit.level,unit.faction,{unit.pos_x,unit.pos_y}));
        }

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON error: " << e.what() << "\n";
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return -1;
    }    
    return vUnits.size();
}

#endif