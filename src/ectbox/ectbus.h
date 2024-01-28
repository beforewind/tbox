#ifndef ECTBUS_H
#define ECTBUS_H

#include <atomic>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <chrono>
#include <inttypes.h>
#include <soem/ethercat.h>

#include "ectbox/slave.h"
#include "utils/tlogger.h"

#include "ectbox/parameter.h" // temp

#include "include/tbox.h"


class EctBus //: public BusBase
{
public:
    // Constructors / Destructor
    EctBus() = delete;
    EctBus(const std::string name);
    ~EctBus();

//    void destroy();
//    int open();
//    int close();
//    int updatePDO();

    static std::vector<std::string> getAdapterNames()
    {
        std::vector<std::string> result;
        ec_adaptert *adapter = ec_find_adapters();
        ec_adaptert *adapter_tmp = adapter;
        while (adapter != nullptr)
        {
            result.emplace_back(adapter->name);
            TDEBUG(adapter->name);
            TDEBUG(adapter->desc);
            adapter = adapter->next;    
        }
        ec_free_adapters(adapter_tmp);  // ec_free_adapters

        return result;
    }

    static std::map<std::string, std::string> findAvailableNics()
    {
        std::map<std::string, std::string> nicListMap;

        ec_adaptert *adapter = ec_find_adapters();
        ec_adaptert *adapter_tmp = adapter;
        int seqNumber = 0;
        while (adapter != nullptr)
        {
            std::string key = std::to_string(seqNumber) + "_" + adapter->desc;
            // nicListMap.insert(std::pair<std::string, std::string>(key, std::string(adapter->name)));
            nicListMap[key] = adapter->name;
            // TINFO("- Name: {}, description: {}", std::string(adapter->name) , key);
            // TINFO("- Name: '", std::string(adapter->name) , "', description: '" ,  std::string(adapter->desc) ,  "'");
            // TINFO(adapter->name);
            adapter = adapter->next;
            seqNumber++;
        }
        ec_free_adapters(adapter_tmp);  // ec_free_adapters
#if 0
        for (auto n : nicListMap)
        {
            TINFO("- Name: {}, description: {}", n.first, n.second);
        }
#endif
        return nicListMap;
    }

    bool openBus(const std::string ifname);

    // int EL7031setup(uint16 slave);
    //int mapPDO();

#if 0
    // 通过FOE读写数据到Slave的Flash中 //
    // index: flash的block地址 //
    int readFoeData(std::string& index, std::vector<uint8_t>& data, uint32_t len);
    int writeFoeData(std::string& index, std::vector<uint8_t>& data, uint32_t len);

    //
    int writeSdoReq();
    int readSdoRsp();
#endif

    int closeBus();
    bool isBusReady();
    int updateBus();
    bool waitUntilAllSlavesReachedOP();
    void createSlaves();
    //std::vector<Slave *> getslavelist();
    Slave* getSlave(uint16_t slaveId);
    bool updateSlave(uint16_t slaveId, const char *filename, unsigned char* data, int length);
    // int sendSdo(uint8 slavenum, slave::REQUEST_T Sdo);//暂时不实现
    // int sendSdo(uint8 slavenum, uint8_t*data, int32_t size);  // yinwenbo
    // int getSdo(uint8 slavenum, uint8_t*data, int32_t& size);  // yinwenbo
    //int sendSDO(uint16_t slaveId, std::string &sdoData);
    //int recvSDO(uint16_t slaveId, std::string &sdoData);
    int getslavenum();
    void testwd();

    bool checkStatus(std::vector<uint16_t> slaveIdList, uint8_t expectedStatus);
    bool checkAllStatus(uint8_t expectedStatus);
    bool setAllStatus(uint8_t expectedStatus);

    bool canAcceptPDO();
    // bool setReqPDODout(uint16_t index, std::map<uint16_t, uint8_t>& reqDout);  // 可能只需要发送Digital Output
    // bool setReqPDOData();  // 可能只需要发送IO激励
    bool setReqPDO(std::map<uint16_t, uint16_t> &reqMode, std::map<uint16_t, std::vector<uint8_t>> &reqDout, std::map<uint16_t, std::vector<uint8_t>> &reqData);

    bool isRspReady();
    bool setSyncPDO();
    bool resetSyncPDO();
    bool getRspPDO(std::map<uint16_t, uint16_t> &rspMode, std::map<uint16_t, std::vector<uint8_t>> &rspDin, std::map<uint16_t, std::vector<uint8_t>> &rspData, std::map<uint16_t, std::vector<uint8_t>> &rspMeasure);

    /*!
     * Send a reading SDO - specialization for strings
     * @param slave          Address of the slave.
     * @param index          Index of the SDO.
     * @param subindex       Sub-index of the SDO.
     * @param completeAccess Access all sub-indices at once.
     * @param value          Return argument, will contain the value which was read. The string needs to be preallocated to the correct size!
     * @return True if successful.
     */
    // template<>
    //bool EctBus::sendSdoRead(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, std::string &value);

    /*!
     * Send a writing SDO - specialization for strings
     * @param slave          Address of the slave.
     * @param index          Index of the SDO.
     * @param subindex       Sub-index of the SDO.
     * @param completeAccess Access all sub-indices at once.
     * @param value          Value to write.
     * @return True if successful.
     */
    // template<>
    //bool sendSdoWrite(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, const std::string value);

