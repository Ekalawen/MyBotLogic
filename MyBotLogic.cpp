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
    gm = GameManager(_levelInfo);
    gm.InitializeBehaviorTree();

    // On associe � chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
    stringstream ss;
    ss << "Dur�e Initialisation = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms";
    GameManager::Log(ss.str());
    auto post = Minuteur::now();
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    stringstream ss;
    ss << "TURN =========================== " << _turnInfo.turnNb << std::endl;
    auto preFAL = Minuteur::now();

    // On compl�te notre mod�le avec l'information qu'on vient de d�couvrir !
    auto pre = Minuteur::now();
    gm.updateModel(_turnInfo);
    ss << "Dur�e Update = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms" << std::endl;
    auto post = Minuteur::now();

    // On d�finit notre strat�gie en ex�cutant notre arbre de comportement
    pre = Minuteur::now();
    gm.execute();
    ss << "Dur�e Execute = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms" << std::endl;
    post = Minuteur::now();

    // On fait se d�placer chaque Npc vers son objectif associ� =)
    pre = Minuteur::now();
    gm.moveNpcs(_actionList);
    post = Minuteur::now();
    ss << "Dur�e Move = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms" << std::endl;

    ss << "Dur�e Tour = " << duration_cast<microseconds>(post - pre).count() / 1000.f << "ms" << std::endl;
    GameManager::Log(ss.str());
    ss.clear();

    ss << "Dur�e Tour num�ro " << _turnInfo.turnNb << " = " << duration_cast<microseconds>(postFAL - preFAL).count() / 1000.f << "ms";
    GameManager::LogRelease(ss.str());
    auto postFAL = Minuteur::now();
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}