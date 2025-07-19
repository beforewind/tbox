
// function examples for using tbox
//

#include <iostream>
#include <fstream>
#include <functional>
#include <chrono>
#include <string>

#include "include//tbox.h"

#ifdef TBOX_DLL
#else
#include "ectbox/ectbox.h"
#endif

#include "tbox_helper.h"

///////////////////////////////////////////////////
// get fileName from the full path.
std::string getFileName(const std::string &filePathName)
{
    // std::string path = "D:\\mydoc\\VS-proj\\SMTDetector\\x64\\Release\\0001.bmp";
    size_t index = filePathName.find_last_of("\\");

    // std::string folderPath = filePathName.substr(0, index);
    std::string fileName = filePathName.substr(index + 1, -1);

    size_t index2 = filePathName.find_last_of(".");
    std::string extendName = filePathName.substr(index2 + 1, -1);

    std::cout << "filePathName:\t" << filePathName << std::endl;
    // std::cout << "folderPath:\t" << folderPath << std::endl;
    std::cout << "fileName:\t" << fileName << std::endl;
    std::cout << "extendName:\t" << extendName << std::endl;
    return fileName;
}

// check if file exitsts.
bool isFileExists(const std::string &filePathName)
{
    std::ifstream file(filePathName.c_str());
    return file.good();
}

// read file data, the fileSize info is implied in fileData.size()
void readFile(const std::string &filePathName, std::vector<uint8_t> &fileData)
{
    std::ifstream file(filePathName.c_str(), std::ofstream::binary);
    if (!file.good())
    {
        std::cout << "File " << filePathName << "does not exist!" << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    fileData.clear();
    fileData.resize(fileSize);
    file.read((char *)&fileData.data()[0], fileSize);
    file.close();
}

///////////////////////////////////////////////////

void printDinStatus(std::vector<uint8_t> array)
{
    unsigned short status = array.data()[0];
    unsigned short din = array.data()[1];

    std::cout << " status:" << std::hex << status;
    std::cout << " din:" << std::hex << din << std::endl;
}

// 打印DoutControl //
void printDoutControl(std::vector<uint8_t> array)
{
    unsigned short dout = array.data()[0];
    unsigned short control = array.data()[1];

    std::cout << " dout:" << std::hex << dout;
    std::cout << " control:" << std::hex << control << std::endl;
}

void printArray(std::vector<uint8_t> array)
{
    // for (int i=0;i<array.size();i++) {
    for (int i = array.size() - 1; i >= 0; i--)
    { // 倒过来输出，byte3_byte2_byte1_byte0
        std::cout << " " << std::hex << static_cast<unsigned short>(array.data()[i]);
    }
    std::cout << std::endl;
}

void printInt(std::vector<uint8_t> array)
{
    assert(array.size() == 4);
    uint32_t tmp = 0;
    for (int i = 0; i < array.size(); i++)
    {
        tmp = tmp + (static_cast<uint32_t>(array.data()[i])) * (1 << (8 * i));
        // std::cout << std::hex << static_cast<unsigned short>(array.data()[i]);
    }
    std::cout << "0x" << std::hex << tmp;
    std::cout << std::endl;
}

// convert 4bytes to a uint32_t
uint32_t convert2Int(std::vector<uint8_t> array)
{
    assert(array.size() == 4);
    uint32_t tmp = 0;
    for (int i = 0; i < array.size(); i++)
    {
        tmp = tmp + (static_cast<uint32_t>(array.data()[i])) * (1 << (8 * i));
    }
    return tmp;
}

// getServiceObject -> getServiceItemNameList -> readServiceItemData
void printServiceObject(Tbox *tbox, std::string &objName)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> timeBeignReadObj = std::chrono::high_resolution_clock::now();

    // std::map<uint16_t, ServiceObject> objMap;
    ServiceObject obj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, objName, obj) != 0)
        { // obj not exists!
            std::cout << "device:" << id << " objName:" << objName << " not exists!" << std::endl;
            continue;
        }

        // get item name list of obj //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, objName, itemNameList);

        // read and print obj info //
        std::cout << "device:" << id << " objName:" << objName << std::endl;
        for (auto itemName : itemNameList)
        {
            std::vector<uint8_t> itemData_tmp(4, 0); // TODO: the data length of item
            tbox->readServiceItemData(id, objName, itemName, itemData_tmp);
            std::cout << " -- item: " << itemName << " value:";
            printInt(itemData_tmp);
        }
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> timeEndReadObj = std::chrono::high_resolution_clock::now();
    auto durationReadObj = (timeEndReadObj - timeBeignReadObj);
    std::cout << "time of printServiceObject:" << double(durationReadObj.count()) / 1000000.0 << " ms" << std::endl;
}

///////////////////////////////////////////////////

int32_t reqCounter = 0;
int32_t rspCounter = 0;

// callback function
void reqCallback(Request req)
{
    reqCounter++;
    std::cout << "req " << req.reqId << " is accepted" << std::endl;

    for (auto rdout : req.reqDoutControl)
    {
        std::cout << "  device:" << rdout.first << " dout:";
        printDoutControl(rdout.second);
    }
    for (auto rdata : req.reqIoPattern)
    {
        std::cout << "  device:" << rdata.first << " stimuli:";
        printArray(rdata.second);
    }
}

void rspCallback(Response rsp)
{
    rspCounter++;
#if 0
    std::cout << "rsp " << rsp.reqId << " is returned" << std::endl;
    
    for (auto rmode : rsp.rspMode) {
        std::cout << "  device:" << rmode.first << " mode:" << rmode.second << std::endl;
    }
#endif

#if 0
    for (auto rdin : rsp.rspDinStatus) {
        std::cout << "  device:" << rdin.first << " din:";
        printArray(rdin.second);
    }
#endif

#if 0
    for (auto rdata : rsp.rspIoFeedback) {
        std::cout << "  device:" << rdata.first << " feedback:";
        printArray(rdata.second);
    }
#endif

#if 0
    for (auto rdata : rsp.rspMeasure) {
        std::cout << "  device:" << rdata.first << " measure:";
        printArray(rdata.second);
    }
#endif
    // may read measured value from SDO according to the field of rspMode //
}

void reqIdCallback(int32_t reqId)
{
    std::cout << "req " << reqId << " is returned" << std::endl;
}

void rspIdCallback(int32_t rspId)
{
    std::cout << "rsp " << rspId << " is returned" << std::endl;
}

void printRsp(Response rsp)
{
    std::cout << "rsp " << rsp.reqId << " is returned" << std::endl;

    for (auto rmode : rsp.rspMode)
    {
        std::cout << "  device:" << rmode.first << " mode:" << rmode.second << std::endl;
    }

    for (auto rdin : rsp.rspDinStatus)
    {
        std::cout << "  device:" << rdin.first << " din:";
        printDinStatus(rdin.second);
    }

    for (auto rdata : rsp.rspIoFeedback)
    {
        std::cout << "  device:" << rdata.first << " feedback:";
        printArray(rdata.second);
    }
}

void waitSync()
{
    // wait the last response
    while (rspCounter != reqCounter)
        ;
}

///////////////////////////////////////////////////
// replaced by the tool updatefirmware.exe
#if 0
// it should be called asynchronically, for its runtime very long.
void updateFirmware(Tbox* tbox, const std::string& filePathName)
{
    std::string fileName = getFileName(filePathName);
    int32_t fileSize;
    std::vector<uint8_t> fileData;
    readFile(filePathName, fileData);

#if 0
    // check file exists
    if (!isFileExists(filePathName)) {
        std::cout << "File " << filePathName << "does not exist!" << std::endl;
        return;
    }
#endif

    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
        tbox->updateFirmware(id, fileName, fileData);
    }

    // set device state?
}
#endif

