
#include "Exploitation.h"
#include "Exploration.h"
#include "MyBotLogic/BehaviorTree/BT_Noeud.h"

BT_Noeud::ETAT_ELEMENT Exploration::execute() {
    GameManager::Log("Appel de Exploration.execute()");
	/// PSEUDO-CODE
	//prend les cases visibles par le npc

	//dans cette stratégie on doit choisir la case qui a le meilleurs score
	// le score se calcule selon la pondération du nombre d'information obtenable (nombre de voisins Visible et nombre voisins Accessibles), et de la distance au NPC

	// score (NPC,ID_TILE);


	return ETAT_ELEMENT();
}
