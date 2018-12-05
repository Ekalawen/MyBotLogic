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
using std::to_string;

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
   // On crée notre modèle du jeu en cours !

   auto pre = Minuteur::now();
   // Le logger
   GameManager::setLog(logpath, "MyLog.log");
   GameManager::setLogRelease(logpath, "MyLogRelease.log");
   manager.Init(_levelInfo);
   manager.InitializeBehaviorTree();
   auto post = Minuteur::now();
   // On associe à chaque npc son objectif !
   stringstream ss;
   ss << "Durée Initialisation = " << Minuteur::dureeMicroseconds(pre, post) / 1000.f << "ms";
   LOG(ss.str());

   tempsAvantServeur = Minuteur::now();
}

/*virtual*/ void MyBotLogic::OnGameStarted()
{
   //Write Code Here
}

/*virtual*/ void MyBotLogic::FillActionList(TurnInfo& _turnInfo, vector<Action*>& _actionList)
{
   ProfilerDebug profiler{ GameManager::getLogger(), "Tour" };
   ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "Tour" };
   profiler << "TURN =========================== " << _turnInfo.turnNb << endl;
   profilerRelease << "TURN =========================== " << _turnInfo.turnNb << endl;

   tempsApresServeur = Minuteur::now();

   profilerRelease << "Duree serveur : " << duration_cast<milliseconds>(tempsApresServeur - tempsAvantServeur).count() << " ms" << endl;

   if (!manager.isFloodFillDejaCalcule) {
      // On complete notre modele avec l'information qu'on vient de decouvrir !
      manager.updateModel(_turnInfo);
      profilerRelease << "ON A DU isFloodFillDejaCalcule " << endl;
   }

   if (manager.floodFillFinished()) {
      profilerRelease << "ON A DU TEMPS " << endl;
      manager.isFloodFillDejaCalcule = false;

      // On definit notre strategie en executant notre arbre de comportement
      //manager.execute();

      // On fait se deplacer chaque Npc vers son objectif associe =)
      manager.moveNpcs(_actionList);

   }
   else {
      profilerRelease << "ON A PAS DU TEMPS " << endl;
      manager.isFloodFillDejaCalcule = true;
   }

   tempsAvantServeur = Minuteur::now();
}

/*virtual*/ void MyBotLogic::Exit()
{
   //Write Code Here
}