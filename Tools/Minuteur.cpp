#include "Minuteur.h"

std::chrono::time_point<std::chrono::high_resolution_clock> Minuteur::now() {
   return std::chrono::high_resolution_clock::now();
}

Minuteur::duree_t Minuteur::dureeHours(time_point_t _instantAvant, time_point_t _instantApres) {
   return std::chrono::duration_cast<std::chrono::hours>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMinutes(time_point_t _instantAvant, time_point_t _instantApres) {
   return std::chrono::duration_cast<std::chrono::minutes>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeSeconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return std::chrono::duration_cast<std::chrono::seconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMilliseconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return std::chrono::duration_cast<std::chrono::milliseconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMicroseconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return std::chrono::duration_cast<std::chrono::microseconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeNanoseconds(time_point_t _instantAvant, time_point_t _instantApres){
   return std::chrono::duration_cast<std::chrono::nanoseconds>(_instantApres - _instantAvant).count();
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