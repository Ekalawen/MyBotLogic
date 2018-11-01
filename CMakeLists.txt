cmake_minimum_required( VERSION 3.0 )
project( AIBot C CXX )

#add_definitions(-DDEBUG_FRAMEWORK)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

# Filters
include( Filters.cmake )


SET( AIBOT_BUILDFILES 

	ChangeLog.txt
	CheckInstall.bat
	CleanAll.bat
	CMakeLists.txt
	Filters.cmake
	GenAIBootCampViewerFiles.bat
	GenAIBootCampViewerFiles.py
	INSTALL.txt
	PlayLocalMatch.bat
	rebuild_vis_sln_x64.bat
	SubmitAIBot.bat
	SubmitAIBot.py
	TeamInfo.team
)

SET( AIBOT_HEADERS 

	#Framework
	Bot.h	
	BotLogicIF.h
	Globals.h
	LevelInfo.h
	Logger.h
	NPCInfo.h
	ObjectInfo.h
	TileInfo.h
	TurnInfo.h

	#MY_BOT
	MyBotLogic/MyBotLogic.h
	MyBotLogic/GameManager.h
	MyBotLogic/Map.h
	MyBotLogic/MapTile.h
	MyBotLogic/Npc.h
	MyBotLogic/Chemin.h
	MyBotLogic/Mouvement.h
	MyBotLogic/Voisin.h
	MyBotLogic/BehaviorTree/BT_Noeud.h
	MyBotLogic/BehaviorTree/BT_Composite.h
	MyBotLogic/BehaviorTree/Composite/Selecteur.h
	MyBotLogic/BehaviorTree/Composite/Sequenceur.h
	MyBotLogic/BehaviorTree/BT_Feuille.h
	MyBotLogic/BT_Tests/ObjectifsForAllNpcs.h
	MyBotLogic/BT_Tests/CheminsForAllNpcs.h
	MyBotLogic/Strategies/Expedition.h
	MyBotLogic/Strategies/Exploitation.h
	MyBotLogic/Strategies/Exploration.h
	MyBotLogic/Strategies/ScoreStrategie.h
)

SET( AIBOT_SRC 

	#Framework
	Bot.cpp
	LevelInfo.cpp
	Logger.cpp
	TurnInfo.cpp

	#MY_BOT
	MyBotLogic/MyBotLogic.cpp
	MyBotLogic/GameManager.cpp
	MyBotLogic/Map.cpp
	MyBotLogic/MapTile.cpp
	MyBotLogic/Npc.cpp
	MyBotLogic/Chemin.cpp
	MyBotLogic/Mouvement.cpp
	MyBotLogic/Voisin.cpp
	MyBotLogic/BT_Tests/ObjectifsForAllNpcs.cpp
	MyBotLogic/BT_Tests/CheminsForAllNpcs.cpp
	MyBotLogic/Strategies/Expedition.cpp
	MyBotLogic/Strategies/Exploitation.cpp
	MyBotLogic/Strategies/Exploration.cpp
	MyBotLogic/Strategies/ScoreStrategie.cpp
	
	#Main
	Main.cpp
)

SET(jobfiles "${AIBOT_HEADERS};${AIBOT_SRC};${AIBOT_BUILDFILES}")
file(WRITE jobfiles.txt "${jobfiles}")

add_executable(
	aibot

	${AIBOT_BUILDFILES}
	${AIBOT_HEADERS}
	${AIBOT_SRC}
)

CREATE_VISUAL_STUDIO_FILTERS(AIBOT)

