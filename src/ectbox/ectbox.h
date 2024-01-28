
#ifndef ECTBOX_H
#define ECTBOX_H

//#include <cassert>
//#include <atomic>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <queue>
#include <chrono>
#include <functional>
#include <mutex>
//#include <variant>

//#include "utils/tlogger.h"

#include "ectbox/ectbus.h"
//#include "ectbox/slave.h"
#include <include/tbox.h>


//namespace tstudio
//{

#ifdef TBOX_DLL
class EcTbox : public Tbox
#else
class EcTbox
#endif
    {
    public:
        const uint32_t MAX_QUEUE_SIZE = 1024;

        EcTbox() { mBus = new EctBus("ECTBUS"); }
        // explicit Tbox(const std::string& name);
        ~EcTbox() {}

        const std::string &getName() const { return m_name; }
        void destroy()
        {
            delete this;
        }

        //static std::vector<std::string> findNic()
        std::vector<std::string> findNic()
        {
            std::map<std::string, std::string> nicNameListMap;
            nicNameListMap = EctBus::findAvailableNics();
            std::vector<std::string> nicNameList;
            // nicNameList.reserve(mNicList.size());
            for (auto const &imap : nicNameListMap)
                nicNameList.emplace_back(imap.first);
            return nicNameList;
        }

        int32_t connect(const std::string& busName);
        int32_t disconnect();
        int32_t getDeviceCount();
        int32_t updateFirmware(uint16_t deviceId, const std::string& fileName, std::vector<uint8_t>& fileData);


        int32_t scanAdapters(
            std::map<uint16_t, std::vector<uint8_t>> &deviceDataList,
            std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>> &adapterListMap);

        // Foe Flash
        int32_t getFlashObjectList(const uint16_t deviceId, const uint32_t objType, std::vector<FlashObject> &objList);
        int32_t writeFlashObject(const uint16_t deviceId, FlashObject &obj);
        int32_t readFlashObject(const uint16_t deviceId, FlashObject &obj);


        // CoE/SDO
        // 获取device中的obj name list //
        int32_t getServiceObjectNameList(const uint16_t deviceId, std::vector<std::string>& objNameList);
        int32_t getServiceItemNameList(const uint16_t deviceId, const std::string& objName, std::vector<std::string>& itemNameList);
        // 获取obj及item的数据结构 //
        //int32_t getDeviceObject(const uint16_t deviceId, DeviceObject& deviceObj);
        int32_t getServiceObjectMap(const std::string& objName, std::map<uint16_t, ServiceObject>& serviceObjMap);
        int32_t getServiceObject(const uint16_t deviceId, const std::string& objName, ServiceObject& serviceObj);
        int32_t getServiceItem(const uint16_t deviceId, const std::string& objName, const std::string& itemName, ServiceItem& serviceItem);

        // 按照数据操作单个的device //
        int32_t readServiceObjectData(const uint16_t deviceId, const std::string& objName, std::vector<uint8_t>& objData);
        int32_t writeServiceObjectData(const uint16_t deviceId, const std::string& objName, std::vector<uint8_t>& objData);
        int32_t readServiceItemData(const uint16_t deviceId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData);
        int32_t writeServiceItemData(const uint16_t deviceId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData);

        // 按照obj的item结构批量操作 //
        int32_t readServiceItem(const std::string& objName, std::map<uint16_t, ServiceItem>& itemMap);
        int32_t writeServiceItem(const std::string& objName, std::map<uint16_t, ServiceItem>& itemMap);
        int32_t readServiceObject(std::map<uint16_t, ServiceObject>& objMap);
        int32_t writeServiceObject(std::map<uint16_t, ServiceObject>& objMap);

        // 批量操作obj的各个分量，各个分量按照string关键字组成map，如设置相关device的参数组 //
        // 适合item都是32bit数据的场景，如参数和device硬件信息 //
        int32_t readServiceObject(const std::string& objName, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>>& objDataMap);
        int32_t writeServiceObject(const std::string& objName, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>>& objDataMap);

        // PDO
        int32_t setCallback(
            const std::function<void(Request)> &requestCallback,
            const std::function<void(Response)> &responseCallback);
        int32_t canAccept();
        int32_t sendRequest(Request &req);
        int32_t recvResponse(Response &rsp);

        // TODO
        //int32_t downloadFirmware(uint16_t slaveId, const std::string file, const std::string password);
        //int32_t uploadFirmware(uint16_t slaveId, const std::string file, const std::string password);
        //int32_t resetDevice(uint16_t slaveId);


        // control the loop thread
        bool start();
        bool pause();
        bool resume();
        bool stop();
        bool isRunning();
        //int32_t txQueueSize();


        //void printServiceObject();

    protected:
        void run();

        int32_t generateObjectDict();
        uint16_t getObjectIndex(const uint16_t deviceId, const std::string& objName);
        uint8_t getObjectItemSubindex(const uint16_t deviceId, const std::string& objName, const std::string& itemName);


    private:
        const std::string m_name = "EcTbox";
        EctBus *mBus{nullptr};
        std::string m_nicName = {""};
        bool m_initlialized{false};

        std::mutex m_update_mutex{};
        bool m_runThreadRunning{false};
        std::thread m_runThread;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_timeStamp;

        // optional
        // std::string m_adapterName;
        // std::vector<Slave> m_slaves;  // 可以放在ectbus中，也可以放在tbox中
        // std::mutex m_update_mutex{};
        // bool m_slaves_initialized {false};

        std::queue<Request> reqQueue_;
        std::queue<Response> rspQueue_;

        // int32_t currentRequestID = 0;
        Request currReq = {};
        Response currRsp = {};

        std::function<void(Request)> req_cb = nullptr;
        std::function<void(Response)> rsp_cb = nullptr;

        //std::map<uint16_t, DeviceObject> deviceObjMap;
        std::map<uint16_t, std::map<std::string, ServiceObject>> m_serviceObjDict;
        std::map<uint16_t, std::map<std::string, FlashObject>> m_flashObjDict;
    };

    // using TboxPtr = std::shared_ptr<Tbox>;

//} // namespace tstudio

#ifdef TBOX_DLL
    extern "C" __declspec(dllexport) Tbox* __cdecl create_Tbox(std::string& tboxName)
    {
        return new EcTbox();
    }
#else
#endif

#endif // ECTBOX_H
