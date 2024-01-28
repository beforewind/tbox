
#ifndef TBOX_H
#define TBOX_H

#define  TBOX_DLL

#ifdef TBOX_DLL
#include <windows.h>
#endif

#include <cassert>
#include <iostream>
//#include <string>
#include <map>
#include <vector>
#include <functional>
#include <variant>



enum FlashObjectType
{
    DUMMY,
    CONFIG,
    MODULE,
    FIRMWARE
    // customed by upper layer
};

struct FlashObject
{
    std::string objName; // <16bytes
    int32_t objSize;  // byte size
    uint32_t objType;
    std::vector<uint8_t> objData;
    // FlashObject() {}
} ;

struct ServiceItem  // entry
{
    std::string itemName; // <16bytes
    uint16_t itemIndex;
    uint16_t itemSize;
    uint32_t itemType;
    int32_t  itemOffset;
    std::vector<uint8_t> itemData;

    //ServiceItem() {};
};

struct ServiceObject
{
    std::string objName;   // <16bytes
    uint16_t objIndex;
    int32_t objSize;  // sum of item.dataSize
    uint32_t objType;  //
    std::vector<uint8_t> objData;

    std::variant<uint8_t, uint16_t, uint32_t, float_t, std::vector<uint8_t>> objValue;
    uint32_t valueIndex;  // index of objValue;

    ServiceItem entryItem;  // 辅助item, the first entry
    std::map<std::string, ServiceItem> itemMap;
    //std::vector<ServiceItem> itemList;  // groupSize in implicit
#if 1
            ServiceObject(){};
#if 0
            std::vector<uint8_t>& getData()
            {
                return objData;
            }

            int32_t setData(std::vector<uint8_t> data)
            {
                if (data.size()==objSize) {
                    objData = data;
                    return 0;
                }
                return -1;
            }
#endif
            // 引用的this //
            int32_t setItem(const std::string& itemName, const void* itemData, const int32_t itemSize) {
                if (itemMap.count(itemName)!=0) {
                    //auto item = itemMap.at(itemName);  // at()返回对与键关联的映射值的引用k //
                    ServiceItem& item = itemMap[itemName];
                    if (item.itemSize == itemSize) {
                        //item.itemData.clear();  // 原因在此！导致后面拷贝不正常！ //
                        //item.itemData.resize(itemSize);
                        // TODO: check
                        memcpy(&item.itemData[0], itemData, itemSize);
                        //itemMap.insert({itemName, item});  // 如果是引用itemMap，需要再次插入吗？ //
                        return 0;
                    }
                }
                return -1;
            }

            int32_t getItem(std::string itemName, void* itemData, int32_t itemSize) {
                if (itemMap.count(itemName)!=0) {
                    auto item = itemMap.at(itemName);
                    if (item.itemSize == itemSize) {
                        memcpy(itemData, &item.itemData.data()[0], itemSize);
                        return 0;  // 返回引用？ //
                    }
                }
                return -1;
            }
#if 0
            int32_t setItem(std::string itemName, std::vector<uint8_t> itemData) {
                if (itemMap.count(itemName)!=0) {
                    ServiceItem item = itemMap[itemName];
                    if (item.itemSize == itemData.size()) {
                        item.itemData = itemData;
                        itemMap.insert({itemName, item});
                        return 0;
                    }
                }
                return -1;
            }

            ServiceItem getItem(std::string itemName) {
                if (itemMap.count(itemName)!=0) {
                    return itemMap[itemName];
                }
                return ServiceItem();
            }
#endif


#endif
};

struct DeviceObject
{
    uint16_t deviceId;
    // hardware related
    //ServiceObject hardwareObj;
    //ServiceObject parameterObj;


    //            uint16_t ioCount;
    //            uint16_t diCount;
    //            uint16_t doCount;
    //            uint16_t ioPerBank;
    //            uint16_t bankCount;
    //            uint32_t hwVersion;

