#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "MyBotLogic/Tools/Minuteur.h"
#include "MyBotLogic/Tools/Profiler.h"

#include "Windows.h"
#include <sstream>

using std::stringstream;
using std::endl;

MyBotLogic::MyBotLogic() :
   logpath{ "" }
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
   GameManager::setLog(logpath, "MyLog.log");
   GameManager::setLogRelease(logpath, "MyLogRelease.log");
   // On cr�e notre mod�le du jeu en cours !
   manager = GameManager(_levelInfo);
   manager.InitializeBehaviorTree();
   auto post = Minuteur::now();

   // On associe � chaque npc son objectif !
   //gm.associateNpcsWithObjectiv();
   stringstream ss;
   ss << "Dur�e Initialisation = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";
   GameManager::log(ss.str());

}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
   //Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, vector<Action*>& _actionList)
{
   Profiler profiler{ GameManager::getLogger(), "FillActionList" };
   profiler << "TURN =========================== " << _turnInfo.turnNb << endl;
   Profiler profilerRelease{ GameManager::getLoggerRelease(), "FillActionList" };
   profilerRelease << "TURN =========================== " << _turnInfo.turnNb << endl;

   // On compl�te notre mod�le avec l'information qu'on vient de d�couvrir !
   manager.updateModel(_turnInfo);

   // On d�finit notre strat�gie en ex�cutant notre arbre de comportement
   manager.execute();

   // On fait se d�placer chaque Npc vers son objectif associ� =)
   manager.moveNpcs(_actionList);
}

/*virtual*/ void MyBotLogic::Exit()
{
   //Write Code Here
}