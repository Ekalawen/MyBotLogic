
#include "Carte.h"
#include "GameManager.h"

#include <algorithm>
#include <map>
#include <sstream>

Carte::Carte(const LevelInfo& _levelInfo) :
   rowCount{ _levelInfo.rowCount },
   colCount{ _levelInfo.colCount },
   nbTilesDecouvertes{ 0 }
{
   // Cr�er toutes les tiles !
   tiles.reserve(getNbTiles());
   // Cr�er toutes les tiles !
   tiles.reserve(getNbTiles());
   int offset = 1;
   for (auto i = 0; i < getRowCount(); ++i)
   {
      offset -= !(i & 1);
      unsigned int indexI = i * getColCount();

      for (auto j = 0; j < getColCount(); ++j)
      {
         tiles.emplace_back(
            indexI + j,
            MapPosition{ offset + j,  i }
         );
      }
   }

   // Connecte toutes les tuiles!
   for (auto& tuile : tiles)
   {
      // On test chaque voisin possible
      for (auto direction = 0; direction < MapTile::NB_VOISINS_TOTAL; ++direction)
      {
         auto voisinPositionOffset = tuile.getPosition().CalculerPositionOffset(static_cast<Tile::ETilePosition>(direction));

         // Si la position est d�finit dans la carte...
         if (estDefiniDansCarte(voisinPositionOffset))
         {
            //On le rajoute comme voisin
            tuile.getVoisins().emplace_back(ConvertirPositionOffsetAID(voisinPositionOffset), static_cast<Tile::ETilePosition>(direction));
         }
      }
   }

   // Mettre � jour les tiles connues
   for (auto tile : _levelInfo.tiles) {
      addTile(tile.second);
   }

   // Enregistrer les objets
   for (auto object : _levelInfo.objects) {
      addObject(object.second);
   }

   // Mettre � visiter les cases initiales des NPCs
   for (auto pair_npc : _levelInfo.npcs) {
      tiles[pair_npc.second.tileID].setStatut(MapTile::Statut::VISITE);
   }
}

bool Carte::isInMap(const int idTile) const noexcept {
   return idTile > -1 && idTile < rowCount * colCount;
}

bool Carte::estDefiniDansCarte(const MapPosition& _position) const noexcept {
   return !(_position.x < 0 || _position.x > getColCount() - 1 || _position.z < 0 || _position.z > getRowCount() - 1);
}

int Carte::ConvertirPositionOffsetAID(const MapPosition& _position) const noexcept {
   return _position.z * getColCount() + _position.x;
}


struct Noeud {
   static float coefEvaluation;
   MapTile tile;
   float cout; // La distance calcul� depuis le d�part
   float evaluation; // La distance estim�e � l'arriv�e
   float heuristique; // La somme du cout et de l'evaluation
   int idPrecedant;
   Noeud() = default;
   Noeud(const MapTile& tile, const float cout, const float evaluation, const int idPrecedant)
      : tile{ tile }, cout{ cout }, evaluation{ evaluation }, idPrecedant{ idPrecedant } {
      heuristique = cout + evaluation * coefEvaluation;
   }
   friend bool operator==(const Noeud& g, const Noeud& d) {
      return g.tile.getId() == d.tile.getId();
   }
};
float Noeud::coefEvaluation = 1;

