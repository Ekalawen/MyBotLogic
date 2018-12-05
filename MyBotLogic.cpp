#include "MyBotLogic.h"

#include "TurnInfo.h"
#include "NPCInfo.h"
#include "LevelInfo.h"

#include "MyBotLogic/Tools/Minuteur.h"
#include "MyBotLogic/Tools/Profiler.h"

#include "Windows.h"
#include <sstream>
#include <thread>
#include <future>
#include <functional>
#include <fstream>

using std::stringstream;
using std::endl;
using std::to_string;

MyBotLogic::MyBotLogic() :
   logpath{ "" }
{
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
   ProfilerDebug profiler{ GameManager::getLogger(), "Tour", GameManager::getLoggerJSON(), GameManager::getTempsDebutProgramme() };
   ProfilerRelease profilerRelease{ GameManager::getLoggerRelease(), "Tour", GameManager::getLoggerJSON(), GameManager::getTempsDebutProgramme(), true, false, manager.TEMPS_ACCORDE_TOUR };
   profiler << "TURN =========================== " << _turnInfo.turnNb << endl;
   profilerRelease << "TURN =========================== " << _turnInfo.turnNb << endl;

   tempsApresServeur = Minuteur::now();

   profilerRelease << "Duree serveur : " << duration_cast<milliseconds>(tempsApresServeur - tempsAvantServeur).count() << " ms" << endl;

   if (manager.etatFloodFill == GameManager::FonctionEtat::PAS_COMMENCE
      && manager.etatExecute == GameManager::FonctionEtat::PAS_COMMENCE) {
      // On complete notre modele avec l'information qu'on vient de decouvrir !
      manager.updateModel(_turnInfo, *this);
   }

   manager.etatFloodFill = (manager.floodFillFinished(*this)) ? GameManager::FonctionEtat::CALCULE_PAS_UTILISE : GameManager::FonctionEtat::EN_COURS;

   if (manager.etatFloodFill == GameManager::FonctionEtat::CALCULE_PAS_UTILISE
      && manager.etatExecute == GameManager::FonctionEtat::PAS_COMMENCE) {

      // On definit notre strategie en executant notre arbre de comportement
      manager.execute(*this);
   }
      
   if (manager.etatFloodFill == GameManager::FonctionEtat::CALCULE_PAS_UTILISE) {
       manager.etatExecute = (manager.executeFinished(*this)) ? GameManager::FonctionEtat::CALCULE_PAS_UTILISE : GameManager::FonctionEtat::EN_COURS;
   }

   if (manager.etatFloodFill == GameManager::FonctionEtat::CALCULE_PAS_UTILISE
      && manager.etatExecute == GameManager::FonctionEtat::CALCULE_PAS_UTILISE) {
         // On fait se deplacer chaque Npc vers son objectif associe =)
         manager.moveNpcs(_actionList);
         manager.etatExecute = GameManager::FonctionEtat::PAS_COMMENCE;
         manager.etatFloodFill = GameManager::FonctionEtat::PAS_COMMENCE;
   }

   if (manager.etatFloodFill == GameManager::FonctionEtat::EN_COURS
      || manager.etatExecute == GameManager::FonctionEtat::EN_COURS) {
       microseconds tempsDepuisDebutTour = duration_cast<microseconds>(Minuteur::now() - tempsApresServeur);
       microseconds dureeSleep = manager.TEMPS_ACCORDE_TOUR;
       if (tempsDepuisDebutTour < dureeSleep) {
           auto tempsAvantSleep = Minuteur::now();
           //profilerRelease << "On voudrait dormir pendant " << (dureeSleep - tempsDepuisDebutTour).count() << " us" << endl;
           while (duration_cast<microseconds>(Minuteur::now() - tempsApresServeur) < dureeSleep);
           //profilerRelease << "On a dormis pendant  " << duration_cast<microseconds>(Minuteur::now() - tempsAvantSleep).count() << " us" << endl;
       }
   }

   tempsAvantServeur = Minuteur::now();
}

#ifndef BOT_LOGIC_DEBUG
//#define VERSION_LIVRAISION 
#endif // !BOT_LOGIC_DEBUG


/*virtual*/ void MyBotLogic::Exit()
{
#ifndef VERSION_LIVRAISION
   {
      std::ofstream jsonFile;
      jsonFile.open(logpath + "\\" + "jsonFile.txt");
      jsonFile << "[" << std::endl;
      for (int i = 0; i < GameManager::getLoggerJSON().size(); i++) {
         jsonFile << GameManager::getLoggerJSON()[i];
         string virgule = (i < GameManager::getLoggerJSON().size() - 1) ? ",": " ";
         jsonFile << virgule << std::endl;
      }
      jsonFile << "]";
      jsonFile.close();
   }
#endif // !VERSION_LIVRAISION
}