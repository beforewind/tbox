
// function examples for using tbox
//

#include <iostream>
#include <fstream>
#include <functional>
#include <chrono>
#include <string>

#include "include/tbox.h"

#ifdef TBOX_DLL
#else
#include "ectbox/ectbox.h"
#endif

#include "tbox_helper.h"


///////////////////////////////////////////////////
// get fileName from the full path.
std::string getFileName(const std::string& filePathName)
{
    //std::string path = "D:\\mydoc\\VS-proj\\SMTDetector\\x64\\Release\\0001.bmp";
    size_t index = filePathName.find_last_of("\\");

    //std::string folderPath = filePathName.substr(0, index);
    std::string fileName = filePathName.substr(index + 1, -1);

    size_t index2 = filePathName.find_last_of(".");
    std::string extendName = filePathName.substr(index2 + 1, -1);

    std::cout << "filePathName:\t" << filePathName << std::endl;
    //std::cout << "folderPath:\t" << folderPath << std::endl;
    std::cout << "fileName:\t" << fileName << std::endl;
    std::cout << "extendName:\t" << extendName << std::endl;
    return fileName;
}

// check if file exitsts.
bool isFileExists(const std::string& filePathName) {
    std::ifstream  file(filePathName.c_str());
    return file.good();
}

// read file data, the fileSize info is implied in fileData.size()
void readFile(const std::string& filePathName, std::vector<uint8_t>& fileData)
{
    std::ifstream  file(filePathName.c_str(), std::ofstream::binary);
    if (!file.good()) {
        std::cout << "File " << filePathName << "does not exist!" << std::endl;
        return;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    fileData.clear();
    fileData.resize(fileSize);
    file.read((char*)&fileData.data()[0], fileSize);
    file.close();
}

///////////////////////////////////////////////////

void printArray(std::vector<uint8_t> array)
{
    //for (int i=0;i<array.size();i++) {
    for (int i = array.size()-1; i >= 0; i--) {  // 倒过来输出，byte3_byte2_byte1_byte0
        std::cout << " " << std::hex << static_cast<unsigned short>(array.data()[i]);
    }
    std::cout << std::endl;
}

void printInt(std::vector<uint8_t> array)
{
    assert(array.size()==4);
    uint32_t tmp = 0;
    for (int i=0;i<array.size();i++) {
        tmp = tmp + (static_cast<uint32_t>(array.data()[i]))*(1<<(8*i));
        //std::cout << std::hex << static_cast<unsigned short>(array.data()[i]);
    }
    std::cout << "0x" << std::hex << tmp;
    std::cout << std::endl;
}

// convert 4bytes to a uint32_t
uint32_t convert2Int(std::vector<uint8_t> array)
{
    assert(array.size()==4);
    uint32_t tmp = 0;
    for (int i=0;i<array.size();i++) {
        tmp = tmp + (static_cast<uint32_t>(array.data()[i]))*(1<<(8*i));
    }
    return tmp;
}


void printServiceObject(Tbox* tbox, std::string& objName)
{
    //std::map<uint16_t, ServiceObject> objMap;
    ServiceObject obj;
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
        if (tbox->getServiceObject(id, objName, obj)!=0)  // obj not exists!
            continue;

        // get item name list of obj //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, objName, itemNameList);

        // read obj info //
        for (auto itemName : itemNameList) {
            std::vector<uint8_t> itemData_tmp(4, 0);
            tbox->readServiceItemData(id, objName, itemName, itemData_tmp);
            std::cout << "device:" << id << " obj info: "<< itemName << " value:";
            printInt(itemData_tmp);
        }
    }
}

///////////////////////////////////////////////////

int32_t reqCounter = 0;
int32_t rspCounter = 0;

// callback function
void reqCallback(Request req)
{
    reqCounter++;
    std::cout << "req " << req.reqId << " is accepted" << std::endl;
    for (auto rdout : req.reqDoutControl) {
        std::cout << "  device:" << rdout.first << " dout:";
        printArray(rdout.second);
    }
    for (auto rdata : req.reqIoPattern) {
        std::cout << "  device:" << rdata.first << " stimuli:";
        printArray(rdata.second);
    }
}

