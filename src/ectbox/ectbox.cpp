
#include <ectbox/ectbox.h>

//namespace tstudio
//{

    int32_t EcTbox::connect(const std::string& nicName)
    {
        // if connected

        std::map<std::string, std::string> nicNameListMap;
        nicNameListMap = EctBus::findAvailableNics();
        if (nicNameListMap.count(nicName)==0)
        {
            TERROR("There is no NIC with name: {}, please check the hardware!", nicName);
            return -1;
        }

        std::string busName = nicNameListMap[nicName];
        //TINFO("key:nicName:{}", nicName);
        //TINFO("value:busName:{}", busName);

        // TODO: 连接状态下的重新连接处理, 是否需要参数作为强制连接的使能信号？ //
        // 是否需要先stop进程？ //
        // if (connectStatus()) {
        disconnect();
        //}

        if (!mBus->openBus(busName.data())) // \Device\NPF_{35E61F5E-2469-4854-A3C5-B185FAC22FC4}
        {                                    // if (!mBus->openBus("\\Device\\NPF_{35E61F5E-2469-4854-A3C5-B185FAC22FC4}"))
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

        //mBus->createPDODescription(); // start from 1, to slavecount

        mBus->testwd();

        mBus->createSlaves();

        // create serviceObject
        generateObjectDict();

#endif
        return 0;
    }

    int32_t EcTbox::disconnect()
    {
        if (m_nicName == "")
            return 0;
        m_nicName = "";

        // stop run thread
        m_runThreadRunning = false;
        // 这是要把线程再 加入执行吗？
        if (m_runThread.joinable())
            m_runThread.join();
        mBus->closeBus();

        TINFO("disconnect successfully");
        return 0;
    }

    int32_t EcTbox::getDeviceCount()
    {
        return mBus->getNumberOfSlaves();
    }

    int32_t EcTbox::updateFirmware(uint16_t deviceId, const std::string& fileName, std::vector<uint8_t>& fileData)
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
        for (int i = 1; i <= getDeviceCount(); i++)  // from 1-n
        {
            // read Flash Objects to initialize device and adapters
            std::vector<uint8_t> configData;
            std::map<std::string, std::vector<uint8_t>> adapterList;
            std::vector<FlashObject> objList;
            getFlashObjectList(i, 0, objList);

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
        std::map<std::string, std::pair<uint32_t, int32_t>> objMap;  // (name,(type, size))
        objMap = mBus->readFlashObjectMap(deviceId, objType);

        for (auto obj : objMap)
        {
            FlashObject flashObj;  // (name,(type, size))
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
    int32_t EcTbox::getServiceObjectNameList(const uint16_t deviceId, std::vector<std::string>& objNameList)
    {
        objNameList.clear();
        if (m_serviceObjDict.count(deviceId)>0) {
            for (auto obj : m_serviceObjDict[deviceId]) {
                objNameList.push_back(obj.first);
            }
        }
        return 0;
    }

    int32_t EcTbox::getServiceItemNameList(const uint16_t deviceId, const std::string& objName, std::vector<std::string>& itemNameList)
    {
        itemNameList.clear();
        if (m_serviceObjDict.count(deviceId)>0) {
            if (m_serviceObjDict[deviceId].count(objName)>0) {
                for (auto item : m_serviceObjDict[deviceId][objName].itemMap) {
                    itemNameList.push_back(item.first);
                }
                return 0;
            }
        }
        return -1;
    }


    int32_t EcTbox::getServiceObjectMap(const std::string& objName, std::map<uint16_t, ServiceObject>& serviceObjMap)
    {
        serviceObjMap.clear();
        for (auto dev : m_serviceObjDict) {
            uint16_t deviceId = dev.first;
            if (dev.second.count(objName)>0) {
                serviceObjMap.insert({deviceId, dev.second[objName]});
            }
        }
        return 0;
    }

    int32_t EcTbox::getServiceObject(const uint16_t deviceId, const std::string& objName, ServiceObject& serviceObj)
    {
        if (m_serviceObjDict.count(deviceId)>0) {
            if (m_serviceObjDict[deviceId].count(objName)>0) {
                serviceObj = m_serviceObjDict[deviceId][objName];
                return 0;
            }
        }
        return -1;
    }

    int32_t EcTbox::getServiceItem(const uint16_t deviceId, const std::string& objName, const std::string& itemName, ServiceItem& serviceItem)
    {
        if (m_serviceObjDict.count(deviceId)>0) {
            if (m_serviceObjDict[deviceId].count(objName)>0) {
                if (m_serviceObjDict[deviceId][objName].itemMap.count(itemName)>0) {
                    serviceItem = m_serviceObjDict[deviceId][objName].itemMap[itemName];
                    return 0;
                }
            }
        }
        return -1;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    int32_t EcTbox::readServiceObjectData(const uint16_t deviceId, const std::string& objName, std::vector<uint8_t>& objData)
    {
        uint16_t index = getObjectIndex(deviceId, objName);
        // 可能不存在index/subindex //
        if (index==0)
            return -1;
        if (mBus->sendSdoRead(deviceId, index, 1, true, objData)) {
            return 0;
        }
        return -1;
    }

    int32_t EcTbox::writeServiceObjectData(const uint16_t deviceId, const std::string& objName, std::vector<uint8_t>& objData)
    {
        uint16_t index = getObjectIndex(deviceId, objName);
        // 可能不存在index/subindex //
        if (index==0)
            return -1;
        if (mBus->sendSdoWrite(deviceId, index, 1, true, objData)) {
            return 0;
        }
        return -1;
    }

    int32_t EcTbox::readServiceItemData(const uint16_t deviceId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData)
    {
        uint16_t index = getObjectIndex(deviceId, objName);
        // 可能不存在index/subindex //
        if (index==0)
            return -1;
        uint8_t subindex = getObjectItemSubindex(deviceId, objName, itemName);
        if (subindex==0)
            return -1;
        if (mBus->sendSdoRead(deviceId, index, subindex, false, itemData)) {
            return 0;
        }
        return -1;
    }

    int32_t EcTbox::writeServiceItemData(const uint16_t deviceId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData)
    {
        uint16_t index = getObjectIndex(deviceId, objName);
        // 可能不存在index/subindex //
        if (index==0)
            return -1;
        uint8_t subindex = getObjectItemSubindex(deviceId, objName, itemName);
        if (subindex==0)
            return -1;
        if (mBus->sendSdoWrite(deviceId, index, subindex, false, itemData)) {
            return 0;
        }
        return -1;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    // 获取对象的子项 //
    int32_t EcTbox::readServiceItem(const std::string& objName, std::map<uint16_t, ServiceItem>& itemMap)
    {
        for(auto dev : itemMap) {
            uint16_t deviceId = dev.first;
            ServiceItem item = dev.second;
            uint16_t index = getObjectIndex(deviceId, objName);
            // 可能不存在index/subindex //
            if (index==0)
                return -1;
            mBus->sendSdoRead(deviceId, index, item.itemIndex, false, item.itemData);
        }
        return 0;
    }

    // 设置对象的子项 //
    int32_t EcTbox::writeServiceItem(const std::string& objName, std::map<uint16_t, ServiceItem>& itemMap)
    {
        for(auto dev : itemMap) {
            uint16_t deviceId = dev.first;
            ServiceItem item = dev.second;
            uint16_t index = getObjectIndex(deviceId, objName);
            // 可能不存在index/subindex //
            if (index==0)
                return -1;
            mBus->sendSdoWrite(deviceId, index, item.itemIndex, false, item.itemData);
        }
        return 0;
    }

    // 获取某个类型的对象及其子项 //
    int32_t EcTbox::readServiceObject(std::map<uint16_t, ServiceObject>& objMap)
    {
        for(auto dev : objMap) {
            uint16_t deviceId = dev.first;
            ServiceObject obj = dev.second;
            for (auto subObj : obj.itemMap) {
                ServiceItem item = subObj.second;
                mBus->sendSdoRead(deviceId, obj.objIndex, item.itemIndex, false, item.itemData);
            }
        }
        return 0;
    }

    // 设置对象及其子项 //
    int32_t EcTbox::writeServiceObject(std::map<uint16_t, ServiceObject>& objMap)
    {
        for(auto dev : objMap) {
            uint16_t deviceId = dev.first;
            ServiceObject obj = dev.second;
            for (auto subObj : obj.itemMap) {
                ServiceItem item = subObj.second;
                mBus->sendSdoWrite(deviceId, obj.objIndex, item.itemIndex, false, item.itemData);
            }
        }
        return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

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
        }
        return 0;
    }


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

    // 增加状态的判断和返回 //
    // flow control
    int32_t EcTbox::canAccept()
    {
        std::cout << "queue size:"<<reqQueue_.size() << std::endl;
        if (reqQueue_.size() <= MAX_QUEUE_SIZE) {
            return (MAX_QUEUE_SIZE-reqQueue_.size());
        } else {
            return -1;
        }
        // return (reqQueue_.size() < MAX_QUEUE_SIZE);
        // return 0;
    }


    // 可以根据mode拆分成set,get req //
    int32_t EcTbox::sendRequest(Request &req)
    {
        reqQueue_.push(req);
        if (req.reqMode.count(1)) {
            std::cout<<"id:"<< req.reqId << " reqMode:" <<req.reqMode.at(1)<<std::endl;
        }
        // ack req is pending

        // parse req
        if (req_cb) {
            req_cb(req);
        }
        return 0;
    }

    int32_t EcTbox::recvResponse(Response &rsp)
    {
        Response r;
        if (!rspQueue_.empty())
        {
            r = rspQueue_.front();
            rspQueue_.pop();
        }
        return 0;
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
            TINFO("load simbus");
            // mBus = new SimBus(name);
        }
        else if (type == "ECTBUS")
        {
            TINFO("load ectbus");
            mBus = new EctBus(name);
        }
        mBus->open();
        return 0;
    }
#endif


    bool EcTbox::start()
    {
        if (!mBus->isBusReady())
            return false;
        m_runThreadRunning = true;

        //
        mBus->resetSyncPDO();

        // yinwenbo,20211020
        // 如果是硬件排错，需要重新扫描硬件的情况，在该进程已经处于运行的情况下？ //
        m_runThread = std::thread{
            [this]
            {
                run();
            }};
        TINFO("start run thread");
        // qDebug()<<QString("find %1 slaves").arg(getAllSlaves());
        return true;
    }

    bool EcTbox::pause()
    {
        m_runThreadRunning = false;
        // 这是要把线程再 加入执行吗？ //
        if (m_runThread.joinable())
            m_runThread.join();

        // need?
        mBus->closeBus();
        TINFO("pause running thread");
        return true;
    }

    bool EcTbox::resume()
    {
        m_runThreadRunning = true;
        return true;
    }

    bool EcTbox::stop()
    {
        m_runThreadRunning = false;
        // 这是要把线程再 加入执行吗？ //
        if (m_runThread.joinable()) {
            m_runThread.join();
        }

        mBus->closeBus();
        TINFO("stop running thread");
        return true;
    }

    bool EcTbox::isRunning()
    {
        return m_runThreadRunning;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////// private functions /////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////

    void EcTbox::run()
    {
        currReq.reqId = 0; // why not cleared?
        mBus->resetSyncPDO();

        while (m_runThreadRunning)
        {

            //m_timeStamp = std::chrono::high_resolution_clock::now();

            // process rsp
            // 所有slave都有响应，响应的数据由返回的rspMode决定 //
            if (mBus->isRspReady()) // getready
            {
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

            // updateBus here?

            // update bus
            bool isrun;
            {
                std::scoped_lock lock(m_update_mutex);
                if (mBus->updateBus())
                {
                    isrun = true;
                }
                else
                {
                    TINFO("Could not update bus."); // 说明有TPU掉线了！！！硬件故障！！
                    isrun = false;
                    TINFO("signal:updateBusFail");
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
        // do something
        // emit connectionLost();  // 该信号要发给上层用 //
        // disConnectNic();
        // qDebug()<<"signal:connectionLost";
    }


    // scan run time config
    // 上位机需要知道：deviceId,io count, di/do count, para list, //
    int32_t EcTbox::generateObjectDict()
    {
        m_serviceObjDict.clear();
        for (int i = 1; i <= getDeviceCount(); i++) {  // TODO: from 1 to slavenum
            std::map<std::string, ServiceObject> serviceObjMap;
            serviceObjMap = mBus->generateObjectDict(i);
            m_serviceObjDict.insert({i, serviceObjMap});
        }

        m_flashObjDict.clear();
        for (int i = 1; i <= getDeviceCount(); i++) {  // TODO: from 1 to slavenum
            std::map<std::string, FlashObject> flashObjMap;
            // TODO
            //mBus->generateFlashObject(i, flashObjMap);
            //m_flashObjDict.insert({i, flashObjMap});
        }
        return m_serviceObjDict.size();  // device count
    }

    uint16_t EcTbox::getObjectIndex(const uint16_t deviceId, const std::string& objName)
    {
        if (m_serviceObjDict.count(deviceId)>0) {
            if (m_serviceObjDict[deviceId].count(objName)>0) {
                return m_serviceObjDict[deviceId].at(objName).objIndex;
            }
        }
        return 0;  // TODO: OK?
    }

    uint8_t EcTbox::getObjectItemSubindex(const uint16_t deviceId, const std::string& objName, const std::string& itemName)
    {
        if (m_serviceObjDict.count(deviceId)>0) {
            if (m_serviceObjDict[deviceId].count(objName)>0) {
                if (m_serviceObjDict[deviceId].at(objName).itemMap.count(itemName)>0) {
                    return m_serviceObjDict[deviceId].at(objName).itemMap[itemName].itemIndex;
                }
            }
        }
        return 0;  // TODO: OK?
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
