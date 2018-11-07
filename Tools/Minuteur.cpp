#include "Minuteur.h"

#include <chrono>

namespace chrono = std::chrono;

chrono::time_point<chrono::high_resolution_clock> Minuteur::now() {
   return chrono::high_resolution_clock::now();
}

Minuteur::duree_t Minuteur::dureeHours(time_point_t _instantAvant, time_point_t _instantApres) {
   return chrono::duration_cast<chrono::hours>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMinutes(time_point_t _instantAvant, time_point_t _instantApres) {
   return chrono::duration_cast<chrono::minutes>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeSeconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return chrono::duration_cast<chrono::seconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMilliseconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return chrono::duration_cast<chrono::milliseconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMicroseconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return chrono::duration_cast<chrono::microseconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeNanoseconds(time_point_t _instantAvant, time_point_t _instantApres){
   return chrono::duration_cast<chrono::nanoseconds>(_instantApres - _instantAvant).count();
}

Minuteur::duree_t Minuteur::getTempsRestant() {
   return timeToReech - dT;
}
bool Minuteur::isFinished() {
   return finished;
}
bool Minuteur::isStarted() {
   return started;
}
bool Minuteur::isConfigured() {
   return timeToReech != 0;
}
void Minuteur::start() {
    oldTime = now();
    finished = false;
    started = true;
}

void Minuteur::refresh() {
   if (!finished && started) {
      dT = dureeSeconds(oldTime, now());
      finished = dT == timeToReech;
      started = !finished;
      timeToReech = (finished) ? 0 : timeToReech;
   }
}

void Minuteur::setDecompte(duree_t _timeToReech) {
   this->timeToReech = _timeToReech;
}