#ifndef ACTIVATEUR_H
#define ACTIVATEUR_H

class Activateur {
private:
    int id;
    int tileId;
public:
    Activateur() = default;
    Activateur(const int id, const int tileId);

    int getId() const noexcept;
    int getTileId() const noexcept;
};

#endif



