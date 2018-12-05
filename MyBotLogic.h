#ifndef MY_BOT_LOGIC_H
#define MY_BOT_LOGIC_H

#include "BotLogicIF.h"
#include "Logger.h"
#include "MyBotLogic/GameManager.h"
#include "MyBotLogic/Tools/Minuteur.h"

#include <thread>
#include <condition_variable>
#include <future>

#ifdef _DEBUG
   #define BOT_LOGIC_DEBUG
#endif

#ifdef BOT_LOGIC_DEBUG
   #define BOT_LOGIC_LOG(logger, text, autoEndLine) logger.Log(text, autoEndLine)
#else
   #define BOT_LOGIC_LOG(logger, text, autoEndLine) 0
#endif

class Minuteur;
//Custom BotLogic where the AIBot decision making algorithms should be implemented.
//This class must be instantiated in main.cpp.
class MyBotLogic : public virtual BotLogicIF
{
public:
    GameManager manager;
    std::string logpath;
    time_point<steady_clock> tempsAvantServeur;
    time_point<steady_clock> tempsApresServeur;
    std::future<void> workerSleep; // Le précursseur !
    std::vector<std::future<void>> workersFloodFill;
    std::future<void> workerExecute;

	MyBotLogic();
	virtual ~MyBotLogic();

	virtual void Configure(int argc, char *argv[], const std::string& _logpath);
	virtual void Start();
	virtual void Init(LevelInfo& _levelInfo);
	virtual void OnGameStarted();
	virtual void FillActionList(TurnInfo& _turnInfo, std::vector<Action*>& _actionList);   //calculate moves for a single turn
	virtual void Exit();

protected:
	Logger mLogger;
};

#endif