    std::map<std::string, ServiceObject> serviceObjMap;
    std::map<std::string, FlashObject> flashObjMap;
#if 0
            ServiceObject getObject(std::string objName) {
                if (objMap.count(objName)>0) {
                    return objMap.at(objName);
                }
                return objMap.end();
            }
#endif
};

typedef struct {
    UINT16 sync_valid : 2;
    UINT16 setdout_valid : 2;  // 00-no, 01-set all, 10-, 11-additional
    UINT16 setio_valid : 3;    // 000-no action, 001-LF, 010-HF, 101-LF additional, 110-HF additional, 111-all on
    UINT16 getio_valid : 3;    // 00-no action, 01-LF, 10-HF, 11-both
    UINT16 getmeasure_valid : 2;
    UINT16 getext_valid : 2;
    UINT16 reserved : 2;  // reserved
} REQ_SYNC_OBJ;

typedef struct {
    UINT16 sync_ready : 2;
    UINT16 setdout_ready : 2;  //
    UINT16 setio_ready : 3;
    UINT16 getio_ready : 3;
    UINT16 getmeasure_ready : 2;
    UINT16 getext_ready : 2;
    UINT16 reserved : 2;  // reserved
} RSP_SYNC_OBJ;

typedef struct {
    int32_t reqId;
    std::map<uint16_t, uint16_t> reqMode;
    std::map<uint16_t, std::vector<uint8_t>> reqDoutControl; // digital output，PC->MCU, LED、magnet etc. //
    std::map<uint16_t, std::vector<uint8_t>> reqIoPattern;   // IO stimuli pattern tx//
} Request;

typedef struct {
    int32_t reqId;
    std::map<uint16_t, uint16_t> rspMode;
    std::map<uint16_t, std::vector<uint8_t>> rspDinStatus;  // digital input, PC<-MCU, status input such as push button, presence pin //
    std::map<uint16_t, std::vector<uint8_t>> rspIoFeedback; // IO feedback pattern rx//
    std::map<uint16_t, std::vector<uint8_t>> rspMeasure;    // HS/LS/Current
    std::map<uint16_t, std::vector<uint32_t>> rspExtend;
} Response;