///////////////////////////////////////////////////
// the runtime is very long to operate large size of FLASH in TPU, especially with hundreds of TPU.
// replaced by device info: deviceSN, adapterCount, adapterSNx, we store these SNs in FLASH as key to locate device/adapter related data.
// currently the SN bit size is 32bit, may be adjusted to 64bit.
void test_foeFlashMM(Tbox *tbox)
{
#if 0 // 
    // device中有一块flash，可以用于存储device的配置信息、adapter的数据 //
    // 这些数据应该是由上层软件序列化成std::vector<uint8_t>的二进制数据，并用objName作为key区分 //
    // 这里提供flash对象的读写接口，解释数据的含义应该由上层软件完成 //
    // config & adapters stored in flash, config is defined by upper
    FlashObject configObj;
    configObj.objName = "config";
    configObj.objType = Tbox::CONFIG;
    configObj.objData = std::vector<uint8_t>(8, 0x55);  // for test
    configObj.objSize = configObj.objData.size();

    FlashObject adapter0Obj;
    adapter0Obj.objName = "adapter0";
    adapter0Obj.objType = Tbox::ADAPTER;
    adapter0Obj.objData = std::vector<uint8_t>(8, 0x77);  // for test
    adapter0Obj.objSize = adapter0Obj.objData.size();

    FlashObject adapter1Obj;
    adapter1Obj.objName = "adapter1";
    adapter1Obj.objType = Tbox::ADAPTER;
    adapter1Obj.objData = std::vector<uint8_t>(8, 0xAA);  // for test
    adapter1Obj.objSize = adapter1Obj.objData.size();

    // devideId: from 1 to deviceCount
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
        tbox->writeFlashObject(id, configObj);
        tbox->writeFlashObject(id, adapter0Obj);
        tbox->writeFlashObject(id, adapter1Obj);
    }

    std::map<uint16_t, std::map<std::string, FlashObject>> flashObjMapMap;
    tbox->getFlashObjectList(flashObjMapMap);
    for (auto dev : flashObjMapMap) {
        for (auto obj : dev.second) {
            TDEBUG("objName:{}",obj.first);
        }
    }

    // read
    std::vector<std::string> objNameList;
    tbox->getFlashObjectList(i, type, objList);

    FlashObject obj;
    tbox->readFlashObject(1, "device", obj);
    TDEBUG("objName:{}",obj.objName);

    tbox->readFlashObject(1, "adapter1", obj);
    TDEBUG("objName:{}",obj.objName);
#else
    FlashObject configObj;
    configObj.objName = "config";
    configObj.objType = 1;  // Tbox::CONFIG;
    configObj.objData = std::vector<uint8_t>(8, 0x55);  // for test
    configObj.objSize = configObj.objData.size();

    FlashObject adapter0Obj;
    adapter0Obj.objName = "adapter0";
    adapter0Obj.objType = 2;  // Tbox::ADAPTER;
    adapter0Obj.objData = std::vector<uint8_t>(8, 0x77);  // for test
    adapter0Obj.objSize = adapter0Obj.objData.size();

    FlashObject adapter1Obj;
    adapter1Obj.objName = "adapter1";
    adapter1Obj.objType = 2;  // Tbox::ADAPTER;
    adapter1Obj.objData = std::vector<uint8_t>(8, 0xAA);  // for test
    adapter1Obj.objSize = adapter1Obj.objData.size();

    // devideId: from 1 to deviceCount
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
        tbox->writeFlashObject(id, configObj);
        tbox->writeFlashObject(id, adapter0Obj);
        tbox->writeFlashObject(id, adapter1Obj);
    }

/*
    std::map<uint16_t, std::map<std::string, FlashObject>> flashObjMapMap;
    tbox->getFlashObjectList(flashObjMapMap);
    for (auto dev : flashObjMapMap) {
        for (auto obj : dev.second) {
            TDEBUG("objName:{}",obj.first);
        }
    }
*/
    // read
    std::vector<FlashObject> flashObjList;
    std::vector<std::string> objNameList;
    tbox->getFlashObjectList(1, 1, flashObjList);

    FlashObject obj;
    tbox->readFlashObject(1, obj);
    //TDEBUG("objName:{}",obj.objName);

    tbox->readFlashObject(1, obj);
    //TDEBUG("objName:{}",obj.objName);
#endif
}

///////////////////////////////////////////////////
void initDevice(Tbox *tbox)
{
    std::string deviceObjName = "device";
    std::map<uint16_t, ServiceObject> deviceObjMap;
    ServiceObject deviceObj;

#if 1
    printServiceObject(tbox, deviceObjName);
#else
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0) // obj not exists!
            continue;

        // get item name list of obj //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, deviceObjName, itemNameList);

        // get obj info //
        for (auto itemName : itemNameList)
        {
            std::vector<uint8_t> itemData_tmp(4, 0);
            tbox->readServiceItemData(id, deviceObjName, itemName, itemData_tmp);
            std::cout << "device:" << id << " device info: " << itemName << " value:";
            printInt(itemData_tmp);
        }
    }
#endif

    // set device, for example:
    // device0: deviceSN=0x11110001, adapterCount=2, adapterSN0=0xAAAA0001, adapterSN1=0xAAAA0001;
    // device1: deviceSN=0x11110002, adapterCount=2, adapterSN2=0xAAAA0002, adapterSN3=0xAAAA0003;
    uint32_t adapterNum = 0;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0) // obj not exists!
            continue;

        // get item name list //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, deviceObjName, itemNameList);

        // set deviceSN if "deviceSN" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "deviceSN") > 0)
        {
            uint32_t deviceSN = 0x11110000 + id;
            deviceObj.setItem("deviceSN", &deviceSN, sizeof(deviceSN));
        }

        // set adapterCount if "adapterCount" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterCount") > 0)
        {
            uint32_t adapterCount = 0x02;
            deviceObj.setItem("adapterCount", &adapterCount, sizeof(adapterCount));
        }

        // set adapterSN0 if "adapterSN0" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterSN0") > 0)
        {
            uint32_t adapterSN0 = 0xAAAA0000 + adapterNum;
            adapterNum++;
            deviceObj.setItem("adapterSN0", &adapterSN0, sizeof(adapterSN0));
        }

        // set adapterSN1 if "adapterSN1" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterSN1") > 0)
        {
            uint32_t adapterSN1 = 0xAAAA0000 + adapterNum;
            adapterNum++;
            deviceObj.setItem("adapterSN1", &adapterSN1, sizeof(adapterSN1));
        }

        deviceObjMap.insert({id, deviceObj});
    }
    tbox->writeServiceObject(deviceObjMap);
    tbox->readServiceObject(deviceObjMap);
#if 0
        uint32_t deviceSN_tmp;
        deviceObjMap.at(id).getItem("deviceSN", &deviceSN_tmp, sizeof(deviceSN_tmp));
        uint32_t adapterCount_tmp;
        deviceObjMap.at(id).getItem("adapterCount", &adapterCount_tmp, sizeof(adapterCount_tmp));
        uint32_t adapterSN0_tmp;
        deviceObjMap.at(id).getItem("adapterSN0", &adapterSN0_tmp, sizeof(adapterSN0_tmp));
#endif
}

