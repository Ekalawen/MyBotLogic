#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "Windows.h"
#include "MyBotLogic/Tools/Minuteur.h"

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
    auto pre = Minuteur::now();
    // Le logger
	GameManager::setLog(logpath, "MyLog.log");
	GameManager::setLogRelease(logpath, "MyLogRelease.log");
    // On crée notre modèle du jeu en cours !
    gm = GameManager(_levelInfo);
    gm.InitializeBehaviorTree();

    // On associe à chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
    auto post = Minuteur::now();
    GameManager::log("Durée Initialisation = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    auto preFAL = Minuteur::now();
    GameManager::log("TURN =========================== " + std::to_string(_turnInfo.turnNb));

    // On complète notre modèle avec l'information qu'on vient de découvrir !
    auto pre = Minuteur::now();
    gm.updateModel(_turnInfo);
    auto post = Minuteur::now();
    GameManager::log("Durée Update = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");

    // On définit notre stratégie en exécutant notre arbre de comportement
    pre = Minuteur::now();
    gm.execute();
    post = Minuteur::now();
    GameManager::log("Durée Execute = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");

    // On fait se déplacer chaque Npc vers son objectif associé =)
    pre = Minuteur::now();
    gm.moveNpcs(_actionList);
    post = Minuteur::now();
    GameManager::log("Durée Move = " + std::to_string(Minuteur::dureeMicroseconds(pre, post) / 1000.f) + "ms");

    auto postFAL = Minuteur::now();
    GameManager::log("Durée Tour = " + std::to_string(Minuteur::dureeMicroseconds(preFAL, postFAL) / 1000.f) + "ms");
    GameManager::LogRelease("Durée Tour numéro " + std::to_string(_turnInfo.turnNb) + " = " + std::to_string(Minuteur::dureeMicroseconds(preFAL, postFAL) / 1000.f) + "ms");
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}