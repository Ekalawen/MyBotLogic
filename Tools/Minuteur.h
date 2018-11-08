#ifndef MINUTEUR_H
#define MINUTEUR_H

#include <chrono>
#include <string>

namespace chrono = std::chrono;
using std::string;

class Minuteur {
public:
   using duree_t = unsigned int;
   using time_point_t = chrono::high_resolution_clock::time_point;
public:
   static time_point_t now();
   static string nowStr();

   static duree_t dureeHours(time_point_t _instantAvant, time_point_t _instantApres);
   static duree_t dureeMinutes(time_point_t _instantAvant, time_point_t _instantApres);
   static duree_t dureeSeconds(time_point_t _instantAvant, time_point_t _instantApres);
   static duree_t dureeMilliseconds(time_point_t _instantAvant, time_point_t _instantApres);
   static duree_t dureeMicroseconds(time_point_t _instantAvant, time_point_t _instantApres);
   static duree_t dureeNanoseconds(time_point_t _instantAvant, time_point_t _instantApres);

private:
   time_point_t oldTime;
   duree_t timeToReech;
   duree_t dT;
   bool finished;
   bool started;

public:
   Minuteur() : Minuteur(duree_t{}) {}
   Minuteur(duree_t _timeToReech) : timeToReech{ _timeToReech }, dT{}, finished{}, oldTime{}, started{} {}
   ~Minuteur() = default;

   duree_t getTempsRestant();
   void setDecompte(duree_t _timeToReech);
   bool isFinished();
   bool isStarted();
   bool isConfigured();
   void start();
   void refresh();
};

#endif //MINUTEUR_H
