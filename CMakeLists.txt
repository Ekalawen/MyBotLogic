cmake_minimum_required( VERSION 3.0 )
project( AIBot C CXX )

#add_definitions(-DDEBUG_FRAMEWORK)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(GLOBAL PROPERTY CMAKE_SKIP_INSTALL_ALL_DEPENDENCY  ON)
set_property(GLOBAL PROPERTY CMAKE_SKIP_PACKAGE_ALL_DEPENDENCY  ON)
set_property(GLOBAL PROPERTY CMAKE_SUPPRESS_REGENERATION  ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#Compile Option
SET(MY_COMPILE_OPTIONS /std:c++latest)

set(MY_RELEASE_OPTIONS ${MY_COMPILE_OPTIONS} /MT)
set(MY_DEBUG_OPTIONS ${MY_COMPILE_OPTIONS} -D_DEBUG /MTd)

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
	MyBotLogic/Carte.h
	MyBotLogic/MapTile.h
	MyBotLogic/Npc.h
	MyBotLogic/Chemin.h
	MyBotLogic/Mouvement.h
	MyBotLogic/Voisin.h
	MyBotLogic/Noeud.h
	MyBotLogic/Porte.h
	MyBotLogic/Activateur.h
	MyBotLogic/Contrainte.h
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
	MyBotLogic/Tools/Minuteur.h
	MyBotLogic/Tools/Profiler.h
	MyBotLogic/Tools/ThreadPool.h
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
	MyBotLogic/Carte.cpp
	MyBotLogic/MapTile.cpp
	MyBotLogic/Npc.cpp
	MyBotLogic/Chemin.cpp
	MyBotLogic/Mouvement.cpp
	MyBotLogic/Voisin.cpp
	MyBotLogic/Noeud.cpp
	MyBotLogic/Porte.cpp
	MyBotLogic/Activateur.cpp
	MyBotLogic/Contrainte.cpp
	MyBotLogic/BT_Tests/ObjectifsForAllNpcs.cpp
	MyBotLogic/BT_Tests/CheminsForAllNpcs.cpp
	MyBotLogic/Strategies/Expedition.cpp
	MyBotLogic/Strategies/Exploitation.cpp
	MyBotLogic/Strategies/Exploration.cpp
	MyBotLogic/Strategies/ScoreStrategie.cpp
	MyBotLogic/Tools/Minuteur.cpp
	MyBotLogic/Tools/Profiler.cpp
	MyBotLogic/Tools/ThreadPool.cpp
	
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

target_compile_options(aibot PUBLIC "$<$<CONFIG:RELEASE>:${MY_RELEASE_OPTIONS}>")
target_compile_options(aibot PUBLIC "$<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>")

CREATE_VISUAL_STUDIO_FILTERS(AIBOT)

