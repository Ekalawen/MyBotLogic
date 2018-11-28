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

#ifdef _DEBUG
#define PROFILER_DEBUG
#endif
//
//#ifdef PROFILER_DEBUG
//#define PROFILER_LOG(logger, text, autoEndLine) logger.Log(text, autoEndLine)
//#define PROFILER_LOG(logger, text) logger.Log(text)
//#else
//#define PROFILER_LOG(logger, text, autoEndLine) 0
//#define PROFILER_LOG(logger, text) 0
//#endif
//
//#ifdef PROFILER_DEBUG
//#define PROFILER_LOG_POINTEUR(logger, text, autoEndLine) logger->Log(text, autoEndLine)
//#define PROFILER_LOG_POINTEUR(logger, text) logger->Log(text)
//#else
//#define PROFILER_LOG_POINTEUR(logger, text, autoEndLine) 0
//#define PROFILER_LOG_POINTEUR(logger, text) 0
//#endif

template<bool keepRelease>
class Profiler {
private:
   Logger* logger;
   string nomMethode;
   stringstream ss{};
   Minuteur::time_point_t debut;
   Minuteur::time_point_t fin;
   bool afficheDuree;

public:
   Profiler(Logger& _logger, string _nomMethode, bool _afficheDuree = true, bool _keepRelease = false) : nomMethode{ _nomMethode }, logger{ &_logger }, afficheDuree{ _afficheDuree } {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif
      ss.str("");
      //ss << Minuteur::nowStr() << " " << nomMethode;
      //logger->Log(ss.str());
      //ss.str("");
      if (afficheDuree) {
         debut = Minuteur::now();
      }
#ifndef PROFILER_DEBUG
      }
#endif
   }
   //Profiler::Profiler(const Profiler & other) : logger{ other.logger }, nomMethode{ other.nomMethode } {}
   ~Profiler() {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif
         if (afficheDuree) {
            fin = Minuteur::now();

            ss << "Duree " << nomMethode << " : " << Minuteur::dureeMicroseconds(debut, fin) / 1000.f << "ms";
            logger->Log(ss.str());
            ss.str("");
         }
#ifndef PROFILER_DEBUG
      }
#endif
   }

   template<class T>
   Profiler& operator<< (T&& message) {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif
      ss << message;
      logger->Log(ss.str(), false);
      ss.str("");
#ifndef PROFILER_DEBUG
   }
#endif
      return *this;
   }
   Profiler& operator<< (ostream& (*pf)(ostream&)) {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif
      ss << pf;
      logger->Log(ss.str(), false);
      ss.str("");
#ifndef PROFILER_DEBUG
   }
#endif
      return *this;
   }
};

using ProfilerRelease = Profiler<true>;
using ProfilerDebug = Profiler<false>;

#endif //PROFILER_H