int readDevice(Tbox *tbox)
{
    std::string deviceObjName = "device";
    std::map<uint16_t, ServiceObject> deviceObjMap;
    ServiceObject deviceObj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0)
        { // obj not exists!
            continue;
        }
        std::cout << "device" << id << " device info:" << std::endl;

        std::vector<uint8_t> itemData;
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, "device", "deviceSN", itemData);
        uint32_t deviceSN = convert2Int(itemData);
        std::cout << "  deviceSN=0x" << std::hex << deviceSN << std::endl;
        // device is not initialized in FLASH.
        if (deviceSN == 0xFFFFFFFF)
        {
            return -1;
        }

        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, "device", "adapterCount", itemData);
        uint32_t adapterCount = convert2Int(itemData);
        std::cout << "  adapterCount=0x" << std::hex << adapterCount << std::endl;
        // at most 8 adapter per device
        if (adapterCount > 8)
        {
            return -1;
        }

        std::string SNx = "adapterSN";
        for (char i = 0; i < adapterCount; i++)
        {
            itemData.clear();
            itemData.resize(4, 0);
            std::string adpSN = SNx + (std::to_string(i));
            tbox->readServiceItemData(id, "device", adpSN, itemData);
            uint32_t adapterSN = convert2Int(itemData);
            std::cout << "  adapterSN[" << std::to_string(i) << "]=0x" << std::hex << adapterSN << std::endl;
            if (adapterSN == 0xFFFFFFFF)
            {
                return -1;
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
// 使用fw中的generalObj作为访问通道，访问所有SDO对象 //
// 1. 访问单个item，用偏移地址方式  //
// 2. 访问整个SDO对象，用偏移地址方式，且可能需要多次访问，才能完成一个Config对象 //


void initAdapter(Tbox *tbox)
{

}

void writeAdapter(Tbox *tbox)
{
    // write header
    

    // write data

}

void readAdapterHeader(Tbox *tbox)
{

}

void readAdapterData(Tbox *tbox)
{

}


///////////////////////////////////////////////////
///////////////////////////////////////////////////

// hardware info: hversion, fversion, dincount, doutcount, bankcount, tpperbank, iocount
void readHardware(Tbox *tbox)
{
    // 运行前，需要获取所有device的硬件信息，如bank数目、io数目、硬件版本等 //
    // hardware信息是每个mcu表示的device内置的参数，应该为只读类型 //
    std::string hardObjName = "hardware";
    std::map<uint16_t, ServiceObject> hardObjMap;
    ServiceObject hardObj;

#if 0
    printServiceObject(tbox, hardObjName);
#endif

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, hardObjName, hardObj) != 0)
        { // obj not exists!
            continue;
        }
        std::cout << "device" << id << " hardware info:" << std::endl;

        std::vector<uint8_t> itemData;

        // hversion
        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, hardObjName, "hversion", itemData);
        uint32_t hversion = convert2Int(itemData);
        std::cout << "  hardware version=0x" << std::hex << hversion << std::endl;

        // fversion
        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, hardObjName, "fversion", itemData);
        uint32_t fversion = convert2Int(itemData);
        std::cout << "  firmware version=0x" << std::hex << fversion << std::endl;

        // dincount
        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, hardObjName, "dincount", itemData);
        uint32_t dincount = convert2Int(itemData);
        std::cout << "  dincount=0x" << std::hex << dincount << std::endl;

        // doutcount
        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, hardObjName, "doutcount", itemData);
        uint32_t doutcount = convert2Int(itemData);
        std::cout << "  doutcount=0x" << std::hex << doutcount << std::endl;

        // bankcount
        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, hardObjName, "bankcount", itemData);
        uint32_t bankcount = convert2Int(itemData);
        std::cout << "  bankcount=0x" << std::hex << bankcount << std::endl;

        // iocount
        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, hardObjName, "iocount", itemData);
        uint32_t iocount = convert2Int(itemData);
        std::cout << "  iocount=0x" << std::hex << iocount << std::endl;
    }
}

///////////////////////////////////////////////////
// the function shows 2 method to access service object
// 1. write/read service object at once using tbox->writeServiceObject.
// 2. write/read each item one by one using tbox->writeServiceItemData.
// the micro ACCESS_AT_ONCE switch between the 2 methods
#define ACCESS_AT_ONCE
void writeParameter(Tbox *tbox)
{
    // should set parameter of each device before run test //
    // parameter can be read and write //
    // a parameter service object includes several items such as current, voltage... //
    // each item has its own type, for example uint32_t or float_t //

    std::string paraObjName = "parameter";
    std::map<uint16_t, ServiceObject> paraObjMap;
    ServiceObject paraObj;

#ifdef ACCESS_AT_ONCE
#else
    std::vector<uint8_t> itemData(4, 0);
#endif

#if 0
    // read all
    printServiceObject(tbox, paraObjName);
#endif

    // set parameter
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, paraObjName, paraObj) != 0) // obj not exists!
            continue;

        // get the item name list of parameter //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, paraObjName, itemNameList);

        // set current if "current" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "current") > 0)
        {
            // float_t current = 10.1;  // mA  float should be convert to int //
            uint32_t current = 9; // mA
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("current", &current, sizeof(current));
#else
            itemData.resize(sizeof(current));
            std::memcpy(itemData.data(), &current, sizeof(current));
            tbox->writeServiceItemData(id, paraObjName, "current", itemData);
#endif
        }

        // set voltage if "voltage" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "voltage") > 0)
        {
            // float_t voltage = 6.1;  // V  float should be convert to int //
            uint32_t voltage = 0xFFF;  // 0xF00; // for test DAC
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("voltage", &voltage, sizeof(voltage));
#else
            itemData.resize(sizeof(voltage));
            std::memcpy(itemData.data(), &voltage, sizeof(voltage));
            tbox->writeServiceItemData(id, paraObjName, "voltage", itemData);
#endif
        }

#if 0        
        // set current if "delayus" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "delayus") > 0)
        {
            uint32_t delayus = 4; // ms
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("delayus", &delayus, sizeof(delayus));
#else
            // tbox->writeServiceItemData(id, paraObjName, "delayus", itemData);
#endif
        }
#endif

        // set lowthreshold if "lowthreshold" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "lowthreshold") > 0)
        {
            uint32_t lowthreshold = 10; // om
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("lowthreshold", &lowthreshold, sizeof(lowthreshold));
#else
            // tbox->writeServiceItemData(id, paraObjName, "lowthreshold", itemData);
#endif
        }

        // set highthreshold if "highthreshold" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "highthreshold") > 0)
        {
            uint32_t highthreshold = 10000; // om
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("highthreshold", &highthreshold, sizeof(highthreshold));
#else
            // tbox->writeServiceItemData(id, paraObjName, "highthreshold", itemData);
#endif
        }

        paraObjMap.insert({id, paraObj});
    }
#ifdef ACCESS_AT_ONCE
    tbox->writeServiceObject(paraObjMap);
    tbox->readServiceObject(paraObjMap);
#else
#endif

    // read and check
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {

#ifdef ACCESS_AT_ONCE
        uint32_t current_tmp;
        paraObjMap.at(id).getItem("current", &current_tmp, sizeof(current_tmp));
        std::cout << "device:" << id << " current value:" << current_tmp << std::endl;
        uint32_t voltage_tmp;
        paraObjMap.at(id).getItem("voltage", &voltage_tmp, sizeof(voltage_tmp));
        std::cout << "device:" << id << " voltage value:" << voltage_tmp << std::endl;
#else
        std::vector<uint8_t> itemData_tmp(4, 0);
        tbox->readServiceItemData(id, paraObjName, "current", itemData_tmp);

        // std::cout << "device:" << id << " current float value:" << current_tmp << " uint8_t:";
        std::cout << "device:" << id << " current value:";
        printInt(itemData_tmp);

        tbox->readServiceItemData(id, paraObjName, "voltage", itemData_tmp);
        // std::cout << "device:" << id << " voltage float value:" << voltage_tmp << " uint8_t:";
        std::cout << "device:" << id << " voltage value:";
        printInt(itemData_tmp);
#endif
    }
}

