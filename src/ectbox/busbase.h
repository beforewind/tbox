#ifndef BUSBASE_H
#define BUSBASE_H

// std
#include <atomic>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <chrono>
#include <cassert>
//#include <deque>
//#include <iostream>
#include <mutex>

#include <soem/ethercat.h>

//#include <ethercat/slavebase.h>

class SlaveBase;
using SlaveBasePtr = std::shared_ptr<SlaveBase>;

class BusBase
{
public:
    using PdoSizePair = std::pair<uint16_t, uint16_t>;
    using PdoSizeMap = std::unordered_map<std::string, PdoSizePair>;

    BusBase() = delete;
    BusBase(const std::string& type, const std::string& name);
    virtual ~BusBase() {}

    //virtual std::vector<std::string> getAdapterNames() = 0;

    virtual int open() = 0;
    virtual int close() = 0;
    virtual int updatePDO() = 0;
    virtual int sendSDO() = 0;
    virtual int recvSDO() = 0;
    // virtual void destroy() = 0;
    // virtual int init() = 0;
	// virtual int connect(std::string nicName) = 0;
	// virtual int disconnect() = 0;
    const std::string type() { return mType; }
    void setType(std::string type) { mType = type; }
    const std::string& getName() const { return mName; }
    const std::string name() { return mName; }
    void setName(std::string name) { mName = name; }

    const std::chrono::time_point<std::chrono::high_resolution_clock>& getUpdateReadStamp() const { return updateReadStamp_; }
    const std::chrono::time_point<std::chrono::high_resolution_clock>& getUpdateWriteStamp() const { return updateWriteStamp_; }
    static bool busIsAvailable(const std::string& name);
    static void printAvailableBusses();
    bool busIsAvailable() const;
    int getNumberOfSlaves() const;
    bool addSlave(const SlaveBasePtr& slave);
    bool startup(const bool sizeCheck = true);
    void updateRead();
    void updateWrite();
    void shutdown();
    void setState(const uint16_t state, const uint16_t slave = 0);
    bool waitForState(const uint16_t state, const uint16_t slave = 0, const unsigned int maxRetries = 40, const double retrySleep = 0.001);
    std::string getErrorString(ec_errort error);
    void printALStatus(const uint16_t slave = 0);
    bool checkForSdoErrors(const uint16_t slave, const uint16_t index);
    void syncDistributedClock0(const uint16_t slave, const bool activate, const double cycleTime, const double cycleShift);
    PdoSizeMap getHardwarePdoSizes();
    PdoSizePair getHardwarePdoSizes(const uint16_t slave);
    //bool sendSdoWrite(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, const Value value);
    //bool sendSdoRead(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, Value& value);
    //bool sendSdoReadVisibleString(const uint16_t slave, const uint16_t index, const uint8_t subindex, std::string& value);
    int getExpectedWorkingCounter(const uint16_t slave = 0) const;
    bool workingCounterIsOk() const;
    bool busIsOk() const;

    //void readTxPdo(const uint16_t slave, TxPdoInfo& txPdo);
    //void writeRxPdo(const uint16_t slave, const RxPdoInfo& rxPdo);
protected:
    //! Type of the bus.
    std::string mType;

    //! Name of the bus.
    std::string mName;

    //! Whether the bus has been initialized successfully
    bool initlialized_{false};

    //! List of slaves.
    std::vector<SlaveBasePtr> slaves_;

    //! Bool indicating whether PDO data has been sent and not read yet.
    bool sentProcessData_{false};

    //! Working counter of the most recent PDO.
    std::atomic<int> wkc_;

    //! Time of the last successful PDO reading.
    std::chrono::time_point<std::chrono::high_resolution_clock> updateReadStamp_;
    //! Time of the last successful PDO writing.
    std::chrono::time_point<std::chrono::high_resolution_clock> updateWriteStamp_;

    //! Maximal number of retries to configure the EtherCAT bus.
    const unsigned int ecatConfigMaxRetries_{5};
    //! Time to sleep between the retries.
    const double ecatConfigRetrySleep_{1.0};

    //! Count working counter too low in a row.
    unsigned int workingCounterTooLowCounter_{0};
    //! Maximal number of working counter to low.
    const unsigned int maxWorkingCounterTooLow_{100};

    // EtherCAT input/output mapping of the slaves within the datagrams.
    char ioMap_[4096];

    // EtherCAT context data elements:

    // Port reference.
    ecx_portt ecatPort_;
    // List of slave data. Index 0 is reserved for the master, higher indices for the slaves.
    ec_slavet ecatSlavelist_[EC_MAXSLAVE];
    // Number of slaves found in the network.
    int ecatSlavecount_{0};
    // Slave group structure.
    ec_groupt ecatGrouplist_[EC_MAXGROUP];
    // Internal, reference to EEPROM cache buffer.
    uint8 ecatEsiBuf_[EC_MAXEEPBUF];
    // Internal, reference to EEPROM cache map.
    uint32 ecatEsiMap_[EC_MAXEEPBITMAP];
    // Internal, reference to error list.
    ec_eringt ecatEList_;
    // Internal, reference to processdata stack buffer info.
    ec_idxstackT ecatIdxStack_;
    // Boolean indicating if an error is available in error stack.
    boolean ecatError_{FALSE};
    // Reference to last DC time from slaves.
    int64 ecatDcTime_{0};
    // Internal, SM buffer.
    ec_SMcommtypet ecatSmCommtype_[EC_MAX_MAPT];
    // Internal, PDO assign list.
    ec_PDOassignt ecatPdoAssign_[EC_MAX_MAPT];
    // Internal, PDO description list.
    ec_PDOdesct ecatPdoDesc_[EC_MAX_MAPT];
    // Internal, SM list from EEPROM.
    ec_eepromSMt ecatSm_;
    // Internal, FMMU list from EEPROM.
    ec_eepromFMMUt ecatFmmu_;

    mutable std::recursive_mutex contextMutex_;
    // EtherCAT context data.
    // Note: SOEM does not use dynamic memory allocation (new/delete). Therefore
    // all context pointers must be null or point to an existing member.
    ecx_contextt ecatContext_ = {&ecatPort_,
                                 &ecatSlavelist_[0],
                                 &ecatSlavecount_,
                                 EC_MAXSLAVE,
                                 &ecatGrouplist_[0],
                                 EC_MAXGROUP,
                                 &ecatEsiBuf_[0],
                                 &ecatEsiMap_[0],
                                 0,
                                 &ecatEList_,
                                 &ecatIdxStack_,
                                 &ecatError_,
                                 0,
                                 0,
                                 &ecatDcTime_,
                                 &ecatSmCommtype_[0],
                                 &ecatPdoAssign_[0],
                                 &ecatPdoDesc_[0],
                                 &ecatSm_,
                                 &ecatFmmu_,
                                 nullptr};

};

//extern "C" __declspec(dllexport) long createTbox(BusBase** ppObj);
//extern "C" __declspec(dllexport) TboxBase* __cdecl create();

//typedef std::unique_ptr<Tbox> TboxPluginPtr;
//extern "C" TboxPluginPtr TboxPlugin_new();

#endif // BUSBASE_H