void rspCallback(Response rsp)
{
    rspCounter++;
    std::cout << "rsp " << rsp.reqId << " is returned" << std::endl;
    
    for (auto rmode : rsp.rspMode) {
        std::cout << "  device:" << rmode.first << " mode:" << rmode.second << std::endl;
    }

    for (auto rdin : rsp.rspDinStatus) {
        std::cout << "  device:" << rdin.first << " din:";
        printArray(rdin.second);
    }

    for (auto rdata : rsp.rspIoFeedback) {
        std::cout << "  device:" << rdata.first << " feedback:";
        printArray(rdata.second);
    }

    // may read measured value from SDO according to the field of rspMode //

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
void test_foeFlashMM(Tbox* tbox)
{
#if 0  // 
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
            TINFO("objName:{}",obj.first);
        }
    }

    // read
    std::vector<std::string> objNameList;
    tbox->getFlashObjectList(i, type, objList);

    FlashObject obj;
    tbox->readFlashObject(1, "device", obj);
    TINFO("objName:{}",obj.objName);

    tbox->readFlashObject(1, "adapter1", obj);
    TINFO("objName:{}",obj.objName);

#endif
}



///////////////////////////////////////////////////
void initDevice(Tbox* tbox)
{
    std::string deviceObjName = "device";
    std::map<uint16_t, ServiceObject> deviceObjMap;
    ServiceObject deviceObj;

#if 1
    printServiceObject(tbox, deviceObjName);
#else
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
        if (tbox->getServiceObject(id, deviceObjName, deviceObj)!=0)  // obj not exists!
            continue;

        // get item name list of obj //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, deviceObjName, itemNameList);

        // get obj info //
        for (auto itemName : itemNameList) {
            std::vector<uint8_t> itemData_tmp(4, 0);
            tbox->readServiceItemData(id, deviceObjName, itemName, itemData_tmp);
            std::cout << "device:" << id << " device info: "<< itemName << " value:";
            printInt(itemData_tmp);
        }
    }
#endif

    // set device, for example:
    // device0: deviceSN=0x11110001, adapterCount=2, adapterSN0=0xAAAA0001, adapterSN1=0xAAAA0001;
    // device1: deviceSN=0x11110002, adapterCount=2, adapterSN2=0xAAAA0002, adapterSN3=0xAAAA0003;
    uint32_t adapterNum = 0;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0)  // obj not exists!
            continue;

        // get item name list //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, deviceObjName, itemNameList);

        // set deviceSN if "deviceSN" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "deviceSN") > 0) {
            uint32_t deviceSN = 0x11110000 + id;
            deviceObj.setItem("deviceSN", &deviceSN, sizeof(deviceSN));
        }

        // set adapterCount if "adapterCount" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterCount") > 0) {
            uint32_t adapterCount = 0x02;
            deviceObj.setItem("adapterCount", &adapterCount, sizeof(adapterCount));
        }

        // set adapterSN0 if "adapterSN0" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterSN0") > 0) {
            uint32_t adapterSN0 = 0xAAAA0000 + adapterNum;
            adapterNum++;
            deviceObj.setItem("adapterSN0", &adapterSN0, sizeof(adapterSN0));
        }

        // set adapterSN1 if "adapterSN1" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterSN1") > 0) {
            uint32_t adapterSN1 = 0xAAAA0000 + adapterNum;
            adapterNum++;
            deviceObj.setItem("adapterSN1", &adapterSN1, sizeof(adapterSN1));
        }

        deviceObjMap.insert({ id, deviceObj });
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