void writeParameter2(Tbox *tbox, uint32_t curr = 8)
{
    // should set parameter of each device before run test //
    // parameter can be read and write //
    // a parameter service object includes several items such as current, voltage... //
    // each item has its own type, for example uint32_t or float_t //

    std::string paraObjName = "parameter";
    std::map<uint16_t, ServiceObject> paraObjMap;
    ServiceObject paraObj;

#ifdef ACCESS_AT_ONCE
#else
    std::vector<uint8_t> itemData(4, 0);
#endif

#if 0
    // read all
    printServiceObject(tbox, paraObjName);
#endif

    // set parameter
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        if (tbox->getServiceObject(id, paraObjName, paraObj) != 0) // obj not exists!
            continue;

        // get the item name list of parameter //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, paraObjName, itemNameList);

        // set current if "current" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "current") > 0)
        {
            // float_t current = 10.1;  // mA  float should be convert to int //
            uint32_t current = curr; // 9;  // mA
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("current", &current, sizeof(current));
#else
            itemData.resize(sizeof(current));
            std::memcpy(itemData.data(), &current, sizeof(current));
            tbox->writeServiceItemData(id, paraObjName, "current", itemData);
#endif
        }

        // set voltage if "voltage" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "voltage") > 0)
        {
            // float_t voltage = 6.1;  // V  float should be convert to int //
            uint32_t voltage = 0xFFF; // for test DAC
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("voltage", &voltage, sizeof(voltage));
#else
            itemData.resize(sizeof(voltage));
            std::memcpy(itemData.data(), &voltage, sizeof(voltage));
            tbox->writeServiceItemData(id, paraObjName, "voltage", itemData);
#endif
        }

#if 0        
        // set current if "delayus" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "delayus") > 0)
        {
            uint32_t delayus = 4; // ms
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("delayus", &delayus, sizeof(delayus));
#else
            // tbox->writeServiceItemData(id, paraObjName, "delayus", itemData);
#endif
        }
#endif

        // set lowthreshold if "lowthreshold" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "lowthreshold") > 0)
        {
            uint32_t lowthreshold = 10; // om
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("lowthreshold", &lowthreshold, sizeof(lowthreshold));
#else
            // tbox->writeServiceItemData(id, paraObjName, "lowthreshold", itemData);
#endif
        }

        // set highthreshold if "highthreshold" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "highthreshold") > 0)
        {
            uint32_t highthreshold = 10000; // om
#ifdef ACCESS_AT_ONCE
            paraObj.setItem("highthreshold", &highthreshold, sizeof(highthreshold));
#else
            // tbox->writeServiceItemData(id, paraObjName, "highthreshold", itemData);
#endif
        }

        paraObjMap.insert({id, paraObj});
    }
#ifdef ACCESS_AT_ONCE
    tbox->writeServiceObject(paraObjMap);
    tbox->readServiceObject(paraObjMap);
#else
#endif

    // read and check
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {

#ifdef ACCESS_AT_ONCE
        uint32_t current_tmp;
        paraObjMap.at(id).getItem("current", &current_tmp, sizeof(current_tmp));
        std::cout << "device:" << id << " current value:" << current_tmp << std::endl;
        uint32_t voltage_tmp;
        paraObjMap.at(id).getItem("voltage", &voltage_tmp, sizeof(voltage_tmp));
        std::cout << "device:" << id << " voltage value:" << voltage_tmp << std::endl;
#else
        std::vector<uint8_t> itemData_tmp(4, 0);
        tbox->readServiceItemData(id, paraObjName, "current", itemData_tmp);

        // std::cout << "device:" << id << " current float value:" << current_tmp << " uint8_t:";
        std::cout << "device:" << id << " current value:";
        printInt(itemData_tmp);

        tbox->readServiceItemData(id, paraObjName, "voltage", itemData_tmp);
        // std::cout << "device:" << id << " voltage float value:" << voltage_tmp << " uint8_t:";
        std::cout << "device:" << id << " voltage value:";
        printInt(itemData_tmp);
#endif
    }
}

///////////////////////////////////////////////////
void readPdoInfo(Tbox *tbox)
{
// 以下四种类型，需要在上层软件中建立di/do/tx/rx pattern与adapter的pin的映射关系 //
// 因此提供了以下示例，用于获取这四类obj的数据结构，并在上层程序中保存和使用 //
#if 1
    // get digital output
    std::string doObjName = "reqDoutPDO";
    std::map<uint16_t, ServiceObject> doObjMap;
    ServiceObject doObj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        tbox->getServiceObject(id, doObjName, doObj);
        doObjMap.insert({id, doObj});
        std::cout << " device:" << id << " obj name : " << doObj.objName << " size : " << doObj.objSize << std::endl;
    }
#endif

#if 1
    // get digital input
    std::string diObjName = "respDinPDO";
    std::map<uint16_t, ServiceObject> diObjMap;
    ServiceObject diObj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        tbox->getServiceObject(id, diObjName, diObj);
        diObjMap.insert({id, diObj});
        std::cout << " device:" << id << " obj name : " << diObj.objName << " size : " << diObj.objSize << std::endl;
    }
#endif

#if 1
    // get tx io pattern
    std::string txObjName = "reqDataPDO";
    std::map<uint16_t, ServiceObject> txObjMap;
    ServiceObject txObj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        tbox->getServiceObject(id, txObjName, txObj);
        txObjMap.insert({id, txObj});
        std::cout << " device:" << id << " obj name : " << txObj.objName << " size : " << txObj.objSize << std::endl;
    }
#endif

#if 1
    // get rx io pattern
    std::string rxObjName = "respDataPDO";
    std::map<uint16_t, ServiceObject> rxObjMap;
    ServiceObject rxObj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        tbox->getServiceObject(id, rxObjName, rxObj);
        rxObjMap.insert({id, rxObj});
        std::cout << " device:" << id << " obj name : " << rxObj.objName << " size : " << rxObj.objSize << std::endl;
    }
#endif
}

///////////////////////////////////////////////////

std::map<int32_t, Request> reqMap;
std::map<int32_t, Response> rspMap;

Request generateSetReqest(Tbox *tbox)
{
    int32_t reqId = reqMap.size();
    Request req;

    std::vector<uint8_t> mainS_dout; // mainSwitch+dout
    mainS_dout.push_back(0x00);      // dout4,3,2,1
    mainS_dout.push_back(0x00);
    // mainS_dout.push_back(0x09);  // main switch // 0x09:1001-COMG/COML/LOOP/HF

    std::vector<uint8_t> ioPattern1, ioPattern2;
    ioPattern1.push_back(0x04); // 0x05:IO2,IO0;  0x39 0011_1001
    ioPattern1.push_back(0x00); // 0xA0:IO15, IO13, 0xC6 1100_0110

    ioPattern2.push_back(0x00); // 0x05:IO2,IO0;  0x39 0011_1001
    ioPattern2.push_back(0x00); // 0xA0:IO15, IO13, 0xC6 1100_0110
    REQ_SYNC_OBJ reqMode;
    uint16_t reqMode_u16;

    reqMode.setdout_valid = 1;
    reqMode.setio_valid = 1; // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
    reqMode.getio_valid = 0;
    reqMode.getmeasure_valid = 0;
    reqMode.getext_valid = 0;
    reqMode.reserved = 0;
    memcpy(&reqMode_u16, (uint16_t *)&reqMode, 2);
#if 0
    for (uint16_t id=1; id<=tbox->getDeviceCount();id++) {
        req.reqDoutControl.insert({id, mainS_dout});       // digital output //
        req.reqIoPattern.insert({id, ioPattern }); // IO pattern //
        req.reqMode.insert({id, reqMode_u16 });
    }
#else
    req.reqDoutControl.insert({1, mainS_dout}); // digital output //
    req.reqIoPattern.insert({1, ioPattern1});   // IO pattern //
    req.reqMode.insert({1, reqMode_u16});

    req.reqDoutControl.insert({2, mainS_dout}); // digital output //
    req.reqIoPattern.insert({2, ioPattern2});   // IO pattern //
    req.reqMode.insert({2, reqMode_u16});
#endif
    req.reqId = reqMap.size();
    reqMap.insert({req.reqId, req});
    return req;
}

