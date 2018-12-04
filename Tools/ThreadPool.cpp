#include "ThreadPool.h"
#include "MyBotLogic/GameManager.h"

#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <deque>
#include <functional>
#include <mutex>

void th_pool::init(unsigned int nthr)
{
    for (decltype(nthr) i = 0; i != nthr; ++i)
        th.emplace_back(std::thread{ [&] {
        while (!meurs) {
            auto f = extraire();
            f(*gm);
        }
    } });
}

void th_pool::setGM(GameManager& g) {
    gm = &g;
}

th_pool::~th_pool() {
    terminer();
    attente.notify_all();
    for (auto & thr : th)
        thr.join();
}

void th_pool::joinAll() {
    gm->cond.wait(mon_verrou);
    while (count != 0);
}

void th_pool::ajouter(std::function<void(GameManager&)> tache) {
    mon_verrou.lock();
    std::lock_guard<std::mutex> _{ mutex_taches };
    taches.push_back(tache);
    attente.notify_one();
}

std::function<void(GameManager&)> th_pool::extraire() {
    std::unique_lock<std::mutex> verrou{ mutex_taches };
    if (taches.empty())
        attente.wait(verrou, [&] { return !taches.empty() || meurs; });
    if (!taches.empty()) {
        auto f = taches.front();
        taches.pop_front();
        ++count;
        return f;
    }
    return {}; // Null object
}

void th_pool::terminer() {
    meurs = true;
}
