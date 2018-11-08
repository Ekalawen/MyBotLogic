#ifndef PROFILER_H
#define PROFILER_H

#include "MyBotLogic/Tools/Minuteur.h"
#include "Logger.h"

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>

using std::string;
using std::stringstream;
using std::ostream;
using std::move;
using std::endl;

class Profiler {
private:
   Logger* logger;
   string nomMethode;
   stringstream ss{};
   Minuteur::time_point_t debut;
   Minuteur::time_point_t fin;
   bool afficheDuree;

public:
   Profiler(Logger& _logger, string _nomMethode, bool _afficheDuree = true) : nomMethode{ _nomMethode }, logger{ &_logger }, afficheDuree{ _afficheDuree } {
      ss.str("");
      //ss << Minuteur::nowStr() << " " << nomMethode;
      //logger->Log(ss.str());
      //ss.str("");
      if (afficheDuree) {
         debut = Minuteur::now();
      }
   }
   //Profiler::Profiler(const Profiler & other) : logger{ other.logger }, nomMethode{ other.nomMethode } {}
   ~Profiler() {
      if (afficheDuree) {
         fin = Minuteur::now();

         ss << "Duree " << nomMethode << " : " << Minuteur::dureeMicroseconds(debut, fin) / 1000.f << "ms";
         logger->Log(ss.str());
         ss.str("");
      }
   }

   template<class T>
   Profiler& operator<< (T&& message) {
      ss << message;
      logger->Log(ss.str(), false);
      ss.str("");
      return *this;
   }
   Profiler& operator<< (ostream& (*pf)(ostream&)) {
      ss << pf;
      logger->Log(ss.str(), false);
      ss.str("");
      return *this;
   }
};
#endif //PROFILER_H