Request generateGetRequest(Tbox *tbox)
{
    int32_t reqId = reqMap.size();
    Request req;

    std::vector<uint8_t> mainS_dout; // mainSwitch+dout
    mainS_dout.push_back(0x00);      // dout4,3,2,1
    mainS_dout.push_back(0x00);
    // mainS_dout.push_back(0x09);  // main switch // 0x09:1001-COMG/COML/LOOP/HF

    std::vector<uint8_t> ioPattern1, ioPattern2;
    ioPattern1.push_back(0x05); // 0x05:IO2,IO0;  0x39 0011_1001
    ioPattern1.push_back(0xA0); // 0xA0:IO15, IO13, 0xC6 1100_0110
    REQ_SYNC_OBJ reqMode;
    uint16_t reqMode_u16;

    reqMode.setdout_valid = 1;
    reqMode.setio_valid = 0; // 0-off, 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
    reqMode.getio_valid = 2; // 2;  // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
                             // 应该和setio_valid相反，即set的是stimu，那么get的是互补的resp，才能构成回路 //
    reqMode.getmeasure_valid = 0; // 获取测量值，用于精确计算阻值 //
    reqMode.getext_valid = 0;
    reqMode.reserved = 0;
    memcpy(&reqMode_u16, (uint16_t *)&reqMode, 2);
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        req.reqDoutControl.insert({id, mainS_dout}); // digital output //
        // req.reqIoPattern.insert({id, ioPattern }); // IO pattern //
        req.reqMode.insert({id, reqMode_u16});
    }

    req.reqId = reqMap.size();
    reqMap.insert({req.reqId, req});
    return req;
}

///////////////////////////////////////////////////

uint32_t getDeviceObjectItem(Tbox *tbox, uint16_t deviceId, const std::string &objName, const std::string &itemName)
{
    ServiceObject serviceObj;
    if (tbox->getServiceObject(deviceId, objName, serviceObj) != 0)
    { // obj not exists!
        std::cout << "";
        return 0;
    }

    std::vector<uint8_t> itemData;
    itemData.resize(4, 0);
    tbox->readServiceItemData(deviceId, objName, itemName, itemData);
    uint32_t objData = convert2Int(itemData);
    std::cout << "  objData=0x" << std::hex << objData << std::endl;

    return objData;
}

int32_t getDeviceObjectSize(Tbox *tbox, uint16_t deviceId, const std::string &objName)
{
    ServiceObject serviceObj;
    if (tbox->getServiceObject(deviceId, objName, serviceObj) != 0)
    { // obj not exists!
        std::cout << "";
        return 0;
    }

    return serviceObj.objSize;
}

void readDeviceList(Tbox *tbox, std::map<uint32_t, device_t> &deviceMap)
{
    deviceMap.clear();

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        device_t dev;
        dev.id = id;

        dev.deviceSN = getDeviceObjectItem(tbox, id, "device", "deviceSN");
#if 0 // v1.8 not support adapterXXX
        dev.adapterCount = getDeviceObjectItem(tbox, id, "device", "adapterCount");
        if (dev.adapterCount <= 0 || dev.adapterCount > 8)
        {
            dev.adapterCount = 0;
            continue;
        }
        for (char i = 0; i < dev.adapterCount; i++)
        {
            dev.adapterSN[i] = getDeviceObjectItem(tbox, id, "device", ("adapterSN" + (std::to_string(i))));
        }
#endif
        dev.hversion = getDeviceObjectItem(tbox, id, "hardware", "hversion");
        dev.fversion = getDeviceObjectItem(tbox, id, "hardware", "fversion");
        dev.dinCount = getDeviceObjectItem(tbox, id, "hardware", "dincount");
        dev.doutCount = getDeviceObjectItem(tbox, id, "hardware", "doutcount");
        dev.bankCount = getDeviceObjectItem(tbox, id, "hardware", "bankcount");
        dev.ioCount = getDeviceObjectItem(tbox, id, "hardware", "iocount");

        dev.current = getDeviceObjectItem(tbox, id, "parameter", "current");
        dev.voltage = getDeviceObjectItem(tbox, id, "parameter", "voltage");
#if 0
        dev.delayus = getDeviceObjectItem(tbox, id, "parameter", "delayus");
        dev.rsample = getDeviceObjectItem(tbox, id, "parameter", "rsample");
#endif
        dev.doSize = getDeviceObjectSize(tbox, id, "reqDoutPDO");
        dev.diSize = getDeviceObjectSize(tbox, id, "respDinPDO");
        dev.txStimuliSize = getDeviceObjectSize(tbox, id, "reqDataPDO");
        dev.rxFeedbackSize = getDeviceObjectSize(tbox, id, "respDataPDO");

        deviceMap.insert({dev.deviceSN, dev});
    }
}

void saveDeviceList(Tbox *tbox, std::map<uint32_t, device_t> &deviceMap)
{
    std::string deviceObjName = "device";
    std::map<uint16_t, ServiceObject> deviceObjMap;
    ServiceObject deviceObj;

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {

        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0) // obj not exists!
            continue;

        // get deviceSN
        uint32_t deviceSN = getDeviceObjectItem(tbox, id, "device", "deviceSN");

        if (deviceMap.count(deviceSN) == 0)
            continue;

        auto &dev = deviceMap[deviceSN];

        // get item name list //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, deviceObjName, itemNameList);

        // set deviceSN if "deviceSN" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "deviceSN") > 0)
        {
            uint32_t deviceSN = dev.deviceSN;
            deviceObj.setItem("deviceSN", &deviceSN, sizeof(deviceSN));
        }

        // set adapterCount if "adapterCount" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterCount") > 0)
        {
            uint32_t adapterCount = dev.adapterCount;
            deviceObj.setItem("adapterCount", &adapterCount, sizeof(adapterCount));
        }

        for (char j = 0; j < dev.adapterCount; j++)
        {
            std::string adapterSNx = "adapterSN" + std::to_string(j);
            if (std::count(itemNameList.begin(), itemNameList.end(), adapterSNx) > 0)
            {
                deviceObj.setItem(adapterSNx, &dev.adapterSN[j], sizeof(dev.adapterSN[j]));
            }
        }

        deviceObjMap.insert({id, deviceObj});
    }
    tbox->writeServiceObject(deviceObjMap);
    // read back
    tbox->readServiceObject(deviceObjMap);
}

///////////////////////////////////////////////////

void generateAdapterExample(std::map<uint32_t, adapter_t> &adapterMap, std::map<uint32_t, device_t> &deviceMap)
{
    adapter_t adp[4];

    // init adapters
    adp[0].adapterSN = 0x66660000; // will store in device, as the key to find in database
    adp[0].diCount = 1;            // store in database
    adp[0].doCount = 1;            // store in database
    adp[0].ioCount = 4;            // store in database

    adp[1].adapterSN = 0x66660001;
    adp[1].diCount = 1;
    adp[1].doCount = 1;
    adp[1].ioCount = 4;

    adp[2].adapterSN = 0x66660002;
    adp[2].diCount = 2;
    adp[2].doCount = 2;
    adp[2].ioCount = 8;

    adp[3].adapterSN = 0x66660003;
    adp[3].diCount = 2;
    adp[3].doCount = 2;
    adp[3].ioCount = 8;

    // clear adapter info in device
    for (auto &d : deviceMap)
    {
        d.second.adapterCount = 0;
    }

    // assign device
    for (uint16_t i = 0; i < 4; i++)
    {
        uint16_t deviceId = (i % deviceMap.size()) + 1; // distribute adapters to devices
        uint32_t deviceSN = 0;
        for (auto &d : deviceMap)
        {
            if (d.second.id == deviceId)
            {
                deviceSN = d.second.deviceSN;
                break;
            }
        }
        if (deviceMap.count(deviceSN) == 0) // does not exists
            continue;
        auto &dev = deviceMap[deviceSN];
        dev.adapterSN[dev.adapterCount] = adp[i].adapterSN;
        dev.adapterCount++;
        adp[i].deviceSN = dev.deviceSN;

        adapterMap.insert({adp[i].adapterSN, adp[i]});
    }

    //
    // save device outside
}