// Il s'agit de l'algorithme AStar auquel on peut rajouter un coefficiant � l'�valuation pour modifier l'heuristique.
// Par d�faut sa valeur est 1. Si on l'augmente l'algorithme ira plus vite au d�triment de trouver un chemin optimal.
// Si on le diminue l'algorithme se rapproche de plus en plus d'un parcours en largeur.
Chemin Carte::aStar(const int depart, const int arrivee, const float coefEvaluation) const noexcept {
   Noeud::coefEvaluation = coefEvaluation;
   // On cr�e nos liste et notre noeud courrant
   std::vector<Noeud> closedList{};
   std::vector<Noeud> openList{};
   Noeud noeudCourant = Noeud(tiles[depart], 0, distanceReel(depart, arrivee), depart);
   Chemin path;

   // On ajoute le noeud initial
   openList.push_back(noeudCourant);
   // Tant qu'il reste des noeuds � traiter ...
   while (!openList.empty() && noeudCourant.tile.getId() != arrivee) {
      // On r�cup�re le premier noeud de notre liste
      noeudCourant = openList.back();
      openList.pop_back();
      // Pour tous les voisins du noeud courant ...
      for (auto voisinID : noeudCourant.tile.getVoisinsIDParEtat(Etats::ACCESSIBLE)) {
         // On v�rifie que le voisin existe ...
         if (tiles[voisinID].existe()) {
            // On construit le nouveau noeud
            Noeud nouveauNoeud = Noeud(tiles[voisinID], noeudCourant.cout + 1, distanceReel(voisinID, arrivee), noeudCourant.tile.getId());
            // On v�rifie s'il existe dans closedList avec un cout inf�rieur ou dans openList avec un cout inf�rieur
            auto itClose = find(closedList.begin(), closedList.end(), nouveauNoeud);
            auto itOpen = find(openList.begin(), openList.end(), nouveauNoeud);

            if (itClose == closedList.end() && itOpen == openList.end()) {
               openList.push_back(nouveauNoeud);
            }
            else if (itClose != closedList.end() && itOpen == openList.end()) {
               // Do nothing
            }
            else if (itClose == closedList.end() && itOpen != openList.end()) {
               if ((*itOpen).heuristique > nouveauNoeud.heuristique) {
                  (*itOpen) = nouveauNoeud;
               }
            }
            else {
               GameManager::log("OMG On a fait n'imp !");
            }
         }
      }
      // On trie notre openList pour que le dernier soit le meilleur !
      // Donc celui qui minimise et le cout, et l'�valuation !
      sort(openList.begin(), openList.end(), [](const Noeud a, const Noeud b) {
         return a.heuristique > b.heuristique; // Par ordre d�croissant
      });

      // On ferme notre noeud
      closedList.push_back(noeudCourant);
   }

   // On test si on a atteint l'objectif ou pas
   if (noeudCourant.tile.getId() == arrivee) {
      // Si oui on reconstruit le path !
      while (noeudCourant.tile.getId() != depart) {
         // On enregistre dans le path ...
         path.addFirst(noeudCourant.tile.getId());
         // On cherche l'ant�c�dant ...
         for (auto n : closedList) {
            if (n.tile.getId() == noeudCourant.idPrecedant) {
               // On remet � jour le noeud ...
               noeudCourant = n;
               break;
            }
         }
      }

   }
   else {
      // Si non le path est inaccessible !
      path.setInaccessible();
   }

   return path;
}

Tile::ETilePosition Carte::getDirection(const int _tuileID, const int _voisinID) const noexcept {
   auto voisins = tiles[_tuileID].getVoisins();
   auto result = std::find_if(begin(voisins), end(voisins), [&_voisinID](const Voisin& v) {
      return v.getTuileIndex() == _voisinID;
   });

   if (result != end(voisins)) {
      return result->getDirection();
   }

   GameManager::log("Erreur dans l'appel de getDirection()!");
   return Tile::CENTER;
}

int Carte::getAdjacentTileAt(const int tileSource, const Tile::ETilePosition direction) const noexcept {
   auto voisins = tiles[tileSource].getVoisins();
   auto result = std::find_if(begin(voisins), end(voisins), [&direction](const Voisin& v) {
      return v.getDirection() == direction;
   });

   if (result != end(voisins)) {
      return result->getTuileIndex();
   }

   GameManager::log("Erreur dans l'appel de getAdjacentTileAt() !");
   return -1;
}

float Carte::distanceReel(const int _depart, const int _fin) const noexcept {
   return tiles[_depart].getPosition().DistancreReelEntre(tiles[_fin].getPosition());
}

int Carte::distanceNbTuiles(const int _depart, const int _fin) const noexcept {
   return tiles[_depart].getPosition().NbTuilesEntre(tiles[_fin].getPosition());
}

int Carte::tailleCheminMax() const noexcept {
   return colCount * rowCount + 1;
}

// Il ne faut pas ajouter une tile qui est d�j� dans la map !
void Carte::addTile(const TileInfo& tile) noexcept {
   // On met � jour le nombre de tiles
   ++nbTilesDecouvertes;

   // On la rajoute aux tiles
   tiles[tile.tileID].setTileDecouverte(tile);

   if (tiles[tile.tileID].getType() == Tile::TileAttribute_Goal) {
      objectifs.push_back(tile.tileID);
   }

   if (tiles[tile.tileID].getType() == Tile::TileAttribute_Forbidden) {
      for (auto voisin : tiles[tile.tileID].getVoisins()) {
         tiles[voisin.getTuileIndex()].removeEtat(Etats::ACCESSIBLE, tile.tileID);
      }
   }

   // Puis on met � jour les voisins de ses voisins ! :D
   for (auto voisinID : tiles[tile.tileID].getVoisinsIDParEtat(Etats::VISIBLE)) {
      // Si ce voisin l'a en voisin myst�rieux, on le lui enl�ve
      tiles[voisinID].removeEtat(Etats::MYSTERIEUX, tile.tileID);
   }

   // On le note !
   std::stringstream ss;
   ss << "Decouverte de la tile " << tile.tileID;
   GameManager::log(ss.str());
}

