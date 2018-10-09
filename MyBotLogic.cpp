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
    // On crée notre modèle du jeu en cours !
    gm = GameManager(_levelInfo);
    // Le logger
	GameManager::SetLog(logpath, "MyLog.log");

    // On associe à chaque npc son objectif !
    gm.associateNpcsWithObjectiv();
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
	//Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList)
{
	//Write Code Here.
    //_actionList.push_back(new Move(_turnInfo.npcs[0].npcID, Tile::E));

    // On fait se déplacer chaque Npc vers son objectif associé =)
    GameManager::Log("TURN =========================== " + to_string(_turnInfo.turnNb));
    gm.moveNpcs(_actionList);
}

/*virtual*/ void MyBotLogic::Exit()
{
	//Write Code Here
}