void generateAdapterMappingExample(std::map<uint32_t, adapter_t> &adapterMap)
{
    // multiple adapters are mounted on a device, start from io/di/do offset
    std::map<uint32_t, uint32_t> ioOffsetOfDevice;
    std::map<uint32_t, uint32_t> diOffsetOfDevice;
    std::map<uint32_t, uint32_t> doOffsetOfDevice;

    for (auto &a : adapterMap)
    {
        // init adapter

        // io mapping from adapter.io to device.io
        // should be edited by GUI
        a.second.ioCount = 4;
        uint32_t ioOffset;
        if (ioOffsetOfDevice.count(a.second.deviceSN))
        {
            ioOffset = ioOffsetOfDevice.at(a.second.deviceSN);
        }
        else
        {
            ioOffset = 0;
        }
        // update
        ioOffsetOfDevice[a.second.deviceSN] = ioOffset + a.second.ioCount;
        // for example
        a.second.ioMap.insert({0, ioOffset + 0}); // adapter.io -> device.io
        a.second.ioMap.insert({1, ioOffset + 1});
        a.second.ioMap.insert({2, ioOffset + 2});
        a.second.ioMap.insert({3, ioOffset + 3});

        // di mapping from adapter.di to device.di
        a.second.diCount = 2;
        uint32_t diOffset;
        if (diOffsetOfDevice.count(a.second.deviceSN))
        {
            diOffset = diOffsetOfDevice.at(a.second.deviceSN);
        }
        else
        {
            diOffset = 0;
        }
        // update
        diOffsetOfDevice[a.second.deviceSN] = diOffset + a.second.diCount;
        a.second.diMap.insert({0, diOffset + 0});
        a.second.diMap.insert({1, diOffset + 1});

        // do mapping from adapter.do to device.do
        a.second.doCount = 2;
        uint32_t doOffset;
        if (doOffsetOfDevice.count(a.second.deviceSN))
        {
            doOffset = doOffsetOfDevice.at(a.second.deviceSN);
        }
        else
        {
            doOffset = 0;
        }
        // update
        doOffsetOfDevice[a.second.deviceSN] = doOffset + a.second.diCount;
        a.second.doMap.insert({0, doOffset + 0});
        a.second.doMap.insert({1, doOffset + 1});
    }
}

void generateWireExample(std::map<uint32_t, adapter_t> &adapterMap, std::vector<connection_t> &wireList)
{
    wireList.clear();
    connection_t wire0, wire1, wire2;
    wire0.name = "wire0";
    wire0.adpFrom = adapterMap[0];
    wire0.pinFrom = 0;
    wire0.adpTo = adapterMap[2];
    wire0.pinTo = 0;
    wireList.push_back(wire0);

    wire1.name = "wire1";
    wire1.adpFrom = adapterMap[0];
    wire1.pinFrom = 1;
    wire1.adpTo = adapterMap[2];
    wire1.pinTo = 1;
    wireList.push_back(wire1);

    wire2.name = "wire2";
    wire2.adpFrom = adapterMap[1];
    wire2.pinFrom = 0;
    wire2.adpTo = adapterMap[3];
    wire2.pinTo = 0;
    wireList.push_back(wire2);
}

///////////////////////////////////////////////////
void generatePatternExample(std::map<uint32_t, device_t> &deviceMap, std::map<uint16_t, std::vector<uint8_t>> &doutPattern, std::map<uint16_t, std::vector<uint8_t>> &stimuliPattern)
{
    std::vector<uint8_t> stimuli1, stimuli2;
    stimuli1.resize(2, 0);
    stimuli2.resize(2, 0);

    stimuli1[0] = 0x05; // 0101_0000:IO5,IO7-LF,  ;0101, IO2,IO0--LF, IO3,IO1--HF
    stimuli1[1] = 0x00;

    stimuli2[0] = 0x05; // 0101_0000:IO5,IO7-LF,  ;0101, IO2,IO0--LF, IO3,IO1--HF
    stimuli2[1] = 0x00;

    for (auto &d : deviceMap)
    {
        auto &dev = d.second;
        uint16_t deviceId = dev.id;
        std::vector<uint8_t> dout;
        // stimuli1.resize(dev.txStimuliSize, 0);
        // stimuli2.resize(dev.txStimuliSize, 0);
        dout.resize(2, 0); // fixed 2 bytes
#if 1
        // stimuli[0] = 0x04;  // 0101_0000:IO5,IO7-LF,  ;0101, IO2,IO0--LF, IO3,IO1--HF
        // stimuli[1] = 0x00;
#else
        for (uint32_t i = 0; i < dev.txStimuliSize; i++)
        {                      // pattern的长度取决于txStimuliSize表示的byte个数 //
            stimuli[i] = 0x05; // 实际应该来自算法处理adapter的io生成的激励，0-off，1-激励 //
        }
#endif
        if (deviceId % 2)
        {
            stimuliPattern.insert({deviceId, stimuli1});
        }
        else
        {
            stimuliPattern.insert({deviceId, stimuli2});
        }

        for (uint32_t i = 0; i < dev.doSize; i++)
        {
            dout[i] = 0x00; // 实际来自控制adapter的数字输出，0-off，1-开启 //
        }
        doutPattern.insert({deviceId, dout});
    }
}

Request generateSetReqestExample(Tbox *tbox, std::map<uint16_t, std::vector<uint8_t>> &doutPattern, std::map<uint16_t, std::vector<uint8_t>> &stimuliPattern)
{
    int32_t reqId = reqMap.size();
    Request req;

#if 0
    REQ_SYNC_OBJ reqMode;
    uint16_t reqMode_u16;
    reqMode.setdout_valid = 1;
    reqMode.setio_valid = 1;  // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
    reqMode.getio_valid = 0;
    reqMode.getmeasure_valid = 0;
    reqMode.getext_valid = 0;
    reqMode.reserved = 0;
    memcpy(&reqMode_u16, (uint16_t*)&reqMode, 2);
#else
#endif

    std::map<uint16_t, uint16_t> doutModePattern;
    std::map<uint16_t, uint16_t> setioModePattern;

#if 0
    std::vector<uint8_t> mainS_dout;  // mainSwitch+dout
    mainS_dout.push_back(0x0F);  // dout4,3,2,1
    mainS_dout.push_back(0x00);  // control
#else
    for (auto &dout : doutPattern)
    {
        req.reqDoutControl.insert({dout.first, dout.second});
        doutModePattern.insert({dout.first, 1});
    }
#endif

#if 0
    std::vector<uint8_t> ioPattern;
    ioPattern.push_back(0x05);  //0x05:IO2,IO0;  0x39 0011_1001
    ioPattern.push_back(0xA0);  //0xA0:IO15, IO13, 0xC6 1100_0110
#else
    for (auto &io : stimuliPattern)
    {
        req.reqIoPattern.insert({io.first, io.second});
        setioModePattern.insert({io.first, 1});
    }
#endif

#if 0
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        req.reqDoutControl.insert({ id, mainS_dout });       // digital output //
        req.reqIoPattern.insert({ id, ioPattern }); // IO pattern //
        req.reqMode.insert({ id, reqMode_u16 });
    }
