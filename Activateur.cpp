
#include "Activateur.h"

Activateur::Activateur(const int id, const int tileId)
    : id{ id }, tileId{ tileId }
{
}

int Activateur::getId() const noexcept {
    return id;
}
int Activateur::getTileId() const noexcept {
    return tileId;
}

