#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "Windows.h"
#include "MyBotLogic/Tools/Minuteur.h"
#include <sstream>

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
    // On cr�e notre mod�le du jeu en cours !
    manager = GameManager(_levelInfo);
    manager.InitializeBehaviorTree();
    auto post = Minuteur::now();

    // On associe � chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
    std::stringstream ss;
    ss << "Dur�e Initialisation = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";
    GameManager::log(ss.str());
   
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    std::stringstream ss;
    ss << "TURN =========================== " << _turnInfo.turnNb << std::endl;
    auto preFAL = Minuteur::now();

    // On compl�te notre mod�le avec l'information qu'on vient de d�couvrir !
    auto pre = Minuteur::now();
    manager.updateModel(_turnInfo);
    auto post = Minuteur::now();
    ss << "Dur�e Update = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << std::endl;

    // On d�finit notre strat�gie en ex�cutant notre arbre de comportement
    pre = Minuteur::now();
    manager.execute();
    post = Minuteur::now();
    ss << "Dur�e Execute = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << std::endl;
    

    // On fait se d�placer chaque Npc vers son objectif associ� =)
    pre = Minuteur::now();
    manager.moveNpcs(_actionList);
    post = Minuteur::now();
    ss << "Dur�e Move = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << std::endl;

    ss << "Dur�e Tour = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms" << std::endl;
    GameManager::log(ss.str());
    ss.clear();

    auto postFAL = Minuteur::now();

    ss << "Dur�e Tour num�ro " << _turnInfo.turnNb << " = " << Minuteur::dureeMicroseconds(preFAL, postFAL) / 1000.f << "ms";
    GameManager::logRelease(ss.str());
    
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}