#else
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        uint16_t reqMode_u16 = 0;
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        if (doutModePattern.count(id) != 0)
        {
            pReqMode->setdout_valid = 1;
        }
        if (setioModePattern.count(id) != 0)
        {
            pReqMode->setio_valid = 1;
        }
        req.reqMode.insert({id, reqMode_u16});
    }
#endif

    req.reqId = reqMap.size();
    reqMap.insert({req.reqId, req});
    return req;
}

Request generateGetRequestExample(Tbox *tbox, std::map<uint16_t, std::vector<uint8_t>> &doutPattern)
{
    int32_t reqId = reqMap.size();
    Request req;

    std::map<uint16_t, uint16_t> doutModePattern;
    // std::map<uint16_t, uint16_t> setioModePattern;

    for (auto &dout : doutPattern)
    {
        req.reqDoutControl.insert({dout.first, dout.second});
        doutModePattern.insert({dout.first, 1});
    }

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        uint16_t reqMode_u16 = 0;
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        if (doutModePattern.count(id) != 0)
        {
            pReqMode->setdout_valid = 1;
        }
        pReqMode->getio_valid = 2; // 2;  // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
        req.reqMode.insert({id, reqMode_u16});
    }

    req.reqId = reqMap.size();
    reqMap.insert({req.reqId, req});
    return req;

#if 0
    std::vector<uint8_t> mainS_dout;  // mainSwitch+dout
    mainS_dout.push_back(0x0F);  // dout4,3,2,1
    mainS_dout.push_back(0x09);  // main switch // 0x09:1001-COMG/COML/LOOP/HF

    std::vector<uint8_t> ioPattern;
    ioPattern.push_back(0x05);  //0x05:IO2,IO0;  0x39 0011_1001
    ioPattern.push_back(0xA0);  //0xA0:IO15, IO13, 0xC6 1100_0110
    REQ_SYNC_OBJ reqMode;
    uint16_t reqMode_u16;

    reqMode.setdout_valid = 1;
    reqMode.setio_valid = 0;  // 0-off, 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
    reqMode.getio_valid = 2; // 2;  // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
        // 应该和setio_valid相反，即set的是stimu，那么get的是互补的resp，才能构成回路 //
    reqMode.getmeasure_valid = 0;  // 获取测量值，用于精确计算阻值 //
    reqMode.getext_valid = 0;
    reqMode.reserved = 0;
    memcpy(&reqMode_u16, (uint16_t*)&reqMode, 2);
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        req.reqDoutControl.insert({ id, mainS_dout });       // digital output //
        req.reqIoPattern.insert({ id, ioPattern }); // IO pattern //
        req.reqMode.insert({ id, reqMode_u16 });
    }
#endif
}

/////////////////////////////////////////////////////////
// components test

// generate an empty reqest for all devices
void generateSyncReq(Tbox *tbox, Request &req)
{
    req.reqDoutControl.clear();
    req.reqIoPattern.clear();
    req.reqMode.clear();

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        uint16_t reqMode_u16 = 0;
        req.reqMode.insert({id, reqMode_u16});
    }
}

// method: test resistor/capacitor/diode using PDO & SDO
// 0. write SDO measureObj with type/nominalValue/...
// 1. setIo: set one terminal(IO1,pinTo) to LF
// 2. setIo: set the other terminal(IO2,pinFrom) to HF
// 3. getMeasure: do measurement in the TPU which IO2(pinFrom) belongs to.
// 4. read SDO measureObj with measuredVale
// 支持两端点的元件测试，如电阻、电容和二极管 //
// TPU中增加了一个measure对象，查询关键字“measure” //
// 限制条件：pinFrom是HF端，独占一个TPU；pinTo是LF端，最好独占一个TPU；一个元件的pinTo和pinFrom可以在一个TPU上，也可以跨TPU。 //
// measure对象中的nominalValue是线束产品中的期望值 //
void testComponent(Tbox *tbox, std::vector<component2t_t> &resList)
{
    // std::vector<resistor_t> resList;
    resList.clear();
    component2t_t comp1;
    comp1.type = 0x100; // resistor
    if (tbox->getDeviceCount() >= 2)
    { // multi nodes
        comp1.deviceFrom = 1;
        comp1.deviceTo = 2;
        comp1.pinFrom = 3;
        comp1.pinTo = 3;
    }
    else
    { // only one node
        comp1.deviceFrom = 1;
        comp1.deviceTo = 1;
        comp1.pinFrom = 1;
        comp1.pinTo = 2;
    }
    comp1.nominalValue = 99;
    comp1.measuredValue = 0;
    comp1.unit = 0;
    resList.push_back(comp1);

#if 0
    if (tbox->getDeviceCount() >= 2) {
        component2t_t comp2;
        comp2.type = 0x100;  // resistor
        comp2.deviceFrom = 2;
        comp2.deviceTo = 2;
        comp2.pinFrom = 1;
        comp2.pinTo = 2;
        comp2.nominalValue = 99;
        comp2.measuredValue = 0;
        comp2.unit = 0;
        resList.push_back(comp2);
    }

    if (tbox->getDeviceCount() >= 2) {
        component2t_t comp3;
        comp3.type = 0x100;  // resistor
        comp3.deviceFrom = 1;
        comp3.deviceTo = 2;
        comp3.pinFrom = 3;
        comp3.pinTo = 3;
        comp3.nominalValue = 99;
        comp3.measuredValue = 0;
        comp3.unit = 0;
        resList.push_back(comp3);
    }
#endif

    std::string objName = "measure";
    printServiceObject(tbox, objName);

#if 0
    // 生成pinTo和pinFrom两个map，维护各自所在deviceId
    std::map<uint16_t, uint16_t> pinToList;
    std::map<uint16_t, uint16_t> pinFromList;
    for (resistor_t& res : resList) {
        pinToList.insert({res.deviceTo, res.pinTo});
        pinFromList.insert({res.deviceFrom, res.pinFrom});
    }
#endif

    // 写measure相关的参数到pinFrom所在节点的measureObj，如电阻类型、标称值等 //
    std::map<uint16_t, ServiceObject> objMap;
    for (component2t_t &comp : resList)
    {
        ServiceObject measureObj;
        tbox->getServiceObject(comp.deviceFrom, "measure", measureObj);
        measureObj.setItem("type", &comp1.type, 4);
        measureObj.setItem("nominal", &comp.nominalValue, 4);
        measureObj.setItem("value", &comp.measuredValue, 4);
        objMap.insert({comp.deviceFrom, measureObj});
    }
    tbox->writeServiceObject(objMap);
    printServiceObject(tbox, objName);

    Request req;
    req.reqId = 0x1234;
    std::vector<uint8_t> stimuli1;
    uint16_t ioIndex;

    // set pinTo to LF, at the same time, set pinFrom to LF to do discharge (may be effective for capacitor).
    generateSyncReq(tbox, req); // empty req
    req.reqId++;
    for (component2t_t &comp : resList)
    {
        uint16_t reqMode_u16 = req.reqMode.at(comp.deviceTo);
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        pReqMode->setio_valid = 1;                     // 001-LF
        req.reqMode.at(comp.deviceFrom) = reqMode_u16; // discharge FromPin
        req.reqMode.at(comp.deviceTo) = reqMode_u16;

        stimuli1.clear();
        stimuli1.resize(2, 0);
        ioIndex = (0x01 << (comp.pinTo - 1));
        stimuli1[0] = ioIndex & 0xFF;
        stimuli1[1] = (ioIndex >> 8 & 0xFF);
        req.reqIoPattern.insert({comp.deviceTo, stimuli1});

        if (comp.deviceFrom != comp.deviceTo)
        { // in different devices, generate a new ioIndex
            stimuli1.clear();
            stimuli1.resize(2, 0);
            ioIndex = (0x01 << (comp.pinFrom - 1));
        }
        else
        {
            ioIndex |= (0x01 << (comp.pinFrom - 1)); // in the same device, merge into one ioIndex
        }
        stimuli1[0] = ioIndex & 0xFF;
        stimuli1[1] = (ioIndex >> 8 & 0xFF);
        req.reqIoPattern.insert({comp.deviceFrom, stimuli1}); // clear deviceFrom
    }
    tbox->sendRequest(req);
    waitSync();

    // set pinFrom to HF
    generateSyncReq(tbox, req); // empty req
    req.reqId++;
    for (component2t_t &comp : resList)
    {
        uint16_t reqMode_u16 = req.reqMode.at(comp.deviceFrom); // deviceFrom?
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        if (comp.deviceFrom != comp.deviceTo)
        {                              // in different devices
            pReqMode->setio_valid = 2; // 010-HF
        }
        else
        {                              // in the same device
            pReqMode->setio_valid = 6; // 110-HF incremental, set pinFrom to HF and hold pinTo on LF status.
        }

        req.reqMode.at(comp.deviceFrom) = reqMode_u16;

        stimuli1.clear();
        stimuli1.resize(2, 0);
        uint16_t ioIndex = (0x01 << (comp.pinFrom - 1));
        stimuli1[0] = ioIndex & 0xFF;
        stimuli1[1] = (ioIndex >> 8 & 0xFF);
        req.reqIoPattern.insert({comp.deviceFrom, stimuli1});
    }
    tbox->sendRequest(req);
    waitSync();

    // 从所有pinFrom getMeasure
    generateSyncReq(tbox, req); // empty req
    req.reqId++;
    for (component2t_t &comp : resList)
    {
        uint16_t reqMode_u16 = req.reqMode.at(comp.deviceFrom); // deviceFrom?
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        pReqMode->getmeasure_valid = 1; //
        req.reqMode.at(comp.deviceFrom) = reqMode_u16;
    }
    tbox->sendRequest(req);

    // wait for the last response
    waitSync();

    // 从SDO读取各个测量值
    // std::map<uint16_t, ServiceObject> objMap;
    // objMap.clear();
    tbox->readServiceObject(objMap); // TODO:看起来是这个函数的release没有正确访问到obj //
    for (component2t_t &comp : resList)
    {
        std::cout << "degbug==========comp:" << std::endl;
        if (objMap.count(comp.deviceFrom))
        {
            std::cout << "comp.deviceFrom:" << comp.deviceFrom << std::endl;
            objMap.at(comp.deviceFrom).getItem("value", &comp.measuredValue, 4);
            std::cout << "comp.measuredValue:" << comp.measuredValue << std::endl;
        }
    }
    printServiceObject(tbox, objName);

    // TODO: clear pinTo and pinFrom
    generateSyncReq(tbox, req); // empty req
    req.reqId++;
    for (component2t_t &comp : resList)
    {
        uint16_t reqMode_u16 = req.reqMode.at(comp.deviceTo);
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        pReqMode->setio_valid = 1; // 001-LF
        req.reqMode.at(comp.deviceFrom) = reqMode_u16;
        req.reqMode.at(comp.deviceTo) = reqMode_u16;

        stimuli1.clear();
        stimuli1.resize(2, 0);
        req.reqIoPattern.insert({comp.deviceTo, stimuli1});   // clear IO settings of deviceTo
        req.reqIoPattern.insert({comp.deviceFrom, stimuli1}); // clear IO settings of deviceFrom
    }
    tbox->sendRequest(req);
    waitSync();
}

