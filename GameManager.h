#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Carte.h"
#include "Npc.h"
#include "Logger.h"
#include "Mouvement.h"
#include "TurnInfo.h"

#include "BehaviorTree/Composite/Selecteur.h"

#include <map>

class npc_inexistant {};
class npc_deja_existant {};

class GameManager {
    static Logger logger, loggerRelease;
    std::map<int, Npc> npcs; // Les npcs sont stock�s par leurs ids
public:
    Carte carte;
    Selecteur behaviorTreeManager; // Arbre de comportement du GameManager pour d�terminer la strat�gie � suivre
    std::vector<int> objectifPris; // Permet de savoir quels sont les objectifs actuellement assign�s � des npcs

    GameManager() = default;
    GameManager(LevelInfo);
    void moveNpcs(std::vector<Action*>& actionList) noexcept; // Remplie l'action liste !
    void reaffecterObjectifsSelonDistance(); // R�affecte les objectifs des Npcs entre
    void ordonnerMouvements(std::vector<Mouvement>& mouvements) noexcept; // Permet d'ordonner les mouvements pour �viter les collisions et g�rer les politesses de priorit�s =)
    void updateModel(const TurnInfo&) noexcept; // Met � jour le mod�le avec les informations que d�couvrent les NPCS
    void InitializeBehaviorTree() noexcept; // Permet d'initialiser le BT
    void execute() noexcept { behaviorTreeManager.execute(); };

    Npc& getNpcById(int id);
    std::map<int, Npc>& getNpcs();
    void addNpc(Npc npc);

    static void log(std::string str) noexcept { // Permet de d�bugger ! :D
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Log(str);
        #endif
    }
    static void logRelease(std::string _str) noexcept { // Permet de d�bugger ! :D
        loggerRelease.Log(_str);
    }
    static void SetLog(std::string path, std::string fileName) noexcept { // Permet d'initialiser le logger =)
        #ifndef _DEBUG
            return;
        #endif
        #ifdef _DEBUG
            logger.Init(path, fileName);
        #endif
    }
    static void SetLogRelease(std::string path, std::string fileName) noexcept { // Permet d'initialiser le logger =)
        loggerRelease.Init(path, fileName);
    }

private:
    void addNewTiles(const TurnInfo& ti) noexcept;
    void addNewObjects(const TurnInfo& ti) noexcept;
    std::vector<Mouvement> getAllMouvements();
    int getIndiceMouvementPrioritaire(std::vector<Mouvement>& mouvements, const std::vector<int>& indicesAConsiderer);
    void gererCollisionsMemeCaseCible(std::vector<Mouvement>& mouvements);
    void stopNonPrioritaireMouvements(std::vector<Mouvement>& mouvements, const std::vector<int>& indicesMouvementsSurMemeCaseCible, const int indiceMouvementPrioritaire, bool& continuer);
};

#endif