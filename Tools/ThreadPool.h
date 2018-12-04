#ifndef TREAD2_POOL_H
#define TREAD2_POOL_H

#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>

class GameManager;

class th_pool {
    std::vector<std::thread> th;
    std::deque<std::function<void(GameManager&)>> taches;
    std::mutex mutex_taches;
    std::atomic<bool> meurs = false;
    std::condition_variable attente; // hum
    GameManager* gm;
    std::unique_lock<std::mutex> mon_verrou;
public:
    void joinAll();
    std::atomic<int> count = 0;
    th_pool() = default;
    void init(unsigned int nthr = std::thread::hardware_concurrency());
    void setGM(GameManager& g);
    ~th_pool();
    void ajouter(std::function<void(GameManager&)> tache);
    std::function<void(GameManager&)> extraire();
    void terminer();
};
#endif //PROFILER_H
