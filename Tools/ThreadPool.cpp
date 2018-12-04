#ifndef TREAD2_POOL_H
#define TREAD2_POOL_H

#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>

class th_pool {
    std::vector<std::thread> th;
    std::deque<std::function<void()>> taches;
    std::mutex mutex_taches;
    std::atomic<bool> meurs = false;
    std::condition_variable attente; // hum
public:
    th_pool() = default;
    void init(unsigned int nthr = std::thread::hardware_concurrency())
    {
        for (decltype(nthr) i = 0; i != nthr; ++i)
            th.emplace_back(std::thread{ [&] {
            while (!meurs) {
                auto f = extraire();
                f();
            }
        } });
    }
    void join() {
        for (auto & thr : th)
            thr.join();
    }
    ~th_pool() {
        terminer();
        attente.notify_all();
        for (auto & thr : th)
            thr.join();
    }
    void ajouter(std::function<void()> tache) {
        std::lock_guard<std::mutex> _{ mutex_taches };
        taches.push_back(tache);
        attente.notify_one();
    }
    std::function<void()> extraire() {
        std::unique_lock<std::mutex> verrou{ mutex_taches };
        if (taches.empty())
            attente.wait(verrou, [&] { return !taches.empty() || meurs; });
        if (!taches.empty()) {
            auto f = taches.front();
            taches.pop_front();
            return f;
        }
        return [] {}; // Null object
    }
    void terminer() {
        meurs = true;
    }
};
#endif //PROFILER_H