#ifdef TBOX_DLL
class Tbox
{
public:

//enum BusType
//{
//    SIMBUS,
//    ECTBUS
//};

Tbox() {}
virtual ~Tbox() {}
virtual void destroy() = 0;

virtual std::vector<std::string> findNic() = 0;
virtual int32_t connect(const std::string& busName) = 0;
virtual int32_t disconnect() = 0;
virtual int32_t getDeviceCount() = 0;
virtual int32_t updateFirmware(uint16_t deviceId, const std::string& fileName, std::vector<uint8_t>& fileData) = 0;

virtual bool start() = 0;
virtual bool pause() = 0;
virtual bool resume() = 0;
virtual bool stop() = 0;
virtual bool isRunning() = 0;
//virtual int32_t status() = 0;
//virtual void printServiceObject() = 0;

virtual int32_t setCallback(
    const std::function<void(Request)> &requestCallback,
    const std::function<void(Response)> &responseCallback) = 0;
virtual int32_t canAccept() = 0;  // -1:no available buffer, >=0:available buffer size
virtual int32_t sendRequest(Request &req) = 0;
virtual int32_t recvResponse(Response &rsp) = 0;

virtual int32_t getServiceObjectNameList(const uint16_t deviceId, std::vector<std::string>& objNameList) = 0;
virtual int32_t getServiceItemNameList(const uint16_t deviceId, const std::string& objName, std::vector<std::string>& itemNameList) = 0;

virtual int32_t getServiceObject(const uint16_t deviceId, const std::string& objName, ServiceObject& serviceObj) = 0;
virtual int32_t getServiceItem(const uint16_t deviceId, const std::string& objName, const std::string& itemName, ServiceItem& serviceItem) = 0;

virtual int32_t readServiceObjectData(const uint16_t deviceId, const std::string& objName, std::vector<uint8_t>& objData) = 0;
virtual int32_t writeServiceObjectData(const uint16_t deviceId, const std::string& objName, std::vector<uint8_t>& objData) = 0;
virtual int32_t readServiceItemData(const uint16_t deviceId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData) = 0;
virtual int32_t writeServiceItemData(const uint16_t deviceId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData) = 0;

virtual int32_t readServiceItem(const std::string& objName, std::map<uint16_t, ServiceItem>& itemMap) = 0;
virtual int32_t writeServiceItem(const std::string& objName, std::map<uint16_t, ServiceItem>& itemMap) = 0;
virtual int32_t readServiceObject(std::map<uint16_t, ServiceObject>& objMap) = 0;
virtual int32_t writeServiceObject(std::map<uint16_t, ServiceObject>& objMap) = 0;

virtual int32_t readServiceObject(const std::string& objName, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>>& objDataMap) = 0;
virtual int32_t writeServiceObject(const std::string& objName, std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>>& objDataMap) = 0;

#if 0
int32_t scanAdapters(
    std::map<uint16_t, std::vector<uint8_t>> &deviceDataList,
    std::map<uint16_t, std::map<std::string, std::vector<uint8_t>>> &adapterListMap);

// Foe Flash
int32_t getFlashObjectList(const uint16_t deviceId, const uint32_t objType, std::vector<FlashObject> &objList);
int32_t writeFlashObject(const uint16_t deviceId, FlashObject &obj);
int32_t readFlashObject(const uint16_t deviceId, FlashObject &obj);

int32_t getDeviceCount();

// Coe SDO/PDO
int32_t getDeviceObject(uint16_t deviceId, DeviceObject& devObj);
int32_t getDeviceObjectMap(std::map<uint16_t, DeviceObject>& deviceObjMap);
int32_t readServiceItem(std::string objName, std::map<uint16_t, ServiceItem>& itemMap);
int32_t readServiceObject(std::map<uint16_t, ServiceObject>& objMap);
int32_t writeServiceItem(std::string objName, std::map<uint16_t, ServiceItem>& itemMap);
int32_t writeServiceObject(std::map<uint16_t, ServiceObject>& objMap);

// PDO loop
int32_t setCallback(
    const std::function<void(Request)> &requestCallback,
    const std::function<void(Response)> &responseCallback);
#endif
}; // Tbox


// A factory of Tbox-implementing objects looks thus
typedef Tbox* (__cdecl *tbox_factory)();

// global
static HINSTANCE dll_handle=0;

static Tbox* loadTbox(const std::string& tboxName) {
    // Load the DLL
    std::string dllFullName = tboxName+".dll";
    dll_handle = ::LoadLibrary(TEXT(dllFullName.c_str()));
    if (!dll_handle) {
        std::cout<<"Unable to load DLL!" << std::endl;
        return nullptr;
    }
    // Get the function from the DLL
    tbox_factory factory_func = reinterpret_cast<tbox_factory>(
        ::GetProcAddress(dll_handle, "create_Tbox"));
    if (!factory_func) {
        std::cout<<"Unable to load create_Tbox from DLL!" << std::endl;
        ::FreeLibrary(dll_handle);
        return nullptr;
    }
    // Ask the factory for a new object implementing the IKlass
    // interface
    Tbox* instance = factory_func();
    return instance;
}

static bool unloadTbox(Tbox** tbox) {
    if (tbox==nullptr) {
        return true;
    }
    (*tbox)->destroy();
    (*tbox) = nullptr;
    ::FreeLibrary(dll_handle);
    return true;
}

// TODO
//static std::vector<std::string> findNic()
//{
//    std::map<std::string, std::string> nicNameListMap;
//    //TODO: nicNameListMap = EctBus::findAvailableNics();
//    std::vector<std::string> nicNameList;
//    // nicNameList.reserve(mNicList.size());
//    for (auto const &imap : nicNameListMap)
//        nicNameList.emplace_back(imap.first);
//    return nicNameList;
//}
#else

#endif

#endif // TBOX_H
