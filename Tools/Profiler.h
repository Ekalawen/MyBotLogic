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

class Profiler {
private:
   Logger* logger;
   string nomMethode;
   stringstream ss{};
   Minuteur::time_point_t debut;
   Minuteur::time_point_t fin;

public:
   Profiler(Logger& _logger, string _nomMethode) : nomMethode{ _nomMethode }, logger{ &_logger } {
      ss.clear();
      ss << Minuteur::nowStr() << " " << nomMethode;
      logger->Log(ss.str());
      ss.clear();

      debut = Minuteur::now();
   }
   ~Profiler() {
      fin = Minuteur::now();

      ss.clear();
      ss << Minuteur::nowStr() << " " << "Duree "<< nomMethode << ":" << Minuteur::dureeMicroseconds(debut, fin) / 1000.f << "ms";
      logger->Log(ss.str());
      ss.clear();
   }
   ostream& operator<< (string message) {
      ss << message;
   }
};
#endif //PROFILER_H