// Il ne faut pas ajouter un objet qui est d�j� dans la map !
void Carte::addObject(const ObjectInfo& object) noexcept {
   int voisin1 = object.tileID;
   int voisin2 = getAdjacentTileAt(object.tileID, object.position);

   // On ajoute notre objet � l'ensemble de nos objets
   if (object.objectTypes.find(Object::ObjectType_Wall) != object.objectTypes.end()) {
      // Fenetre
      if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
         fenetres[object.objectID] = object;
         if (isInMap(voisin1))
            tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
         if (isInMap(voisin2))
            tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
         // Mur
      }
      else {
         murs[object.objectID] = object;
         if (isInMap(voisin1)) {
            tiles[voisin1].removeEtat(Etats::MYSTERIEUX, voisin2);
            tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
            tiles[voisin1].removeEtat(Etats::VISIBLE, voisin2);
         }
         if (isInMap(voisin2)) {
            tiles[voisin2].removeEtat(Etats::MYSTERIEUX, voisin1);
            tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
            tiles[voisin2].removeEtat(Etats::VISIBLE, voisin1);
         }
      }
   }
   if (object.objectTypes.find(Object::ObjectType_Door) != object.objectTypes.end()) {
      portes[object.objectID] = object;
      //Porte Ferme
      if (object.objectStates.find(Object::ObjectState_Closed) != object.objectStates.end()) {
         // Porte Fenetre
         if (object.objectTypes.find(Object::ObjectType_Window) != object.objectTypes.end()) {
            if (isInMap(voisin1))
               tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
            if (isInMap(voisin2))
               tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
            // Porte
         }
         else {
            if (isInMap(voisin1)) {
               tiles[voisin1].removeEtat(Etats::ACCESSIBLE, voisin2);
               tiles[voisin1].removeEtat(Etats::VISIBLE, voisin2);
            }
            if (isInMap(voisin2)) {
               tiles[voisin2].removeEtat(Etats::ACCESSIBLE, voisin1);
               tiles[voisin2].removeEtat(Etats::VISIBLE, voisin1);
            }
         }
         // Porte ouverte
      }
      else {
         // Si la porte est ouverte on est accessible ET visible ! =)
      }
   }
   if (object.objectTypes.find(Object::ObjectType_PressurePlate) != object.objectTypes.end()) {
      activateurs[object.objectID] = object;
      // prout !
   }

   // On le note !

   std::stringstream ss;
   ss << "Decouverte de l'objet " << object.objectID << " sur la tuile " << object.tileID << " orient� en " << object.position;
   GameManager::log(ss.str());
}

int Carte::getX(const int id) const noexcept {
   return id % colCount;
}
int Carte::getY(const int id) const noexcept {
   return id / colCount;
}
int Carte::getRowCount() const noexcept {
   return rowCount;
}

int Carte::getColCount() const noexcept {
   return colCount;
}

int Carte::getNbTiles() const noexcept {
   return getRowCount() * getColCount();
}

int Carte::getNbTilesDecouvertes() const noexcept {
   return nbTilesDecouvertes;
}

MapTile& Carte::getTile(const int id) {
   if (id < 0 || id >= getNbTiles())
      throw tile_inexistante{};
   return tiles[id];
}

const MapTile& Carte::getTile(const int id) const {
   if (id < 0 || id >= getNbTiles())
      throw tile_inexistante{};
   return tiles[id];
}

std::vector<unsigned int> Carte::getObjectifs() const noexcept {
   return objectifs;
}

std::vector<unsigned int>& Carte::getObjectifs() {
   return objectifs;
}

std::map<unsigned int, ObjectInfo> Carte::getMurs() {
   return murs;
}

std::map<unsigned int, ObjectInfo> Carte::getPortes() {
   return portes;
}

std::map<unsigned int, ObjectInfo> Carte::getFenetres() {
   return fenetres;
}

std::map<unsigned int, ObjectInfo> Carte::getActivateurs() {
   return activateurs;
}

bool Carte::objectExist(const int objet) const noexcept {
   return murs.find(objet) != murs.end()
      || portes.find(objet) != portes.end()
      || fenetres.find(objet) != fenetres.end()
      || activateurs.find(objet) != activateurs.end();
}