int readDevice(Tbox* tbox)
{
    std::string deviceObjName = "device";
    std::map<uint16_t, ServiceObject> deviceObjMap;
    ServiceObject deviceObj;
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0) {  // obj not exists!
            continue;
        }
        std::cout << "device" << id << " device info:" << std::endl;

        std::vector<uint8_t> itemData;
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, "device", "deviceSN", itemData);
        uint32_t deviceSN = convert2Int(itemData);
        std::cout << "  deviceSN=0x" << std::hex << deviceSN << std::endl;
        // device is not initialized in FLASH.
        if (deviceSN == 0xFFFFFFFF) {
            return -1;
        }

        itemData.clear();
        itemData.resize(4, 0);
        tbox->readServiceItemData(id, "device", "adapterCount", itemData);
        uint32_t adapterCount = convert2Int(itemData);
        std::cout << "  adapterCount=0x" << std::hex << adapterCount << std::endl;
        // at most 8 adapter per device
        if (adapterCount > 8) {
            return -1;
        }
        
        std::string SNx = "adapterSN";
        for (char i = 0; i < adapterCount; i++) {
            itemData.clear();
            itemData.resize(4, 0);
            std::string adpSN = SNx + (std::to_string(i));
            tbox->readServiceItemData(id, "device", adpSN, itemData);
            uint32_t adapterSN = convert2Int(itemData);
            std::cout << "  adapterSN[" << std::to_string(i) << "]=0x" << std::hex << adapterSN << std::endl;
            if (adapterSN == 0xFFFFFFFF) {
                return -1;
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////////

// hardware info: version, dincount, doutcount, bankcount, tpperbank, iocount
void readHardware(Tbox* tbox)
{
    // 运行前，需要获取所有device的硬件信息，如bank数目、io数目、硬件版本等 //
    // hardware信息是每个mcu表示的device内置的参数，应该为只读类型 //
    std::string hardObjName = "hardware";
    std::map<uint16_t, ServiceObject> hardObjMap;
    ServiceObject hardObj;

#if 0
    printServiceObject(tbox, hardObjName);
#endif

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        if (tbox->getServiceObject(id, hardObjName, hardObj) != 0) {  // obj not exists!
            continue;
        }
        std::cout << "device" << id << " hardware info:" << std::endl;

        std::vector<uint8_t> itemData;

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

void writeParameter(Tbox* tbox)
{
    // 运行前，设置所有device的参数，主要有电流、电压等 //
    // parameter是device内置的参数表，可以读写 //
    // parameter对象包括多个item字段如current, voltage //
    // GUI可以根据将这些item name作为编辑框的label，值用于设置item对应的参数 //
    // 每个item有itemType，表示数据类型如uint32_t, float_t等 //

    std::string paraObjName = "parameter";
    std::map<uint16_t, ServiceObject> paraObjMap;
    ServiceObject paraObj;

#if 0
    // read all
    printServiceObject(tbox, paraObjName);
#endif

    // set parameter
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        if (tbox->getServiceObject(id, paraObjName, paraObj) != 0)  // obj not exists!
            continue;

        // 获取parameter的item name list //
        // 作为参数编辑框的item label //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, paraObjName, itemNameList);

        // set current if "current" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "current") > 0) {
            //float_t current = 10.1;  // mA  浮点值需要转换成整数值 //
            uint32_t current = 10;  // mA
            paraObj.setItem("current", &current, sizeof(current));
            //tbox->writeServiceItemData(id, paraObjName, "current", itemData);
        }

        // set voltage if "voltage" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "voltage") > 0) {
            //float_t voltage = 6.1;  // V  浮点值需要转换成整数值 //
            uint32_t voltage = 0xF00;  // for test DAC
            paraObj.setItem("voltage", &voltage, sizeof(voltage));
            //tbox->writeServiceItemData(id, paraObjName, "current", itemData);
        }

        // set current if "delayus" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "delayus") > 0) {
            uint32_t delayus = 4;  // ms
            paraObj.setItem("delayus", &delayus, sizeof(delayus));
            //tbox->writeServiceItemData(id, paraObjName, "delayus", itemData);
        }

        paraObjMap.insert({ id, paraObj });
    }
    tbox->writeServiceObject(paraObjMap);
    tbox->readServiceObject(paraObjMap);

    // read and check
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
#if 1
        float_t current_tmp;
        paraObjMap.at(id).getItem("current", &current_tmp, sizeof(current_tmp));
        float_t voltage_tmp;
        paraObjMap.at(id).getItem("voltage", &voltage_tmp, sizeof(voltage_tmp));
#endif
        std::vector<uint8_t> itemData_tmp(4, 0);
        tbox->readServiceItemData(id, paraObjName, "current", itemData_tmp);

        std::cout << "device:" << id << " current float value:" << current_tmp << " uint8_t:";
        printInt(itemData_tmp);

        tbox->readServiceItemData(id, paraObjName, "voltage", itemData_tmp);
        std::cout << "device:" << id << " voltage float value:" << voltage_tmp << " uint8_t:";
        printInt(itemData_tmp);
    }
}

