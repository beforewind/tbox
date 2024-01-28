
//#include "simobject.h"
//#include "simtpu.h"

#include <ethercat/simbus.h>


SimBus::SimBus(const std::string name) :
    BusBase("simbus", name) {
    //
    TDEBUG("This is SimBus, bus type is: {}, bus name is:{}", type(), getName());
}

SimBus::~SimBus() {

}

void SimBus::destroy()
{
    delete this;
}

int SimBus::open()
{

    return 0;
}

int SimBus::close()
{
    return 0;
}

int SimBus::updatePDO()
{
    return 0;
}

int SimBus::sendSDO()
{
    return 0;
}

int SimBus::recvSDO()
{
    return 0;
}

//bool BusBase::busIsAvailable(const std::string& name) {
//    return true;
//}









