#include "Minuteur.h"

#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

using std::stringstream;
using std::string;

namespace chrono = std::chrono;

Minuteur::time_point_t Minuteur::now() {
   return chrono::high_resolution_clock::now();
}

string Minuteur::nowStr() {
   auto now = std::chrono::system_clock::now();
   auto in_time_t = std::chrono::system_clock::to_time_t(now);

   struct tm time_info;
   localtime_s(&time_info, &in_time_t);

   std::stringstream ss;
   ss << std::put_time(&time_info, "%Y-%m-%d %X");
   return ss.str();
}

Minuteur::duree_t Minuteur::dureeHours(time_point_t _instantAvant, time_point_t _instantApres) {
   return (Minuteur::duree_t)chrono::duration_cast<chrono::hours>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMinutes(time_point_t _instantAvant, time_point_t _instantApres) {
   return (Minuteur::duree_t)chrono::duration_cast<chrono::minutes>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeSeconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return (Minuteur::duree_t)chrono::duration_cast<chrono::seconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMilliseconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return (Minuteur::duree_t)chrono::duration_cast<chrono::milliseconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeMicroseconds(time_point_t _instantAvant, time_point_t _instantApres) {
   return (Minuteur::duree_t)chrono::duration_cast<chrono::microseconds>(_instantApres - _instantAvant).count();
}
Minuteur::duree_t Minuteur::dureeNanoseconds(time_point_t _instantAvant, time_point_t _instantApres){
   return (Minuteur::duree_t)chrono::duration_cast<chrono::nanoseconds>(_instantApres - _instantAvant).count();
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
      dT = dureeMilliseconds(oldTime, now());
      finished = dT == timeToReech;
      started = !finished;
      timeToReech = (finished) ? 0 : timeToReech;
   }
}

void Minuteur::setDecompte(duree_t _timeToReech) {
   this->timeToReech = _timeToReech;
}