///////////////////////////////////////////////////
void readPdoInfo(Tbox* tbox)
{
// 以下四种类型，需要在上层软件中建立di/do/tx/rx pattern与adapter的pin的映射关系 //
// 因此提供了以下示例，用于获取这四类obj的数据结构，并在上层程序中保存和使用 //
#if 1
    // get digital output
    std::string doObjName = "reqDoutPDO";
    std::map<uint16_t, ServiceObject> doObjMap;
    ServiceObject doObj;
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
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
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
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
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
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
    for (uint16_t id = 1; id<= tbox->getDeviceCount(); id++) {
        tbox->getServiceObject(id, rxObjName, rxObj);
        rxObjMap.insert({id, rxObj});
        std::cout << " device:" << id << " obj name : " << rxObj.objName << " size : " << rxObj.objSize << std::endl;
    }
#endif
}


///////////////////////////////////////////////////

std::map<int32_t, Request> reqMap;
std::map<int32_t, Response> rspMap;

Request generateSetReqest(Tbox* tbox)
{
    int32_t reqId = reqMap.size();
    Request req;

    std::vector<uint8_t> mainS_dout;  // mainSwitch+dout
    mainS_dout.push_back(0x0F);  // dout4,3,2,1
    mainS_dout.push_back(0x00);
    //mainS_dout.push_back(0x09);  // main switch // 0x09:1001-COMG/COML/LOOP/HF

    std::vector<uint8_t> ioPattern1, ioPattern2;
    ioPattern1.push_back(0x04);  //0x05:IO2,IO0;  0x39 0011_1001
    ioPattern1.push_back(0x00);  //0xA0:IO15, IO13, 0xC6 1100_0110

    ioPattern2.push_back(0x00);  //0x05:IO2,IO0;  0x39 0011_1001
    ioPattern2.push_back(0x00);  //0xA0:IO15, IO13, 0xC6 1100_0110
    REQ_SYNC_OBJ reqMode;
    uint16_t reqMode_u16;

    reqMode.setdout_valid = 1;
    reqMode.setio_valid = 1;  // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
    reqMode.getio_valid = 0;
    reqMode.getmeasure_valid = 0;
    reqMode.getext_valid = 0;
    reqMode.reserved = 0;
    memcpy(&reqMode_u16, (uint16_t*)&reqMode, 2);
#if 0
    for (uint16_t id=1; id<=tbox->getDeviceCount();id++) {
        req.reqDoutControl.insert({id, mainS_dout});       // digital output //
        req.reqIoPattern.insert({id, ioPattern }); // IO pattern //
        req.reqMode.insert({id, reqMode_u16 });
    }
#else
    req.reqDoutControl.insert({1, mainS_dout});       // digital output //
    req.reqIoPattern.insert({1, ioPattern1 }); // IO pattern //
    req.reqMode.insert({1, reqMode_u16 });

    req.reqDoutControl.insert({2, mainS_dout});       // digital output //
    req.reqIoPattern.insert({2, ioPattern2 }); // IO pattern //
    req.reqMode.insert({2, reqMode_u16 });
#endif
    req.reqId = reqMap.size();
    reqMap.insert({req.reqId, req});
    return req;
}

Request generateGetRequest(Tbox* tbox)
{
    int32_t reqId = reqMap.size();
    Request req;

    std::vector<uint8_t> mainS_dout;  // mainSwitch+dout
    mainS_dout.push_back(0x0F);  // dout4,3,2,1
    mainS_dout.push_back(0x00);
    //mainS_dout.push_back(0x09);  // main switch // 0x09:1001-COMG/COML/LOOP/HF

    std::vector<uint8_t> ioPattern1, ioPattern2;
    ioPattern1.push_back(0x05);  //0x05:IO2,IO0;  0x39 0011_1001
    ioPattern1.push_back(0xA0);  //0xA0:IO15, IO13, 0xC6 1100_0110
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
    for (uint16_t id=1; id<=tbox->getDeviceCount();id++) {
        req.reqDoutControl.insert({id, mainS_dout});       // digital output //
        //req.reqIoPattern.insert({id, ioPattern }); // IO pattern //
        req.reqMode.insert({id, reqMode_u16 });
    }

    req.reqId = reqMap.size();
    reqMap.insert({req.reqId, req});
    return req;
}

