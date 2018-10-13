#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "windows.h"

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
    // Le logger
	GameManager::SetLog(logpath, "MyLog.log");
    // On cr�e notre mod�le du jeu en cours !
    gm = GameManager(_levelInfo);
    gm.InitializeBehaviorTree();


    // On associe � chaque npc son objectif !
    //gm.associateNpcsWithObjectiv();
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
    // On compl�te notre mod�le avec l'information qu'on vient de d�couvrir !
    gm.updateModel(_turnInfo);

    // On d�finit notre strat�gie en ex�cutant notre arbre de comportement
    gm.execute();

    // On fait se d�placer chaque Npc vers son objectif associ� =)
    GameManager::Log("TURN =========================== " + to_string(_turnInfo.turnNb));
    gm.moveNpcs(_actionList);
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}