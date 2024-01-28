#ifndef SIMBUS_H
#define SIMBUS_H

// std
#include <atomic>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <chrono>
#include <cassert>

//#include "exports.h"
#include "busbase.h"
#include <utils/tlogger.h>

class SimBus : public BusBase
{
public:
    // Constructors / Destructor
    SimBus() = delete;
    SimBus(const std::string name);
    ~SimBus();

    void destroy();
    int open();
    int close();
    int updatePDO();
    int sendSDO();
    int recvSDO();

    static std::vector<std::string> getAdapterNames() {
        std::vector<std::string> result;
        result.push_back(std::string("Simulator"));
        TDEBUG("Simulator");
        return result;
    }

public:
//    static SimTbox* getInstance() {
//        if(instance == NULL)
//            instance = new Singleton();
//        return instance;
//    }


//    void destroy();
//    int init();
//    int connect(std::string nicName);
//    int disconnect();

private:

//    class Impl;
//    Impl* impl_;
};

// init static member
// SimTbox* SimTbox::instance = nullptr;

//extern "C" __declspec(dllexport) SimBus* getInstance();
//extern "C" __declspec(dllexport) int add(int a, int b);

#if 0
extern "C" __declspec(dllexport) SimTbox* __cdecl create()
{
    return new SimTbox;
}
#endif

#endif // SIMBUS_H