///////////////////////////////////////////////////

uint32_t getDeviceObjectItem(Tbox* tbox, uint16_t deviceId, const std::string &objName, const std::string &itemName)
{
    ServiceObject serviceObj;
    if (tbox->getServiceObject(deviceId, objName, serviceObj) != 0) {  // obj not exists!
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

int32_t getDeviceObjectSize(Tbox* tbox, uint16_t deviceId, const std::string& objName)
{
    ServiceObject serviceObj;
    if (tbox->getServiceObject(deviceId, objName, serviceObj) != 0) {  // obj not exists!
        std::cout << "";
        return 0;
    }

    return serviceObj.objSize;
}


void readDeviceList(Tbox* tbox, std::map<uint32_t, device_t>& deviceMap)
{
    deviceMap.clear();

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        device_t dev;
        dev.id = id;
        
        dev.deviceSN = getDeviceObjectItem(tbox, id, "device", "deviceSN");
        dev.adapterCount = getDeviceObjectItem(tbox, id, "device", "adapterCount");
        if (dev.adapterCount <= 0 || dev.adapterCount > 8) {
            dev.adapterCount = 0;
            continue;
        }
        for (char i = 0; i < dev.adapterCount; i++) {
            dev.adapterSN[i] = getDeviceObjectItem(tbox, id, "device", ("adapterSN"+(std::to_string(i))));
        }

        dev.version = getDeviceObjectItem(tbox, id, "hardware", "version");
        dev.dinCount = getDeviceObjectItem(tbox, id, "hardware", "dinCount");
        dev.doutCount = getDeviceObjectItem(tbox, id, "hardware", "doutCount");
        dev.bankCount = getDeviceObjectItem(tbox, id, "hardware", "bankCount");
        dev.ioCount = getDeviceObjectItem(tbox, id, "hardware", "ioCount");

        dev.current = getDeviceObjectItem(tbox, id, "parameter", "current");
        dev.voltage = getDeviceObjectItem(tbox, id, "parameter", "voltage");
        dev.delayus = getDeviceObjectItem(tbox, id, "parameter", "delayus");
        dev.rsample = getDeviceObjectItem(tbox, id, "parameter", "rsample");

        dev.doSize = getDeviceObjectSize(tbox, id, "reqDoutPDO");
        dev.diSize = getDeviceObjectSize(tbox, id, "respDinPDO");
        dev.txStimuliSize = getDeviceObjectSize(tbox, id, "reqDataPDO");
        dev.rxFeedbackSize = getDeviceObjectSize(tbox, id, "respDataPDO");

        deviceMap.insert({dev.deviceSN, dev});
    }
}

void saveDeviceList(Tbox* tbox, std::map<uint32_t, device_t>& deviceMap)
{
    std::string deviceObjName = "device";
    std::map<uint16_t, ServiceObject> deviceObjMap;
    ServiceObject deviceObj;

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {

        if (tbox->getServiceObject(id, deviceObjName, deviceObj) != 0)  // obj not exists!
            continue;

        // get deviceSN
        uint32_t deviceSN = getDeviceObjectItem(tbox, id, "device", "deviceSN");

        if (deviceMap.count(deviceSN) == 0)
            continue;

        auto& dev = deviceMap[deviceSN];

        // get item name list //
        std::vector<std::string> itemNameList;
        tbox->getServiceItemNameList(id, deviceObjName, itemNameList);

        // set deviceSN if "deviceSN" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "deviceSN") > 0) {
            uint32_t deviceSN = dev.deviceSN;
            deviceObj.setItem("deviceSN", &deviceSN, sizeof(deviceSN));
        }

        // set adapterCount if "adapterCount" item exists
        if (std::count(itemNameList.begin(), itemNameList.end(), "adapterCount") > 0) {
            uint32_t adapterCount = dev.adapterCount;
            deviceObj.setItem("adapterCount", &adapterCount, sizeof(adapterCount));
        }

        for (char j = 0; j < dev.adapterCount; j++) {
            std::string adapterSNx = "adapterSN" + std::to_string(j);
            if (std::count(itemNameList.begin(), itemNameList.end(), adapterSNx) > 0) {
                deviceObj.setItem(adapterSNx, &dev.adapterSN[j], sizeof(dev.adapterSN[j]));
            }
        }

        deviceObjMap.insert({ id, deviceObj });
    }
    tbox->writeServiceObject(deviceObjMap);
    // read back
    tbox->readServiceObject(deviceObjMap);
}


