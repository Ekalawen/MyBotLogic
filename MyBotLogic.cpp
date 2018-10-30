#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "Windows.h"
#include <chrono>
using namespace std::chrono;


MyBotLogic::MyBotLogic() :
    logpath{""}
{
	//Write Code Here
}

/*virtual*/ MyBotLogic::~MyBotLogic()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Configure(int argc, char *argv[], const std::string& _logpath)
{
#ifdef BOT_LOGIC_DEBUG
	mLogger.Init(_logpath, "MyBotLogic.log");
#endif

	BOT_LOGIC_LOG(mLogger, "Configure", true);
    /* _logpath =
    C:\Users\dusa2404\Documents\IA\IABootCamp\AIBot_v0.59\\LocalMatchResults\aibotlog
    */
    logpath = _logpath;
	
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Start()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Init(LevelInfo& _levelInfo)
{
    auto pre = high_resolution_clock::now();
    // Le logger
	GameManager::setLog(logpath, "MyLog.log");
	GameManager::setLogRelease(logpath, "MyLogRelease.log");
    // On cr�e notre mod�le du jeu en cours !
    gm = GameManager(_levelInfo);
    gm.InitializeBehaviorTree();

    // On associe � chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
    auto post = high_resolution_clock::now();
    GameManager::log("Dur�e Initialisation = " + std::to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    auto preFAL = high_resolution_clock::now();
    GameManager::log("TURN =========================== " + std::to_string(_turnInfo.turnNb));

    // On compl�te notre mod�le avec l'information qu'on vient de d�couvrir !
    auto pre = high_resolution_clock::now();
    gm.updateModel(_turnInfo);
    auto post = high_resolution_clock::now();
    GameManager::log("Dur�e Update = " + std::to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    // On d�finit notre strat�gie en ex�cutant notre arbre de comportement
    pre = high_resolution_clock::now();
    gm.execute();
    post = high_resolution_clock::now();
    GameManager::log("Dur�e Execute = " + std::to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    // On fait se d�placer chaque Npc vers son objectif associ� =)
    pre = high_resolution_clock::now();
    gm.moveNpcs(_actionList);
    post = high_resolution_clock::now();
    GameManager::log("Dur�e Move = " + std::to_string(duration_cast<microseconds>(post - pre).count() / 1000.f) + "ms");

    auto postFAL = high_resolution_clock::now();
    GameManager::log("Dur�e Tour = " + std::to_string(duration_cast<microseconds>(postFAL - preFAL).count() / 1000.f) + "ms");
    GameManager::LogRelease("Dur�e Tour num�ro " + std::to_string(_turnInfo.turnNb) + " = " + std::to_string(duration_cast<microseconds>(postFAL - preFAL).count() / 1000.f) + "ms");
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}