    // private
    char *dtype2string(uint16 dtype);
    char *SDO2string(uint16 slave, uint16 index, uint8 subidx, uint16 dtype);
    std::map<std::string, ServiceObject> generateObjectDict(uint16_t slaveId);
    PDODescription createSlavePDODescription(uint16_t slaveId);
    int32_t createPDODescription();





    bool foeWritePage(uint16_t slaveId, uint32_t pageIndex, int32_t pageSize, uint8_t *pageData);
    bool foeReadPage(uint16_t slaveId, uint32_t pageIndex, int32_t &pageSize, uint8_t *pageData);
    bool foeWriteData(const uint16_t slaveId, const std::string &objName, const uint32_t objType, std::vector<uint8_t> &objData);
    bool foeReadData(const uint16_t slaveId, const std::string &objName, const uint32_t objType, std::vector<uint8_t> &objData);
    bool loadSlaveMetaData(const uint16_t slaveId);
    bool saveSlaveMetaData(const uint16_t slaveId);
    std::map<std::string, std::pair<uint32_t, int32_t>> readFlashObjectMap(const uint16_t slaveId, const uint32_t objType);


    //bool sdoWriteData(const uint16_t slaveId, const std::string &objName, const uint32_t objType, std::vector<uint8_t> &objData);
    //bool sdoReadData(const uint16_t slaveId, const std::string &objName, const uint32_t objType, std::vector<uint8_t> &objData);
    //std::map<std::string, std::pair<uint32_t, int32_t>> readSdoObjectMap(const uint16_t slaveId, const uint32_t objType);




    std::string type() { return mType; }
    std::string name() { return mName; }


    int32_t getNumberOfSlaves();
    bool sendSdoRead(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, std::vector<uint8_t> &value);
    bool sendSdoWrite(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, std::vector<uint8_t>& value);


private:

    std::map<uint16_t, std::map<std::string, ServiceObject>> serviceObjMapDict;

    // std::string m_adapterName;
    // std::vector<Slave> m_slaves;  // 可以放在ectbus中，也可以放在tbox中
    // std::mutex m_update_mutex{};
    // bool m_slaves_initialized {false};

    std::string mType{};
    std::string mName{};
    bool m_busReady = false; // 这个bus在第一次进入op状态之后变为true，之后一直保持true直到关闭网口，即使期间不再是op状态也依然是true。
    bool slavesCreated = false;
    int m_expectedWKC;
    // std::vector<slave*> slavelist;
    //std::vector<Slave *> slavelist; // 要用个map维护
    std::map<std::uint16_t, Slave *> m_slaveMap;

    char m_IOmap[4096];
    void init();
    //bool foe(int slave, const char *filename);
    bool foe(int slave, const char* filename, unsigned char* data, int length);

    //       ecx_context ecx_context_ins;
    ec_slavet ec_slave[EC_MAXSLAVE];  // TODO: start from 1??
    /** number of slaves found on the network */
    int ec_slavecount = 0;
    /** slave group structure */
    ec_groupt ec_group[EC_MAXGROUP];

    /** cache for EEPROM read functions */
    uint8 ec_esibuf[EC_MAXEEPBUF];
    /** bitmap for filled cache buffer bytes */
    uint32 ec_esimap[EC_MAXEEPBITMAP];
    /** current slave for EEPROM cache buffer */
    ec_eringt ec_elist;
    ec_idxstackT ec_idxstack; //

    /** SyncManager Communication Type struct to store data of one slave */
    ec_SMcommtypet ec_SMcommtype[EC_MAX_MAPT];
    /** PDO assign struct to store data of one slave */
    ec_PDOassignt ec_PDOassign[EC_MAX_MAPT];
    /** PDO description struct to store data of one slave */
    ec_PDOdesct ec_PDOdesc[EC_MAX_MAPT];

    /** buffer for EEPROM SM data */
    ec_eepromSMt ec_SM; //
    /** buffer for EEPROM FMMU data */
    ec_eepromFMMUt ec_FMMU; //
    /** Global variable TRUE if error available in error stack */
    boolean EcatError = FALSE;
    int64 ec_DCtime = 0;
    ecx_portt ecx_port;
    ecx_redportt ecx_redport;

    // TODO:
    mutable std::recursive_mutex contextMutex_;

    ecx_contextt ecx_context_ins = {
        &ecx_port,         // .port          =
        &ec_slave[0],      // .slavelist     =
        &ec_slavecount,    // .slavecount    =
        EC_MAXSLAVE,       // .maxslave      =
        &ec_group[0],      // .grouplist     =
        EC_MAXGROUP,       // .maxgroup      =
        &ec_esibuf[0],     // .esibuf        =
        &ec_esimap[0],     // .esimap        =
        0,                 // .esislave      =
        &ec_elist,         // .elist         =
        &ec_idxstack,      // .idxstack      =
        &EcatError,        // .ecaterror     =
        0,                 // .DCt0          =
        0,                 // .DCl           =
        &ec_DCtime,        // .DCtime        =
        &ec_SMcommtype[0], // .SMcommtype    =
        &ec_PDOassign[0],  // .PDOassign     =
        &ec_PDOdesc[0],    // .PDOdesc       =
        &ec_SM,            // .eepSM         =
        &ec_FMMU,          // .eepFMMU       =
        nullptr,           // .FOEhook()
        nullptr,           // .EOEhook()
        0                  // .manualstatechange
    };
};

// extern "C" __declspec(dllexport) EctBus* getInstance();
// extern "C" __declspec(dllexport) int add(int a, int b);

#if 0
extern "C" __declspec(dllexport) SimTbox* __cdecl create()
{
    return new SimTbox;
}
#endif

#endif // ECTBUS_H
