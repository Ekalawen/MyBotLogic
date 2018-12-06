#ifndef PROFILER_H
#define PROFILER_H

#include "MyBotLogic/Tools/Minuteur.h"
#include "Logger.h"
#include "MyBotLogic/GameManager.h"
#include "MyBotLogic/Tools/VersionLivraison.h"

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>

using std::string;
using std::stringstream;
using std::ostream;
using std::move;
using std::endl;

#ifdef _DEBUG
#define PROFILER_DEBUG
#endif

#ifdef PROFILER_DEBUG
#define PROFILER_LOG_ENDL(autoEndLine) log(autoEndLine)
#define PROFILER_LOG() log()
#else
#define PROFILER_LOG_ENDL(autoEndLine) 0
#define PROFILER_LOG() 0
#endif

#ifndef VERSION_LIVRAISON

class GameManager;

template<bool keepRelease>
class Profiler {
private:
   Logger* logger;
   string nomMethode;
   stringstream ss{};
   Minuteur::time_point_t debut;
   Minuteur::time_point_t fin;
   Minuteur::time_point_t tempsDebutProgramme;
   bool afficheDuree;
   std::chrono::microseconds dureeMethode;
   std::vector<string>* elemsJSON;
   std::mutex* mutex;

public:
   Profiler(Logger& _logger, string _nomMethode, std::vector<string>& _elemsJSON, std::mutex& _mutex, Minuteur::time_point_t _tempsDebutProgramme, bool _afficheDuree = true, bool _keepRelease = false, std::chrono::microseconds _dureeMethode = 0us) : nomMethode{ _nomMethode }, logger{ &_logger }, elemsJSON{ &_elemsJSON }, mutex{ &_mutex }, tempsDebutProgramme{ _tempsDebutProgramme }, afficheDuree{ _afficheDuree }, dureeMethode{ _dureeMethode } {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif // ! PROFILER_DEBUG
         ss.str("");
         if (afficheDuree) {
            debut = Minuteur::now();
         }
#ifndef PROFILER_DEBUG
      }
#endif // ! PROFILER_DEBUG
   }
   ~Profiler() {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif // ! PROFILER_DEBUG
         if (afficheDuree) {
            fin = Minuteur::now();
            int dureeReel = Minuteur::dureeMicroseconds(debut, fin);
            ss << "Duree " << nomMethode << " : " << dureeReel / 1000.f << "ms" << endl;
            if (dureeReel > dureeMethode.count() && dureeMethode > 0us) ss << "Duree " << nomMethode << " : " << "ALERTE TROP LONG de " << ((dureeReel - dureeMethode.count()) / 1000.f) << "ms" << endl;
            mutex->lock();
            log(false);
            mutex->unlock();

            ss << "{"
               << "\"name\": " << "\"" << nomMethode << "\","
               << "\"pid\": " << "\"" << std::this_thread::get_id() << "\","
               << "\"ts\": " << "\"" << duration_cast<microseconds>(debut- tempsDebutProgramme).count() << "\","
               << "\"ph\": " << "\"B\""
               << "}";
            mutex->lock();
            elemsJSON->push_back(ss.str());
            mutex->unlock();
            ss.str("");

            ss << "{"
               << "\"name\": " << "\"" << nomMethode << "\","
               << "\"pid\": " << "\"" << std::this_thread::get_id() << "\","
               << "\"ts\": " << "\"" << duration_cast<microseconds>(fin - tempsDebutProgramme).count() << "\","
               << "\"ph\": " << "\"E\""
               << "}";
            mutex->lock();
            elemsJSON->push_back(ss.str());
            mutex->unlock();
            ss.str("");
         }

      }
#ifndef PROFILER_DEBUG
   }
#endif // ! PROFILER_DEBUG

private:
   void log(bool _bAutoNewLine = true) {
      logger->Log(ss.str(), _bAutoNewLine);
      ss.str("");
   }
public:
   template<class T>
   Profiler& operator<< (T&& message) {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif // ! PROFILER_DEBUG
         ss << message;

         PROFILER_LOG_ENDL(false);

#ifndef PROFILER_DEBUG
      }
#endif // ! PROFILER_DEBUG
      return *this;
   }
   Profiler& operator<< (ostream& (*pf)(ostream&)) {
#ifndef PROFILER_DEBUG
      if constexpr (keepRelease) {
#endif // ! PROFILER_DEBUG
         ss << pf;

         PROFILER_LOG_ENDL(false);

#ifndef PROFILER_DEBUG
      }
#endif // ! PROFILER_DEBUG
      return *this;
   }
};

#endif // ! VERSION_LIVRABLE

// VERSION POUR LA LIVRAISON

#ifdef VERSION_LIVRAISON

template<bool keepRelease>
class Profiler {
public:
   Profiler(Logger& _logger, string _nomMethode, std::vector<string>& _elemsJSON, std::mutex& _mutex, Minuteur::time_point_t _tempsDebutProgramme, bool _afficheDuree = true, bool _keepRelease = false, std::chrono::microseconds _dureeMethode = 0us) {}
   ~Profiler() {}

   template<class T>
   Profiler& operator<< (T&& message) {
      return *this;
   }
   Profiler& operator<< (ostream& (*pf)(ostream&)) {
      return *this;
   }
};

#endif // VERSION_LIVRAISON

using ProfilerRelease = Profiler<true>;
using ProfilerDebug = Profiler<false>;

#endif //PROFILER_H