///////////////////////////////////////////////////

void generateAdapterExample(std::map<uint32_t, adapter_t>& adapterMap, std::map<uint32_t, device_t>& deviceMap)
{
    adapter_t adp[4];
    
    // init adapters
    adp[0].adapterSN = 0x66660000;  // will store in device, as the key to find in database
    adp[0].diCount = 1;  // store in database
    adp[0].doCount = 1;  // store in database
    adp[0].ioCount = 4;  // store in database

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
    for (auto& d : deviceMap) {
        d.second.adapterCount = 0;
    }

    // assign device
    for (uint16_t i = 0; i < 4; i++) {
        uint16_t deviceId = (i % deviceMap.size()) + 1;  // distribute adapters to devices
        uint32_t deviceSN = 0;
        for (auto& d : deviceMap) {
            if (d.second.id == deviceId) {
                deviceSN = d.second.deviceSN;
                break;
            }
        }
        if (deviceMap.count(deviceSN) == 0)  // does not exists
            continue;
        auto& dev = deviceMap[deviceSN];
        dev.adapterSN[dev.adapterCount] = adp[i].adapterSN;
        dev.adapterCount++;
        adp[i].deviceSN = dev.deviceSN;

        adapterMap.insert({adp[i].adapterSN, adp[i]});
    }

    //
    // save device outside
}

void generateAdapterMappingExample(std::map<uint32_t, adapter_t>& adapterMap)
{
    // multiple adapters are mounted on a device, start from io/di/do offset
    std::map<uint32_t, uint32_t> ioOffsetOfDevice;
    std::map<uint32_t, uint32_t> diOffsetOfDevice;
    std::map<uint32_t, uint32_t> doOffsetOfDevice;

    for (auto& a : adapterMap) {
        // init adapter


        // io mapping from adapter.io to device.io
        // should be edited by GUI
        a.second.ioCount = 4;
        uint32_t ioOffset;
        if (ioOffsetOfDevice.count(a.second.deviceSN)) {
            ioOffset = ioOffsetOfDevice.at(a.second.deviceSN);
        }
        else {
            ioOffset = 0;
        }
        // update
        ioOffsetOfDevice[a.second.deviceSN] = ioOffset + a.second.ioCount;
        // for example
        a.second.ioMap.insert({ 0,ioOffset + 0 });  // adapter.io -> device.io
        a.second.ioMap.insert({ 1,ioOffset + 1 });
        a.second.ioMap.insert({ 2,ioOffset + 2 });
        a.second.ioMap.insert({ 3,ioOffset + 3 });


        // di mapping from adapter.di to device.di
        a.second.diCount = 2;
        uint32_t diOffset;
        if (diOffsetOfDevice.count(a.second.deviceSN)) {
            diOffset = diOffsetOfDevice.at(a.second.deviceSN);
        }
        else {
            diOffset = 0;
        }
        // update
        diOffsetOfDevice[a.second.deviceSN] = diOffset + a.second.diCount;
        a.second.diMap.insert({ 0,diOffset+0 });
        a.second.diMap.insert({ 1,diOffset+1 });


        // do mapping from adapter.do to device.do
        a.second.doCount = 2;
        uint32_t doOffset;
        if (doOffsetOfDevice.count(a.second.deviceSN)) {
            doOffset = doOffsetOfDevice.at(a.second.deviceSN);
        }
        else {
            doOffset = 0;
        }
        // update
        doOffsetOfDevice[a.second.deviceSN] = doOffset + a.second.diCount;
        a.second.doMap.insert({ 0,doOffset + 0 });
        a.second.doMap.insert({ 1,doOffset + 1 });
    }
}


