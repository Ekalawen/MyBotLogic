#ifndef TREAD_POOL_H
#define TREAD_POOL_H

#include <vector>
#include <thread>

class ThreadPool {
private:
   std::vector<std::thread> workers;

public:
   ThreadPool() : workers{ } { }

   ~ThreadPool() {
      joinAll();
   }

   void addThread(std::thread&& thread) {
      workers.push_back(std::move(thread));
   }

   void joinAll() {
      for (std::thread& t : workers) {
         if (t.joinable()) {
            t.join();
         }
      }
   }
};
#endif //PROFILER_H