// 同步设置doutControl，发起release
// tbox中有一个异步release函数，暂不使用 //
void testRelease(Tbox *tbox)
{
    std::cout << "testRelease" << std::endl;

    // 可以选择释放任意个adapter
    Request req;
    req.reqId = 0x4567;
    generateSyncReq(tbox, req); // empty req
    
    // 没有pengding状态的同步请求
    waitSync();

    // req模式，只有dout有效，其他都为0
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        uint16_t reqMode_u16 = req.reqMode.at(id);
        REQ_SYNC_OBJ *pReqMode = (REQ_SYNC_OBJ *)&reqMode_u16;
        pReqMode->setdout_valid = 1;  // only dout is valid
        req.reqMode.at(id) = reqMode_u16;

        std::vector<uint8_t> doutControl;
        doutControl.push_back(0x00);   // byte0: dout7-0(8-1)
        doutControl.push_back(0x40);   // byte1: bit6-release; bit[5:4]-indicator
        req.reqDoutControl.insert({id, doutControl});
    }
    tbox->sendRequest(req);
    waitSync();



    // 等待TC释放Adapter, 判断释放完成的条件：plugin开关针置0，表示Adapter已经脱离出socket //
    // 手工模拟：将DIN7（bit6）的跳线拔下，相当于plug in开关针被拔下 //
    // 开关针是由lock电磁阀锁定的，release的FSM会根据延时参数的设定依次释放leak、level、lock电磁阀 //
    // 释放完成后，DIN7的开关针会因unlock而弹出，表示Adapter已经脱离出socket //
    while(1)
    {
        std::map<uint16_t, std::vector<uint8_t>> dinStatus = tbox->getDinStatus();
        // 检查plug in off(即已经弹出)的Adapter数量 //
        uint16_t offCount = 0;
        // 可以选择需要release的Adapter进行操作和判断 //
        for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
        {
            if ((dinStatus.at(id).data()[1] & 0x40) == 0x00) // din[6]为1表示开关针连接，那么offCount不应计数  
            {
                offCount++;
            }
        }
        if (offCount == tbox->getDeviceCount())
        {
            break;
        }
        Sleep(100); // 100ms
        //std::this_thread::sleep_for(10ms); // 1ms, 1um, 1s...
    }
}


// 有2块user memory
// 1. user header: 16*64bytes
// 2. user data: 64*64bytes
void testUserMemory(Tbox *tbox)
{
#if 1
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++)
    {
        // user data
        std::vector<uint8_t> writeData1, writeData2, readData1, readData2;

        writeData1.resize(64, 0x12);
        tbox->writeUserData(id, writeData1);
        tbox->readUserData(id, 64, readData1);
        if (writeData1 == readData1)
        {
            std::cout << "writeData1 == readData1" << std::endl;
        }
        else
        {
            std::cout << "writeData1 != readData1" << std::endl;
        }

        writeData2.resize(64, 0x55);
        tbox->writeUserData(id, writeData2);
        tbox->readUserData(id, 64, readData2);
        if (writeData2 == readData2)
        {
            std::cout << "writeData2 == readData2" << std::endl;
        }
        else
        {
            std::cout << "writeData2 != readData2" << std::endl;
        }

        // user header
        std::vector<uint8_t> writeHeader1, writeHeader2, readHeader1, readHeader2;

        writeHeader1.resize(64, 0x77);
        tbox->writeUserHeader(id, writeHeader1);
        tbox->readUserHeader(id, 64, readHeader1);
        if (writeHeader1 == readHeader1)
        {
            //std::cout << "writeHeader1:" << writeHeader1.data() << std::endl;
            //std::cout << "readHeader1:" << readHeader1.data() << std::endl;
            std::cout << "writeHeader1 == readHeader1" << std::endl;
        }
        else
        {
            std::cout << "writeHeader1 != readHeader1" << std::endl;
        }

        writeHeader2.resize(64, 0x33);
        tbox->writeUserHeader(id, writeHeader2);
        tbox->readUserHeader(id, 64, readHeader2);
        if (writeHeader2 == readHeader2)
        {
            //std::cout << "writeHeader2:" << writeHeader2.data() << std::endl;
            //std::cout << "readHeader2:" << readHeader2.data() << std::endl;
            std::cout << "writeHeader2 == readHeader2" << std::endl;
        }
        else
        {
            std::cout << "writeHeader2 != readHeader2" << std::endl;
        }

    }
#endif
}