void generateWireExample(std::map<uint32_t, adapter_t>& adapterMap, std::vector<connection_t>& wireList)
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
void generatePatternExample(std::map<uint32_t, device_t>& deviceMap, std::map<uint16_t, std::vector<uint8_t>>& doutPattern, std::map<uint16_t, std::vector<uint8_t>>& stimuliPattern)
{
    std::vector<uint8_t> stimuli1, stimuli2;
    stimuli1.resize(2,0);
    stimuli2.resize(2,0);

    stimuli1[0] = 0x00;  // 0101_0000:IO5,IO7-LF,  ;0101, IO2,IO0--LF, IO3,IO1--HF
    stimuli1[1] = 0x80;

    stimuli2[0] = 0x05;  // 0101_0000:IO5,IO7-LF,  ;0101, IO2,IO0--LF, IO3,IO1--HF
    stimuli2[1] = 0x00;

    for (auto& d : deviceMap) {
        auto& dev = d.second;
        uint16_t deviceId = dev.id;
        std::vector<uint8_t> dout;
        //stimuli1.resize(dev.txStimuliSize, 0);
        //stimuli2.resize(dev.txStimuliSize, 0);
        dout.resize(2, 0);  // fixed 2 bytes
#if 1
        //stimuli[0] = 0x04;  // 0101_0000:IO5,IO7-LF,  ;0101, IO2,IO0--LF, IO3,IO1--HF
        //stimuli[1] = 0x00;
#else
        for (uint32_t i = 0; i < dev.txStimuliSize; i++) {  // pattern的长度取决于txStimuliSize表示的byte个数 //
            stimuli[i] = 0x05;  // 实际应该来自算法处理adapter的io生成的激励，0-off，1-激励 //
        }
#endif
        if (deviceId % 2) {
            stimuliPattern.insert({deviceId, stimuli1});
        } else {
            stimuliPattern.insert({deviceId, stimuli2});
        }


        for (uint32_t i = 0; i < dev.doSize; i++) {
            dout[i] = 0x0F;  // 实际来自控制adapter的数字输出，0-off，1-开启 //
        }
        doutPattern.insert({deviceId, dout});
    }
}

Request generateSetReqestExample(Tbox* tbox, std::map<uint16_t, std::vector<uint8_t>>& doutPattern, std::map<uint16_t, std::vector<uint8_t>>& stimuliPattern)
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
    mainS_dout.push_back(0x09);  // main switch // 0x09:1001-COMG/COML/LOOP/HF
#else
    for (auto& dout : doutPattern) {
        req.reqDoutControl.insert({dout.first, dout.second});
        doutModePattern.insert({dout.first, 1});
    }
#endif

#if 0
    std::vector<uint8_t> ioPattern;
    ioPattern.push_back(0x05);  //0x05:IO2,IO0;  0x39 0011_1001
    ioPattern.push_back(0xA0);  //0xA0:IO15, IO13, 0xC6 1100_0110
#else
    for (auto& io : stimuliPattern) {
        req.reqIoPattern.insert({ io.first, io.second });
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
    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        uint16_t reqMode_u16 = 0;
        REQ_SYNC_OBJ* pReqMode = (REQ_SYNC_OBJ*)&reqMode_u16;
        if (doutModePattern.count(id) != 0) {
            pReqMode->setdout_valid = 1;
        }
        if (setioModePattern.count(id) != 0) {
            pReqMode->setio_valid = 1;
        }
        req.reqMode.insert({ id, reqMode_u16 });
    }
#endif

    req.reqId = reqMap.size();
    reqMap.insert({ req.reqId, req });
    return req;
}

Request generateGetRequestExample(Tbox* tbox, std::map<uint16_t, std::vector<uint8_t>>& doutPattern)
{
    int32_t reqId = reqMap.size();
    Request req;

    std::map<uint16_t, uint16_t> doutModePattern;
    //std::map<uint16_t, uint16_t> setioModePattern;

    for (auto& dout : doutPattern) {
        req.reqDoutControl.insert({ dout.first, dout.second });
        doutModePattern.insert({ dout.first, 1 });
    }

    for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
        uint16_t reqMode_u16 = 0;
        REQ_SYNC_OBJ* pReqMode = (REQ_SYNC_OBJ*)&reqMode_u16;
        if (doutModePattern.count(id) != 0) {
            pReqMode->setdout_valid = 1;
        }
        pReqMode->getio_valid = 2;  // 2;  // 1-设置io为LF，2-设置io为HF，3-设置io为连接到LF和HF //
        req.reqMode.insert({ id, reqMode_u16 });
    }

    req.reqId = reqMap.size();
    reqMap.insert({ req.reqId, req });
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
