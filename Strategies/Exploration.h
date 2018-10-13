#ifndef EXPLORATION_H
#define EXPLORATION_H

#include "MyBotLogic/BehaviorTree/BT_Feuille.h"

class Exploration : public BT_Feuille {
public:
	Exploration() = default;
	~Exploration() = default;
	virtual ETAT_ELEMENT execute() override;
};

#endif
