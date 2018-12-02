#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "MyBotLogic/Tools/Minuteur.h"

#include "Windows.h"
#include "MyBotLogic/Tools/Minuteur.h"
#include <sstream>

using std::stringstream;
using std::endl;
using std::to_string;

MyBotLogic::MyBotLogic() :
    logpath{""}
{
	//Write Code Here
}

/*virtual*/ MyBotLogic::~MyBotLogic()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::Configure(int argc, char *argv[], const string& _logpath)
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
	GameManager::SetLog(logpath, "MyLog.log");
	GameManager::SetLogRelease(logpath, "MyLogRelease.log");

    // On crée notre modèle du jeu en cours !
    manager = GameManager(_levelInfo);
    manager.InitializeBehaviorTree();
    auto post = Minuteur::now();

    // On associe à chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
    stringstream ss;
    ss << "Durée Initialisation = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";
    GameManager::log(ss.str());
   
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, vector<Action*>& _actionList)
{
    auto preFAL = Minuteur::now();
    stringstream ss;
    GameManager::log("TURN =========================== " + to_string(_turnInfo.turnNb));

    // On complète notre modèle avec l'information qu'on vient de découvrir !
    auto pre = Minuteur::now();
    manager.updateModel(_turnInfo);
    auto post = Minuteur::now();
    ss << "Durée Update = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;

    // On définit notre stratégie en exécutant notre arbre de comportement
    pre = Minuteur::now();
    manager.execute();
    manager.reafecterObjectifsSelonDistance();
    manager.affecterContraintes();
    post = Minuteur::now();
    ss << "Durée Execute = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;
    

    // On fait se déplacer chaque Npc vers son objectif associé =)
    pre = Minuteur::now();
    manager.moveNpcs(_actionList);
    post = Minuteur::now();
    ss << "Durée Move = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << endl;


    auto postFAL = Minuteur::now();
    ss << "Durée Tour = " << Minuteur::dureeMicroseconds(preFAL, postFAL) / 1000.f << "ms" << endl;
    GameManager::log(ss.str());
    ss << "Durée Tour numéro " << _turnInfo.turnNb << " = " << Minuteur::dureeMicroseconds(preFAL, postFAL) / 1000.f << "ms";
    GameManager::logRelease(ss.str());
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}