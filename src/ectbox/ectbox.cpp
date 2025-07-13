
#include <ectbox/ectbox.h>

// namespace tstudio
//{

int32_t EcTbox::connect(const std::string &nicName)
{
    TDEBUG("connect to NIC: {}", nicName);

    std::map<std::string, std::string> nicNameListMap;
    nicNameListMap = EctBus::findAvailableNics();
    if (nicNameListMap.count(nicName) == 0)
    {
        TERROR("There is no NIC with name: {}, please check the hardware!", nicName);
        return -1;
    }

    std::string busName = nicNameListMap[nicName];
    // TDEBUG("key:nicName:{}", nicName);
    // TDEBUG("value:busName:{}", busName);

    // TODO: 连接状态下的重新连接处理, 是否需要参数作为强制连接的使能信号？ //
    // 是否需要先stop进程？ //
    // if (connectStatus()) {
    disconnect();
    //}

    if (!mBus->openBus(busName.data())) // \Device\NPF_{35E61F5E-2469-4854-A3C5-B185FAC22FC4}
    {                                   // if (!mBus->openBus("\\Device\\NPF_{35E61F5E-2469-4854-A3C5-B185FAC22FC4}"))
        TERROR("Failed connect to NIC : {}, please check the logfile!", nicName);
        return -1;
    }
    m_nicName = busName;

#if 1
    // 下面代码移出 //
    if (!mBus->waitUntilAllSlavesReachedOP())
    {
        TERROR("Failed to wait device state, please check the logfile!");
        return -1;
    }

    // mBus->createPDODescription(); // start from 1, to slavecount

    mBus->testwd();

    if (mBus->createSlaves() < 0)
    {
        return -1;
    }

    // create serviceObject
    generateObjectDict();

#endif
    return 0;
}

int32_t EcTbox::disconnect()
{
    TDEBUG("disconnect from NIC: {}", m_nicName);

    if (m_nicName == "")
    {
        TERROR("nicName is empty!");
        return 0;
    }
    m_nicName = "";

    // TODO: 需要先停止run线程，再断开网络连接   //
    // stop run thread
    m_runThreadRunning = false;
    // 这是要把线程再 加入执行吗？
    if (m_runThread.joinable())
        m_runThread.join();
    mBus->closeBus();

    TDEBUG("disconnect successfully");
    return 0;
}

int32_t EcTbox::getDeviceCount()
{
    return mBus->getNumberOfSlaves();
}

