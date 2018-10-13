#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "MyBotLogic/BehaviorTree/BT_Noeud.h"

class Exploration : public BT_Noeud {
public:
	Exploration() = default;
	~Exploration() = default;
	virtual ETAT_ELEMENT execute() override;
};

#endif