int32_t EcTbox::updateFirmware(uint16_t deviceId, const std::string &fileName, std::vector<uint8_t> &fileData)
{
    mBus->updateSlave(deviceId, fileName.data(), &fileData.data()[0], fileData.size());
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// scan hardware config
// 分开deviceConfig/adapterList //
int32_t EcTbox::scanAdapters(std::map<uint16_t, std::vector<uint8_t>> &configDataList, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>> &adapterListMap)
{
    // foreach device in bus, readDevice
    for (int i = 1; i <= getDeviceCount(); i++) // from 1-n
    {
        // read Flash Objects to initialize device and adapters
        std::vector<uint8_t> configData;
        std::map<std::string, std::vector<uint8_t>> adapterList;
        std::vector<FlashObject> objList;
        getFlashObjectList(i, 1, objList);

        for (auto obj : objList)
        {
            readFlashObject(i, obj);
            if (obj.objType == CONFIG)
            {
                configData = obj.objData;
            }
            else if (obj.objType == MODULE)
            {
                adapterList.insert({obj.objName, obj.objData});
            }
        }
        configDataList.insert({i, configData});
        adapterListMap.insert({i, adapterList});
    }
    return 0;
}

int32_t EcTbox::getFlashObjectList(const uint16_t deviceId, const uint32_t objType, std::vector<FlashObject> &objList)
{
    // std::vector<FlashObject> objList;
    objList.clear();
    std::map<std::string, std::pair<uint32_t, int32_t>> objMap; // (name,(type, size))
    objMap = mBus->readFlashObjectMap(deviceId, objType);

    for (auto obj : objMap)
    {
        FlashObject flashObj; // (name,(type, size))
        flashObj.objName = obj.first;
        flashObj.objType = obj.second.first;
        flashObj.objSize = obj.second.second;
        objList.push_back(flashObj);
    }
    // return objList;
    return 0;
}

int32_t EcTbox::writeFlashObject(const uint16_t deviceId, FlashObject &obj)
{
    int32_t size = mBus->foeWriteData(deviceId, obj.objName, obj.objType, obj.objData);
    return size;
}

int32_t EcTbox::readFlashObject(const uint16_t deviceId, FlashObject &obj)
{
    int32_t size = mBus->foeReadData(deviceId, obj.objName, obj.objType, obj.objData);
    if (obj.objSize != obj.objData.size())
    {
        // TODO:
    }
    return size;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
int32_t EcTbox::getServiceObjectNameList(const uint16_t deviceId, std::vector<std::string> &objNameList)
{
    objNameList.clear();
    if (m_serviceObjDict.count(deviceId) > 0)
    {
        for (auto obj : m_serviceObjDict[deviceId])
        {
            objNameList.push_back(obj.first);
        }
    }
    return 0;
}

int32_t EcTbox::getServiceItemNameList(const uint16_t deviceId, const std::string &objName, std::vector<std::string> &itemNameList)
{
    itemNameList.clear();
    if (m_serviceObjDict.count(deviceId) > 0)
    {
        if (m_serviceObjDict[deviceId].count(objName) > 0)
        {
            for (auto item : m_serviceObjDict[deviceId][objName].itemMap)
            {
                itemNameList.push_back(item.first);
            }
            return 0;
        }
    }
    return -1;
}

int32_t EcTbox::getServiceObjectMap(const std::string &objName, std::map<uint16_t, ServiceObject> &serviceObjMap)
{
    serviceObjMap.clear();
    for (auto dev : m_serviceObjDict)
    {
        uint16_t deviceId = dev.first;
        if (dev.second.count(objName) > 0)
        {
            serviceObjMap.insert({deviceId, dev.second[objName]});
        }
    }
    return 0;
}

int32_t EcTbox::getServiceObject(const uint16_t deviceId, const std::string &objName, ServiceObject &serviceObj)
{
    if (m_serviceObjDict.count(deviceId) > 0)
    {
        if (m_serviceObjDict[deviceId].count(objName) > 0)
        {
            serviceObj = m_serviceObjDict[deviceId][objName];
            return 0;
        }
    }
    return -1;
}

int32_t EcTbox::getServiceItem(const uint16_t deviceId, const std::string &objName, const std::string &itemName, ServiceItem &serviceItem)
{
    if (m_serviceObjDict.count(deviceId) > 0)
    {
        if (m_serviceObjDict[deviceId].count(objName) > 0)
        {
            if (m_serviceObjDict[deviceId][objName].itemMap.count(itemName) > 0)
            {
                serviceItem = m_serviceObjDict[deviceId][objName].itemMap[itemName];
                return 0;
            }
        }
    }
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

int32_t EcTbox::readServiceObjectData(const uint16_t deviceId, const std::string &objName, std::vector<uint8_t> &objData)
{
    uint16_t index = getObjectIndex(deviceId, objName);
    // 可能不存在index/subindex //
    if (index == 0)
        return -1;
    if (mBus->sendSdoRead(deviceId, index, 1, true, objData))
    {
        return 0;
    }
    return -1;
}

int32_t EcTbox::writeServiceObjectData(const uint16_t deviceId, const std::string &objName, std::vector<uint8_t> &objData)
{
    uint16_t index = getObjectIndex(deviceId, objName);
    // 可能不存在index/subindex //
    if (index == 0)
        return -1;
    if (mBus->sendSdoWrite(deviceId, index, 1, true, objData))
    {
        return 0;
    }
    return -1;
}

int32_t EcTbox::readServiceItemData(const uint16_t deviceId, const std::string &objName, const std::string &itemName, std::vector<uint8_t> &itemData)
{
    uint16_t index = getObjectIndex(deviceId, objName);
    // 可能不存在index/subindex //
    if (index == 0)
        return -1;
    uint8_t subindex = getObjectItemSubindex(deviceId, objName, itemName);
    if (subindex == 0)
        return -1;
    if (mBus->sendSdoRead(deviceId, index, subindex, false, itemData))
    {
        return 0;
    }
    return -1;
}

int32_t EcTbox::writeServiceItemData(const uint16_t deviceId, const std::string &objName, const std::string &itemName, std::vector<uint8_t> &itemData)
{
    uint16_t index = getObjectIndex(deviceId, objName);
    // 可能不存在index/subindex //
    if (index == 0)
        return -1;
    uint8_t subindex = getObjectItemSubindex(deviceId, objName, itemName);
    if (subindex == 0)
        return -1;
    if (mBus->sendSdoWrite(deviceId, index, subindex, false, itemData))
    {
        return 0;
    }
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// 获取对象的子项 //
int32_t EcTbox::readServiceItem(const std::string &objName, std::map<uint16_t, ServiceItem> &itemMap)
{
    for (auto dev : itemMap)
    {
        uint16_t deviceId = dev.first;
        ServiceItem item = dev.second;
        uint16_t index = getObjectIndex(deviceId, objName);
        // 可能不存在index/subindex //
        if (index == 0)
            return -1;
        mBus->sendSdoRead(deviceId, index, item.itemIndex, false, item.itemData);
    }
    return 0;
}

// 设置对象的子项 //
int32_t EcTbox::writeServiceItem(const std::string &objName, std::map<uint16_t, ServiceItem> &itemMap)
{
    for (auto dev : itemMap)
    {
        uint16_t deviceId = dev.first;
        ServiceItem item = dev.second;
        uint16_t index = getObjectIndex(deviceId, objName);
        // 可能不存在index/subindex //
        if (index == 0)
            return -1;
        mBus->sendSdoWrite(deviceId, index, item.itemIndex, false, item.itemData);
    }
    return 0;
}

// 获取某个类型的对象及其子项 //
int32_t EcTbox::readServiceObject(std::map<uint16_t, ServiceObject> &objMap)
{
    for (auto &dev : objMap)
    {
        uint16_t deviceId = dev.first;
        ServiceObject &obj = dev.second;
        //TDEBUG("readServiceObject: deviceId:{}, index:{} ", deviceId, obj.objIndex);
        for (auto &subObj : obj.itemMap)
        {
            // std::string itemName = subObj.first;
            ServiceItem &item = subObj.second;
            //TDEBUG("item.itemData.size:{}", item.itemData.size());
            mBus->sendSdoRead(deviceId, obj.objIndex, item.itemIndex, false, item.itemData);
            // obj.itemMap[subObj.first] = item;
            uint32_t tmp = item.itemData[0] + (item.itemData[1] << 8) + (item.itemData[2] << 16) + (item.itemData[3] << 24);
            //TDEBUG("--itemIndex:{}, itemData:{}", item.itemIndex, tmp);
            // TDEBUG("--itemIndex:{}, itemData:{},{},{},{},{}", item.itemIndex, item.itemData, item.itemData[0], item.itemData[1], item.itemData[2], item.itemData[3]);
        }
        // objMap[deviceId] = obj;
    }
    return 0;
}

// 设置对象及其子项 //
int32_t EcTbox::writeServiceObject(std::map<uint16_t, ServiceObject> &objMap)
{
    for (auto &dev : objMap)
    {
        uint16_t deviceId = dev.first;
        ServiceObject &obj = dev.second;
        //TDEBUG("writeServiceObject: deviceId:{}, index:{} ", deviceId, obj.objIndex);
        for (auto &subObj : obj.itemMap)
        {
            ServiceItem &item = subObj.second;
            mBus->sendSdoWrite(deviceId, obj.objIndex, item.itemIndex, false, item.itemData);
            // uint32_t tmp = item.itemData[0] + (item.itemData[1] << 8) + (item.itemData[2] << 16) + (item.itemData[3] << 24);
            // TDEBUG("--itemIndex:{0:x}, itemData:{}", item.itemIndex, tmp);
            // TDEBUG("--itemIndex:{0:x}, itemData:{},{},{},{},{}", item.itemIndex, item.itemData, item.itemData[0], item.itemData[1], item.itemData[2], item.itemData[3]);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
#define GENERAL_OBJECT_INDEX             (0x8000)  // general object of firmware
#define COMMAND_OBJECT_INDEX             (0x8040)  // measure object subindex = 1 
//#define COMMAND_OBJECT_INDEX             (0x9000)  // 用这个obj传递信息给general操作，如pageIndex等

#define USER_MEMORY_PAGE_SIZE            (64)      // 64 bytes per page

#define EEPROM_OBJECT_START_ADDR         (0)  // (32*USER_MEMORY_PAGE_SIZE)
// #define EEPROM_HARDWARE_OBJ_ADDR         (EEPROM_OBJECT_START_ADDR + 0*EEPROM_PAGE_SIZE)  // 最大4page, 256bytes
// #define EEPROM_PARAMETER_OBJ_ADDR        (EEPROM_OBJECT_START_ADDR + 4*EEPROM_PAGE_SIZE)
// #define EEPROM_DEVICE_OBJ_ADDR           (EEPROM_OBJECT_START_ADDR + 8*EEPROM_PAGE_SIZE)
// #define EEPROM_LOCKCOUNTER_ADDR          (EEPROM_OBJECT_START_ADDR + 12*EEPROM_PAGE_SIZE)
#define EEPROM_USER_HEADR_ADDR           (EEPROM_OBJECT_START_ADDR + 16*USER_MEMORY_PAGE_SIZE)  // 最大16page, 1024bytes
#define EEPROM_USER_DATA_ADDR            (EEPROM_OBJECT_START_ADDR + 32*USER_MEMORY_PAGE_SIZE)  // 最大64page, 4096bytes

// 仅支持page_size的整数倍访问 //
int32_t EcTbox::readUserMemory(uint16_t deviceId, uint16_t memAddr, uint16_t byteSize, std::vector<uint8_t> &data)
{
    if (byteSize % USER_MEMORY_PAGE_SIZE)
    {
        TERROR("byteSize should be mulitple of USER_MEMORY_PAGE_SIZE(64bytes)");
        return -1;
    }

    data.clear();

    for (int i = 0; i < byteSize; i += USER_MEMORY_PAGE_SIZE)
    {
        int32_t this_size = std::min(USER_MEMORY_PAGE_SIZE, byteSize - i);
        std::vector<uint8_t> itemData;
        itemData.resize(this_size, 0);
        //uint8_t pageIndex = (memAddr+i)/USER_MEMORY_PAGE_SIZE;

        // 用command object设置访问空间地址 //
        std::vector<uint8_t> pageIndexData;
        uint32_t addr = memAddr+i;
        pageIndexData.resize(4, 0);
        pageIndexData.data()[0] = addr & 0xff;
        pageIndexData.data()[1] = (addr >> 8) & 0xff;
        pageIndexData.data()[2] = (addr >> 16) & 0xff;
        pageIndexData.data()[3] = (addr >> 24) & 0xff;

        // 将地址写到command object的第0个item中 //
        bool ret = mBus->sendSdoWrite(deviceId, COMMAND_OBJECT_INDEX, 1, false, pageIndexData);
        if (!ret)
        {
            TERROR("readUserMemory addr failed");
            return -1;
        }

        // 从general object读取数据 //
        // bool ret2 = mBus->sendSdoRead(deviceId, GENERAL_OBJECT_INDEX, 1, (this_size< USER_MEMORY_PAGE_SIZE) ? false : true, itemData);
        bool ret2 = mBus->sendSdoRead(deviceId, GENERAL_OBJECT_INDEX, 1, true, itemData);
        if (ret2)
        {
            data.insert(data.end(), itemData.begin(), itemData.end());
        }
        else
        {
            TERROR("readUserMemory data failed");
            return -1;
        }
    }
    return data.size();
}

// 仅支持page_size的整数倍访问 //
int32_t EcTbox::writeUserMemory(uint16_t deviceId, uint16_t memAddr, uint16_t byteSize, std::vector<uint8_t> &data)
{
    if (byteSize % USER_MEMORY_PAGE_SIZE)
    {
        TERROR("byteSize should be mulitple of USER_MEMORY_PAGE_SIZE(64bytes)");
        return -1;
    }

    int32_t write_size = 0;
    for (int i = 0; i < byteSize; i += USER_MEMORY_PAGE_SIZE)
    {
        int32_t this_size = std::min(USER_MEMORY_PAGE_SIZE, byteSize - i);
        std::vector<uint8_t> itemData;
        itemData.clear();
        itemData.insert(itemData.begin(), data.begin() + i, data.begin() + i + this_size);
        //uint8_t pageIndex = (memAddr+i)/USER_MEMORY_PAGE_SIZE;
        
        // 用command object设置访问空间地址 //
        std::vector<uint8_t> pageIndexData;
        uint32_t addr = memAddr+i;
        pageIndexData.resize(4, 0);
        pageIndexData.data()[0] = addr & 0xff;
        pageIndexData.data()[1] = (addr >> 8) & 0xff;
        pageIndexData.data()[2] = (addr >> 16) & 0xff;
        pageIndexData.data()[3] = (addr >> 24) & 0xff;

        // 将地址写到command object的第0个item中 //
        bool ret = mBus->sendSdoWrite(deviceId, COMMAND_OBJECT_INDEX, 1, false, pageIndexData);
        if (!ret)
        {
            TERROR("writeUserMemory addr failed");
            return -1;
        }

        // 用general object写入数据 //
        bool ret2 = mBus->sendSdoWrite(deviceId, GENERAL_OBJECT_INDEX, 1, (this_size < USER_MEMORY_PAGE_SIZE) ? false : true, itemData);
        // bool ret2 = mBus->sendSdoWrite(deviceId, GENERAL_OBJECT_INDEX, 1, true, itemData);
        if (!ret2)
        {
            TERROR("writeUserMemory data failed");
            return -1;
        }
        else
        {
            write_size += this_size;
        }
    }
    return write_size;
}

int32_t EcTbox::readUserHeader(uint16_t deviceId, uint16_t byteSize, std::vector<uint8_t> &data)
{
    if (byteSize > 16*USER_MEMORY_PAGE_SIZE)
    {
        TERROR("readUserHeader failed, data size too large");
        return -1;
    }
    return readUserMemory(deviceId, EEPROM_USER_HEADR_ADDR, byteSize, data);
}

int32_t EcTbox::writeUserHeader(uint16_t deviceId, std::vector<uint8_t> &data)
{
    if (data.size() > 16*USER_MEMORY_PAGE_SIZE)
    {
        TERROR("writeUserHeader failed, data size too large");
        return -1;
    }
    return writeUserMemory(deviceId, EEPROM_USER_HEADR_ADDR, data.size(), data);
}

int32_t EcTbox::readUserData(uint16_t deviceId, uint16_t byteSize, std::vector<uint8_t> &data)
{
    if (byteSize > 64*USER_MEMORY_PAGE_SIZE)
    {
        TERROR("readUserData failed, data size too large");
        return -1;
    }
    return readUserMemory(deviceId, EEPROM_USER_DATA_ADDR, byteSize, data);
}

int32_t EcTbox::writeUserData(uint16_t deviceId, std::vector<uint8_t> &data)
{
    if (data.size() > 64*USER_MEMORY_PAGE_SIZE)
    {
        TERROR("writeUserData failed, data size too large");
        return -1;
    }
    return writeUserMemory(deviceId, EEPROM_USER_DATA_ADDR, data.size(), data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
    // objName, (slaveId, (itemName, itemData))
    int32_t EcTbox::readServiceObject(const std::string& objName, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>>& objDataMap)
    {
        for(auto dev : objDataMap) {
            uint16_t deviceId = dev.first;
            std::map<std::string, std::vector<uint8_t>> obj = dev.second;
            uint16_t index = getObjectIndex(deviceId, objName);
            // 可能不存在index/subindex //
            if (index==0)
                continue;  // TODO: check valid
            for (auto item : obj) {
                std::string itemName = item.first;
                std::vector<uint8_t> itemData = item.second;
                itemData.clear();
                itemData.resize(4,0);  // TODO: default 4bytes
                uint8_t subindex = getObjectItemSubindex(deviceId, objName, itemName);
                if (subindex==0)
                    continue;  // TODO: check valid
                mBus->sendSdoRead(deviceId, index, subindex, false, itemData);
                // check size
                objDataMap[deviceId].at(itemName) = itemData;
            }
            TDEBUG("readServiceObject: deviceId:{}, index:{} ", deviceId, index);
        }
        return 0;
    }

    int32_t EcTbox::writeServiceObject(const std::string& objName, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>>& objDataMap)
    {
        for(auto dev : objDataMap) {
            uint16_t deviceId = dev.first;
            std::map<std::string, std::vector<uint8_t>> obj = dev.second;
            uint16_t index = getObjectIndex(deviceId, objName);
            // 可能不存在index/subindex //
            if (index==0)
                continue;  // TODO: check valid
            for (auto item : obj) {
                std::string itemName = item.first;
                std::vector<uint8_t> itemData = item.second;
                // TODO: check itemData.size
                itemData.resize(4);  // TODO: default 4bytes
                uint8_t subindex = getObjectItemSubindex(deviceId, objName, itemName);
                if (subindex==0)
                    continue;  // TODO: check valid
                mBus->sendSdoWrite(deviceId, index, subindex, false, itemData);
            }
            TDEBUG("writeServiceObject: deviceId:{}, index:{} ", deviceId, index);
        }
        return 0;
    }
#endif

#if 0
    // function test: resistor/capacitor...
    // How to trigger the actions? and how to synchronize slaves?????????????????
    int32_t EcTbox::sendAsyncRequest(AsyncRequest &req)
    {

    }

    int32_t EcTbox::recvAsyncRsponse()
    {

    }
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

int32_t EcTbox::setCallback(
    const std::function<void(Request)> &reqCallback,
    const std::function<void(Response)> &rspCallback)
{
    req_cb = reqCallback;
    rsp_cb = rspCallback;
    return 0;
}

// callback returns only id
int32_t EcTbox::setIdCallback(
    const std::function<void(int32_t)> &reqIdCallback,
    const std::function<void(int32_t)> &rspIdCallback)
{
    reqId_cb = reqIdCallback;
    rspId_cb = rspIdCallback;
    return 0;
}

// 增加状态的判断和返回 //
// flow control
int32_t EcTbox::canAccept()
{
    size_t size = 0;
    bool isrunning = false;
    {
        std::lock_guard<std::mutex> locker{this->m};
        size = this->reqQueue_.size();
        isrunning = this->m_runThreadRunning;
    }
    if (!isrunning)
    {
        TERROR("Can not accept request, because the thread is not running.");
        return -1;
    }
    // TDEBUG("The pending requests in the queue: {} out of {}", size, MAX_QUEUE_SIZE);
    int32_t avail = MAX_QUEUE_SIZE - size;
    if (avail <= 0)
    {
        // TDEBUG("The pending request buffer is insufficient to accept new requests");
        return -1;
    }
    else
    {
        return avail;
    }
}

// 可以根据mode拆分成set,get req //
int32_t EcTbox::sendRequest(Request &req)
{
    {
        std::lock_guard<std::mutex> locker{this->m};
        reqQueue_.push(req);
    }

    // if (req.reqMode.count(1)) {
    //     std::cout<<"id:"<< req.reqId << " reqMode:" <<req.reqMode.at(1)<<std::endl;
    // }

    // parse req
    if (req_cb)
    {
        req_cb(req);
    }
    if (reqId_cb)
    {
        reqId_cb(req.reqId);
    }
    return 0;
}

int32_t EcTbox::recvResponse(Response &rsp)
{
    {
        std::lock_guard<std::mutex> locker{this->m};
        if (!rspQueue_.empty())
        {
            rsp = rspQueue_.front();
            rspQueue_.pop();
            return 1;
        }
        return 0;
    }
}

int32_t EcTbox::dropResponse()
{
    {
        std::lock_guard<std::mutex> locker{this->m};
        if (!rspQueue_.empty())
        {
            // Response rsp = rspQueue_.front();
            rspQueue_.pop();
            return 1;
        }
        return 0;
    }
}

std::map<uint16_t, std::vector<uint8_t>> EcTbox::getDinStatus()
{
    std::lock_guard<std::mutex> locker{this->m};
    if (!currDinStatus.empty())
    {
        return currDinStatus;
    }
    return std::map<uint16_t, std::vector<uint8_t>>();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
    // user args: type, nicName, if reconnect
    int EcTbox::loadDriver(std::string type, std::string name, bool reConnect)
    {
        // close?
        if (mBus != nullptr)
        {
            if ((mBus->type() == type) && (mBus->name() == name))
            { // if connected
                if (reConnect)
                {
                    // mBus->connect(name);
                    mBus->open();
                }
                else
                {
                    return 0; // connected
                }
            }
        }
        if (type == "SIMBUS")
        {
            TDEBUG("load simbus");
            // mBus = new SimBus(name);
        }
        else if (type == "ECTBUS")
        {
            TDEBUG("load ectbus");
            mBus = new EctBus(name);
        }
        mBus->open();
        return 0;
    }
#endif

bool EcTbox::start()
{
    if (!mBus->isBusReady())
    {
        TERROR("Bus is not ready.");
        return false;
    }

    {
        std::lock_guard<std::mutex> locker{this->m};
        m_runThreadRunning = true;
    }

    //
    mBus->resetSyncPDO();

    // yinwenbo,20211020
    // 如果是硬件排错，需要重新扫描硬件的情况，在该进程已经处于运行的情况下？ //
    auto fn = [this]() -> void
    { this->run(); };
    m_runThread = std::thread(fn);
#if 0
        m_runThread = std::thread{
            [this]
            {
                run();
            }};
#endif
    TDEBUG("start running thread");
    return true;
}

// no use?
bool EcTbox::pause()
{
    {
        std::lock_guard<std::mutex> locker{this->m};
        m_runThreadRunning = false;
    }

    // 这是要把线程再 加入执行吗？ //
    if (m_runThread.joinable())
        m_runThread.join();

    // mBus->closeBus();
    TDEBUG("pause running thread");
    return true;
}

// no use, be replaced by start()
bool EcTbox::resume()
{
    {
        std::lock_guard<std::mutex> locker{this->m};
        m_runThreadRunning = true;
    }
    TDEBUG("resume running thread");
    return true;
}

bool EcTbox::stop()
{
    {
        std::lock_guard<std::mutex> locker{this->m};
        m_runThreadRunning = false;
    }

    // TODO: 这是要把线程再 加入执行吗？ //
    if (m_runThread.joinable())
    {
        m_runThread.join();
    }

    // mBus->closeBus();
    TDEBUG("stop running thread");
    return true;
}


bool EcTbox::release(std::vector<uint16_t> &deviceIdList)
{
    for (auto deviceId : deviceIdList)
    {
        // TODO: sendSdoWrite or setPDO
#define INDX_DOUT_RELEASE  (7)  // TODO
        mBus->setDoutControlSingle(deviceId, (INDX_DOUT_RELEASE-1), 1);  //
        // mBus->release(deviceId);
    }
    return true;
}


bool EcTbox::isRunning()
{
    bool isrunning = false;
    {
        std::lock_guard<std::mutex> locker{this->m};
        isrunning = m_runThreadRunning;
    }
    return isrunning;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////// private functions /////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// 退出前需要将队列中的req执行完 //
void EcTbox::run()
{
    // 该线程内部维护的数据结构 //
    // std::queue<Request> reqQueue;
    // std::queue<Response> rspQueue;
    // Request currReq = {};
    // Response currRsp = {};
    currReq.reqId = 0;
    mBus->resetSyncPDO();

    bool isrunning = false;
    bool pendingRsp = false;

    while (true)
    {
        // 临界区 //
        {
            std::lock_guard<std::mutex> locker{this->m};
            isrunning = m_runThreadRunning;
        }

        // process rsp
        // 所有slave都有响应，响应的数据由返回的rspMode决定 //
        // rsp写入队列，由上层软件从队列取出并处理，减少本线程的负荷 //
        if (mBus->isRspReady() && pendingRsp == true) // getready
        {
            currRsp.reqId = currReq.reqId;
            mBus->getRspPDO(currRsp.rspMode, currRsp.rspDinStatus, currRsp.rspIoFeedback, currRsp.rspMeasure);
            {
                std::lock_guard<std::mutex> locker{this->m};
                rspQueue_.push(currRsp); // for test, may overlow
            }
            if (rspId_cb)
            {
                rspId_cb(currRsp.reqId);
            }
            if (rsp_cb)
            {
                rsp_cb(currRsp); // 由rsp的回调函数，从rsp队列读取，回调函数中不计算 //
            }
            currReq.reqId = 0; // clear
            mBus->resetSyncPDO();
            pendingRsp = false;
        }

        // process req
        // should be all related device can accept new request
        if (mBus->canAcceptPDO() && pendingRsp == false) // idle
        {
            std::lock_guard<std::mutex> locker{this->m};
            if (reqQueue_.size() > 0)
            {
                currReq = reqQueue_.front();
                if (mBus->setReqPDO(currReq.reqMode, currReq.reqDoutControl, currReq.reqIoPattern))
                {
                    mBus->setSyncPDO();
                    reqQueue_.pop();
                    currRsp.reqId = currReq.reqId;
                    pendingRsp = true;
                }
            }
        }

        // 命令设置stop  无pending的请求  无pending的响应 //
        if (isrunning == false && reqQueue_.size() == 0 && pendingRsp == false)
        {
            // m_runThreadRunning = false;
            break;
        }

        // 更新pdo数据到devices //
        // 如果更新失败  是否需要停止线程  //
        if (mBus->updateBus() == 0)
        {
            // 更新dinStatus //
            mBus->getDinStatus(currDinStatus);

            using namespace std::chrono_literals;
            // std::this_thread::sleep_for(1ms);
            std::this_thread::sleep_for(1us); // 1ms, 1um, 1s...
        }
        else
        {
            TERROR("Could not update bus."); // Device Error!
            m_runThreadRunning = false;
            break;
        }
    }
}

#if 0
    void EcTbox::run_ori()
    {
        currReq.reqId = 0; // why not cleared?
        mBus->resetSyncPDO();

        // 每次循环采集外部启停信号m_runThreadRunning

        while (m_runThreadRunning)
        {

            //m_timeStamp = std::chrono::high_resolution_clock::now();

            // process rsp
            // 所有slave都有响应，响应的数据由返回的rspMode决定 //
            if (mBus->isRspReady()) // getready
            {
                //TDEBUG("run0");
                currRsp.reqId = currReq.reqId;
                mBus->getRspPDO(currRsp.rspMode, currRsp.rspDinStatus, currRsp.rspIoFeedback, currRsp.rspMeasure);
                rsp_cb(currRsp);  // just for test
                rspQueue_.push(currRsp); // for test, may overlow
                currReq.reqId = 0;       // clear
                mBus->resetSyncPDO();
            }

            // process req
            // should be all related device can accept new request
            if (mBus->canAcceptPDO())  // idle
            {
                //TDEBUG("run1");
                if (reqQueue_.size()>0) {
                    currReq = reqQueue_.front();
                    if (mBus->setReqPDO(currReq.reqMode, currReq.reqDoutControl, currReq.reqIoPattern))
                    {
                        // 需要所有的slave都设置sync吗？需要所有slave同步！！没有数据要求的，也要做同步响应！！ //
                        mBus->setSyncPDO();
                        reqQueue_.pop();
                        currRsp.reqId = currReq.reqId;
                    }
                }
            }

            // update bus
            bool isrun;
            {
                std::scoped_lock lock(m_update_mutex);
                if (mBus->updateBus()==0)
                {
                    isrun = true;
                }
                else
                {
                    TERROR("Could not update bus."); // Device Error!
                    isrun = false;
                    //TERROR("signal:updateBusFail");
                }
            }
            if (isrun)
            {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1us); // 1ms, 1um, 1s...
            }
            else
            {
                m_runThreadRunning = false;
            }
        }
    }
#endif

// scan run time config
// 上位机需要知道：deviceId,io count, di/do count, para list, //
int32_t EcTbox::generateObjectDict()
{
    m_serviceObjDict.clear();
    for (int i = 1; i <= getDeviceCount(); i++)
    { // TODO: from 1 to slavenum
        std::map<std::string, ServiceObject> serviceObjMap;
        serviceObjMap = mBus->generateObjectDict(i);
        m_serviceObjDict.insert({i, serviceObjMap});
    }

    m_flashObjDict.clear();
    for (int i = 1; i <= getDeviceCount(); i++)
    { // TODO: from 1 to slavenum
        std::map<std::string, FlashObject> flashObjMap;
        // TODO
        // mBus->generateFlashObject(i, flashObjMap);
        // m_flashObjDict.insert({i, flashObjMap});
    }
    return m_serviceObjDict.size(); // device count
}

uint16_t EcTbox::getObjectIndex(const uint16_t deviceId, const std::string &objName)
{
    if (m_serviceObjDict.count(deviceId) > 0)
    {
        if (m_serviceObjDict[deviceId].count(objName) > 0)
        {
            return m_serviceObjDict[deviceId].at(objName).objIndex;
        }
    }
    return 0; // TODO: OK?
}

uint8_t EcTbox::getObjectItemSubindex(const uint16_t deviceId, const std::string &objName, const std::string &itemName)
{
    if (m_serviceObjDict.count(deviceId) > 0)
    {
        if (m_serviceObjDict[deviceId].count(objName) > 0)
        {
            if (m_serviceObjDict[deviceId].at(objName).itemMap.count(itemName) > 0)
            {
                return m_serviceObjDict[deviceId].at(objName).itemMap[itemName].itemIndex;
            }
        }
    }
    return 0; // TODO: OK?
}

#if 0
    void EcTbox::printServiceObject()
    {
        // 打印各个device中的obj name type size ...  //
        for (uint16_t id = 1; id<= getDeviceCount(); id++) {
            std::vector<std::string> objNameList;
            getServiceObjectNameList(id, objNameList);
            std::cout << "device:" << id << std::endl;
            for (auto objName : objNameList) {
                std::map<uint16_t, ServiceObject> objMap;
                ServiceObject obj;
                getServiceObject(id, objName, obj);
                objMap.insert({id, obj});
                readServiceObject(objMap);
                obj = objMap[id];  // update data
                std::cout << "  object:0x" << std::hex << obj.objIndex << " name:" << obj.objName << " type:" << obj.objType << " size:" << obj.objSize << " value:";
                printArray(obj.objData);
                //std::cout << std::endl;
                for (auto it : obj.itemMap) {
                    ServiceItem item = it.second;
                    std::cout << "    item:" << item.itemIndex << " name:" << item.itemName << " type:" << item.itemType << " size:" << item.itemSize << " offset:" << item.itemOffset << " value:";
                    printArray(item.itemData);
                    //std::cout << std::endl;
                }
            }
        }
    }
#endif
//} // namespace tstudio
