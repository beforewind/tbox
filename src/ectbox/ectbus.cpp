
#include <ectbox/ectbus.h>

EctBus::EctBus(const std::string name) // :BusBase("ectbus", name)
{
    //
    // TDEBUG("This is EctBus, bus type is: {}, bus name is:{}", type(), getName());
}

EctBus::~EctBus()
{
}

#if 0
void EctBus::destroy()
{
    delete this;
}

int EctBus::open()
{

    return 0;
}

int EctBus::close()
{
    return 0;
}

int EctBus::updatePDO()
{
    return 0;
}
#endif

void EctBus::init()
{
    m_busReady = false;
    slavesCreated = false;
    m_expectedWKC = 0;
    memset(&ec_slave, 0, sizeof(ec_slave));
    memset(&ec_group, 0, sizeof(ec_group));
    memset(&ec_esibuf, 0, sizeof(ec_esibuf));
    memset(&ec_esimap, 0, sizeof(ec_esimap));
    memset(&ec_elist, 0, sizeof(ec_elist));
    memset(&ec_idxstack, 0, sizeof(ec_idxstack));
    memset(&ec_SMcommtype, 0, sizeof(ec_SMcommtype));
    memset(&ec_PDOassign, 0, sizeof(ec_PDOassign));
    memset(&ec_PDOdesc, 0, sizeof(ec_PDOdesc));
    memset(&ec_SM, 0, sizeof(ec_SM));
    memset(&ec_FMMU, 0, sizeof(ec_FMMU));
    memset(&ecx_port, 0, sizeof(ecx_port));
    memset(&ecx_redport, 0, sizeof(ecx_redport));
    memset(&m_IOmap, 0, sizeof(m_IOmap));
}

bool EctBus::openBus(const std::string ifname)
{
    init();
    if (ecx_init(&ecx_context_ins, ifname.data()) > 0)
    {
        std::cerr << "Initialized bus on adapter " << ifname.data() << "\n";
        return true;
    }

    std::cerr << "Could not init EtherCAT on adpater " << ifname.data() << "\n";
    return false;
}

#if 0
int EctBus::openBus()
{
    init();
    if (ecx_init(&ecx_context_ins, mName.data()) >0)
    {
        TDEBUG("Initialized bus on adapter {}", mName.data());
        return 0;
    }
    TERROR("Could not init EtherCAT on adpater {}", mName.data());
    return -1;
}
#endif

#if 0
// remapping hook
int EL7031setup(ecx_contextt *context, uint16 slave)
{
    int retval;
    uint16 u16val;

    // map velocity
    uint16 map_1c12[4] = {0x0003, 0x1601, 0x1602, 0x1604};
    uint16 map_1c13[3] = {0x0002, 0x1a01, 0x1a03};

    retval = 0;

    // Set PDO mapping using Complete Access
    // Strange, writing CA works, reading CA doesn't
    // This is a protocol error of the slave.
    retval += ecx_SDOwrite(context, slave, 0x1c12, 0x00, TRUE, sizeof(map_1c12), &map_1c12, EC_TIMEOUTSAFE);
    retval += ecx_SDOwrite(context, slave, 0x1c13, 0x00, TRUE, sizeof(map_1c13), &map_1c13, EC_TIMEOUTSAFE);
    // retval += ec_SDOwrite(slave, 0x1c12, 0x00, TRUE, sizeof(map_1c12), &map_1c12, EC_TIMEOUTSAFE);
    // retval += ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_1c13), &map_1c13, EC_TIMEOUTSAFE);

    // int send = ecx_SDOwrite(&ecx_context_ins, (slave), INDX_SDO_REQ, 1,
    //     true, sdoData.size(), sdoData.data(), EC_TIMEOUTSTATE * 3);

    // bug in EL7031 old firmware, CompleteAccess for reading is not supported even if the slave says it is.
    // ec_slave[slave].CoEdetails &= ~ECT_COEDET_SDOCA;
    // while(EcatError) printf("%s", ec_elist2string());

    printf("EL7031 slave %d set, retval = %d\n", slave, retval);
    return 1;
}


int EctBus::mapPDO()
{
    int wkc;
    wkc = ecx_config_init(&ecx_context_ins, FALSE);
    if (wkc == 0)
        return wkc;

#if 0 // 貌似没有查询到该状态
    // check state, should be
    ecx_statecheck(&ecx_context_ins,0, EC_STATE_PRE_OP,  EC_TIMEOUTSTATE * 3);
    if (ecx_context_ins.slavelist[0].state != EC_STATE_PRE_OP ) {
        std::cout << "Not all slaves reached pre operational state." << std::endl;
        return 0;
    }
#endif
    std::cout << "here" << std::endl;

    for (int i = 1; i <= *(ecx_context_ins.slavecount); i++)
    {
        if ((ec_slave[i].eep_man == 0x0000abcd) && (ec_slave[i].eep_id == 0x00000001))
        {
            printf("Found %s at position %d\n", ec_slave[i].name, i);
            // if (ecx_context_ins.slavelist[i].name == "tbox_node")
            {
                printf("Found %s at position %d\n", ec_slave[i].name, i);
                // link slave specific setup to preop->safeop hook
                ecx_context_ins.slavelist[i].PO2SOconfigx = &EL7031setup;
            }
        }
    }

    ecx_config_map_group(&ecx_context_ins, &m_IOmap, 0);

    return wkc;
}
#endif

int ec_config_mul(ecx_contextt *context, uint8 usetable, void *pIOmap);

int ec_config_mul(ecx_contextt *context, uint8 usetable, void *pIOmap)
{
    int wkc;
    wkc = ecx_config_init(context, usetable);
    if (wkc)
    {
        ecx_config_map_group(context, pIOmap, 0);
    }
    return wkc;
}

bool EctBus::waitUntilAllSlavesReachedOP()
{
    //    IO_offset.clear();
    //     IO_num = 0;
    //      m_busReady = false;
    if (ec_config_mul(&ecx_context_ins, FALSE, &m_IOmap) > 0)
    // if (mapPDO() > 0)
    {
        ecx_configdc(&ecx_context_ins);
        while (*(ecx_context_ins.ecaterror))
            printf("%s", ecx_elist2string(&ecx_context_ins));
        printf("%d slaves found and configured.\n", *(ecx_context_ins.slavecount));
        /* wait for all slaves to reach SAFE_OP state */
        ecx_statecheck(&ecx_context_ins, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 3);
        if (ecx_context_ins.slavelist[0].state != EC_STATE_SAFE_OP)
        {
            printf("Not all slaves reached safe operational state.\n");
            ecx_readstate(&ecx_context_ins);
            for (int i = 1; i <= *(ecx_context_ins.slavecount); i++)
            {
                if (ecx_context_ins.slavelist[i].state != EC_STATE_SAFE_OP)
                {
                    printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                           i, ecx_context_ins.slavelist[i].state, ecx_context_ins.slavelist[i].ALstatuscode, ec_ALstatuscode2string(ecx_context_ins.slavelist[i].ALstatuscode));
                }
            }
        }
        else
        {
            std::cerr << "All slaves reached SAFE-OP\n";

            m_expectedWKC = (ecx_context_ins.grouplist[0].outputsWKC * 2) + ecx_context_ins.grouplist[0].inputsWKC;
            printf("Calculated workcounter %d\n", m_expectedWKC);

            for (int slave = 1; slave <= *(ecx_context_ins.slavecount); slave++)
            {
                ecx_context_ins.slavelist[slave].state = EC_STATE_OPERATIONAL;
                ecx_writestate(&ecx_context_ins, slave);
            }
            ecx_statecheck(&ecx_context_ins, 0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE * 3);
            for (int i = 1; i <= *(ecx_context_ins.slavecount); i++)
            {
                if (ecx_context_ins.slavelist[i].state != EC_STATE_OPERATIONAL)
                {
                    printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                           i, ecx_context_ins.slavelist[i].state, ecx_context_ins.slavelist[i].ALstatuscode, ec_ALstatuscode2string(ecx_context_ins.slavelist[i].ALstatuscode));
                }
                else
                {
                    m_busReady = true;
                    printf("All slaves reached OP\n");  // TODO: should be slave[i] reached OP?
                }
            }
        }
        return true;
    }
    else
    {
        printf("No slaves found!\n");
        closeBus();
        return false;
    }
}

int EctBus::updateBus()
{
    if (m_busReady)
    {
        ecx_send_processdata_group(&ecx_context_ins, 0);
        int lastWorkCounter = ecx_receive_processdata_group(&ecx_context_ins, 0, EC_TIMEOUTMON * 10);
        int aa = 1;
        if (lastWorkCounter >= m_expectedWKC) // 是不是要更新？？ //
        {
            return true;
        }
        //       return true;//pigfly
    }
    return 0;
}

int EctBus::closeBus()
{
    ecx_close(&ecx_context_ins);
    m_busReady = false;
    {
        for (auto slv : m_slaveMap) {
            delete slv.second;
        }
        m_slaveMap.clear();

//        for (size_t i = 0; i < slavelist.size(); i++)
//        {
//            delete slavelist[i];
//        }
//        slavelist.clear();
    }
    TDEBUG("Closed bus");
    return 0;
}

bool EctBus::isBusReady()
{
    return m_busReady;
}

// 需要支持重新创建，尤其是硬件排错阶段
void EctBus::createSlaves()
{
    // yinwenbo,20211020
    if (slavesCreated)
    {
        // 需要reset已经建立的数据结构，应该不需要修改连接进程的状态变量？
        // slavelist.clear();  // TODO
        // qDebug()<<"Slave have already been created! There must be a logic error.";
        throw std::logic_error("Slave have already been created! There must be a logic error.");
    }

    if (!m_busReady)
    {
        std::cerr << "Bus not ready!\n";
    }
    else
    {
        for (uint16_t i = 1; i <= *(ecx_context_ins.slavecount); i++)
        {
            // 这里需要将PDO分开
            //Slave *slave_ = new Slave((RESPONSE_SYNC_T *)ecx_context_ins.slavelist[i].inputs, (REQUEST_SYNC_T *)ecx_context_ins.slavelist[i].outputs, i);

            // read DeviceObject
            std::map<std::string, ServiceObject> serviceObjDict = generateObjectDict(i);
            // read PDO mapping

            Slave *slave_ = new Slave(&ecx_context_ins.slavelist[i], i, ecx_context_ins.slavelist[i].inputs, ecx_context_ins.slavelist[i].outputs);
            m_slaveMap.insert({i, slave_});
            //slavelist.push_back(slave_);
            serviceObjMapDict.insert({i, serviceObjDict});
        }
    }

    slavesCreated = true;
}

#if 0
std::vector<Slave *> EctBus::getslavelist()
{
    //    if(slavesCreated)
    //        std::cout<<"the slavelist has not been not created" << std::endl;
    //    else
    //        std::cout<<"the slavelist has been created" << std::endl;
    return slavelist;
}
#endif

// TODO: slaveId是从1开始的！！//
Slave* EctBus::getSlave(uint16_t slaveId)
{
    if (m_slaveMap.count(slaveId)>0) {
        return m_slaveMap[slaveId];
    }
    return nullptr;

//    if (slaveId < *ecx_context_ins.slavecount) {
//        return slavelist[slaveId];
//    }
//    return nullptr;
}

// update firmware
#if 1
bool EctBus::updateSlave(uint16_t slaveId, const char* filename, unsigned char* data, int length)
{
    if (m_busReady)
    {
        ecx_context_ins.slavelist[slaveId].state = EC_STATE_INIT;
        ecx_writestate(&ecx_context_ins, slaveId);
        ecx_statecheck(&ecx_context_ins, slaveId, EC_STATE_INIT, EC_TIMEOUTSTATE * 3);
        ecx_context_ins.slavelist[slaveId].state = EC_STATE_BOOT;
        ecx_writestate(&ecx_context_ins, slaveId);
        ecx_statecheck(&ecx_context_ins, slaveId, EC_STATE_BOOT, EC_TIMEOUTSTATE * 3);
        // (int slave, const char* filename, unsigned char* data, int length)
        bool loadsuccess = foe(slaveId, filename, data, length); // 这里需要foe吗？
        ecx_context_ins.slavelist[slaveId].state = EC_STATE_INIT;
        ecx_writestate(&ecx_context_ins, slaveId);
        ecx_statecheck(&ecx_context_ins, slaveId, EC_STATE_INIT, EC_TIMEOUTSTATE * 3);
        if (loadsuccess)
            return true;
        else
            return false;
    }
    else
        return false;
}
#else
bool EctBus::updateSlave(uint16_t slaveId, const char *filename)
{
    if (m_busReady)
    {
        ecx_context_ins.slavelist[slaveId].state = EC_STATE_INIT;
        ecx_writestate(&ecx_context_ins, slaveId);
        ecx_statecheck(&ecx_context_ins, slaveId, EC_STATE_INIT, EC_TIMEOUTSTATE * 3);
        ecx_context_ins.slavelist[slaveId].state = EC_STATE_BOOT;
        ecx_writestate(&ecx_context_ins, slaveId);
        ecx_statecheck(&ecx_context_ins, slaveId, EC_STATE_BOOT, EC_TIMEOUTSTATE * 3);
        bool loadsuccess = foe(slaveId, filename); // 这里需要foe吗？
        ecx_context_ins.slavelist[slaveId].state = EC_STATE_INIT;
        ecx_writestate(&ecx_context_ins, slaveId);
        ecx_statecheck(&ecx_context_ins, slaveId, EC_STATE_INIT, EC_TIMEOUTSTATE * 3);
        if (loadsuccess)
            return true;
        else
            return false;
    }
    else
        return false;
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief EctBus::foeReadPage
/// \param slaveId
/// \param pageIndex
/// \param pageSize
/// \param pageData
/// \return
/// TODO:需要加上checksum，就是将所有有效数据累加生成的值存储在数据头部 //
bool EctBus::foeReadPage(uint16_t slaveId, uint32_t pageIndex, int32_t &pageSize, uint8_t *pageData)
{
    if (pageIndex >= COUNT_FLASH_PAGE)
    { // TODO
        return false;
    }
    uint32_t pageAddr = ADDR_FLASH_BASE + pageIndex * SIZE_FLASH_PAGE;
    // pageData.clear();
    // pageData.resize(pageSize, 0);
    // if (pageData.size()<dataOffset+pageSize) {
    //     // TODO: check pagaDate.size if enough
    // }
    char name[16] = "page\000";
    int32_t size = pageSize;
    int wkc = ecx_FOEread(&ecx_context_ins, slaveId, name, pageAddr, &size, pageData, EC_TIMEOUTMON * 10);
    if (wkc > 0)
    {
        // TODO:
    }
    if (size != pageSize)
    {
        // TODO:
        pageSize = size;
    }
    return true;
}

// mcu should erase page first
bool EctBus::foeWritePage(uint16_t slaveId, uint32_t pageIndex, int32_t pageSize, uint8_t *pageData)
{
    if (pageIndex >= COUNT_FLASH_PAGE)
    { // TODO
        return false;
    }
    uint32_t pageAddr = ADDR_FLASH_BASE + pageIndex * SIZE_FLASH_PAGE;
    char name[16] = "page\000";
    int wkc = ecx_FOEwrite(&ecx_context_ins, slaveId, name, pageAddr, pageSize, pageData, EC_TIMEOUTMON * 10);
}

// 根据itemName从flash的meta中寻找数据itemData,可能有多个itemData，需要拼接 //
// 1-查询slave的meta数据表，如果meta数据没有加载，那么从flash加载；如果meta数据不存itemName，返回；如果存在则从flash数据中查找是否存在itemName //
// 2-根据itemName及其地址，从flash中读取数据拼接到itemData //
// 为了避免大量的数据搬移，应该是乱序page number存储；对于不再适用的page，应该做erase处理，确保magicnumber无效 //
bool EctBus::foeReadData(const uint16_t slaveId, const std::string &objName, const uint32_t objType, std::vector<uint8_t> &objData)
{
    if (objName.size() > MAX_NAME_SIZE)
        return false;

    Slave *slv = getSlave(slaveId);
    if (slv == nullptr)
    {
        return false;
    }

    // get dataSize of item, and allocate memory
    std::vector<FoeFlashItem *> itemList = slv->getFlashItemList(objName);
    if (itemList.size() == 0)
    {
        return false;
    }
    objData.clear();
    objData.resize(slv->getFlashObjectSize(objName), 0);

    // int32_t dataOffset = 0;
    for (auto item : itemList)
    {
        if ((item->dataOffset + item->dataSize) > objData.size()) // 当前page的数据偏移地址+数据大小，不能超过objData的大小 //
            return false;
        foeReadPage(slaveId, item->pageIndex, item->dataSize, &objData.data()[item->dataOffset]);
        // dataOffset+=item->dataSize;
    }
    // if (dataOffset!=slv->getObjectSize(objName)) {
    //     // TODO: check
    // }
    return true;
}

// 根据关键字访问，如"device","adapter01"
// 查询是否有item，如果有item，判断size是否够用（page_count），如果不够用，咋办？是不是应该按照page方式链接？
bool EctBus::foeWriteData(const uint16_t slaveId, const std::string &objName, const uint32_t objType, std::vector<uint8_t> &objData)
{
    if (objName.size() > MAX_NAME_SIZE)
    {
        return false;
    }

    //    if (objData.size()<objSize) {
    //        return false;
    //    }

    Slave *slv = getSlave(slaveId);  // getslavelist()[slaveId];
    if (slv == nullptr)
    {
        return false;
    }

    slv->invalidFlashObject(objName);

    // if not exists! allocate
    // allocate pages for obj
    std::vector<FoeFlashItem *> itemList;
    itemList = slv->addFlashObjectToItemList(objName, objType, objData.size());
    // check if allocate success
    if (itemList.size() == 0)
    {
        return false;
    }

    int32_t dataOffset = 0;
    for (auto item : itemList)
    {
        // int pageAddr = ADDR_FLASH_BASE+item->pageIndex*SIZE_FLASH_PAGE;
        if ((item->dataOffset + item->dataSize) > objData.size())
        {
            return false;
        }
        foeWritePage(slaveId, item->pageIndex, item->dataSize, &objData.data()[item->dataOffset]);
        // TODO: check
        dataOffset += item->dataSize;
    }

    // currently, the meta is updated of new itemList, so write it to flash
    if (!saveSlaveMetaData(slaveId))
    {
        // recovery
        slv->invalidFlashObject(objName);
        return false;
    }
    return true;
}

///
/// \brief EctBus::loadSlaveMetaData: if page0 is invalid, create and save it!
/// \param slaveId
/// \return
///
bool EctBus::loadSlaveMetaData(uint16_t slaveId)
{
    Slave *slv = getSlave(slaveId);  // getslavelist()[slaveId]; // TODO
    if (slv == nullptr)
        return false;

    // slv->foeFlashInitialized(false);

    // read meta from page 0
    uint32_t pageIndex = INDX_FLASH_META;
    int32_t pageSize = SIZE_FLASH_PAGE;
    // std::vector<uint8_t> pageData(SIZE_FLASH_PAGE, 0);
    foeReadPage(slaveId, pageIndex, pageSize, &slv->meta().data()[0]);
    if (pageSize != SIZE_FLASH_PAGE)
    {
        // TODO:
    }
    // check meta
    if (slv->checkMeta())
    {
        return true;
    }

    // need new creation of meta data
    slv->createMeta();
    // save meta, keep consistent between flash and slave.metaPage
    if (!saveSlaveMetaData(slaveId))
        return false;
    return true;
}

bool EctBus::saveSlaveMetaData(uint16_t slaveId)
{
    Slave *slv = getSlave(slaveId);  // getslavelist()[slaveId]; // TODO
    if (slv == nullptr)
        return false;

    // check meta is valid
    if (!slv->checkMeta())
    {
        return false;
    }

    // write meta to page 0
    uint32_t pageIndex = INDX_FLASH_META;
    int32_t pageSize = SIZE_FLASH_PAGE;
    foeWritePage(slaveId, INDX_FLASH_META, pageSize, &slv->meta().data()[0]);
    // TODO: check result
    return true;
}

///
/// \brief EctBus::readObjectList
/// \param slaveId
/// \param objType
/// \return <objName, objSize>
///
std::map<std::string, std::pair<uint32_t, int32_t>> EctBus::readFlashObjectMap(const uint16_t slaveId, const uint32_t objType)
{
    std::map<std::string, std::pair<uint32_t, int32_t>> objMap;  // (name,(type, size))
    Slave *slv = getSlave(slaveId);  // getslavelist()[slaveId]; // TODO
    if (slv == nullptr)
    {
        return objMap;
    }

    if (!loadSlaveMetaData(slaveId))
    {
        return objMap;
    }
    objMap = slv->getFlashObjectMap();
    if (objType == 0)
    {
        return objMap;
    }
    // remove redundant item with same type
    for (auto obj : objMap)
    {
        if (obj.second.first != objType)
        {
            objMap.erase(obj.first);
        }
    }
    return objMap;
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
#if 0
bool EctBus::getServiceItem(uint16_t deviceId, std::string objName, std::string itemName, ServiceItem& item)
{
    if (objName.size() > MAX_NAME_SIZE || itemName.size()>MAX_NAME_SIZE)
    {
        return false;
    }

    Slave *slv = getSlave(deviceId);
    if (slv == nullptr)
    {
        return false;
    }

    ServiceItem item = deviceMap[deviceId].at(objName).;

    mBus->getServiceItem(deviceId, objName, itemName, item);
    return 0;
}


bool EctBus::sdoWriteObjectData(const uint16_t slaveId, const std::string& objName, std::vector<uint8_t>& objData)
{
    if (objName.size() > MAX_NAME_SIZE)
    {
        return false;
    }

    Slave *slv = getSlave(slaveId);  // getslavelist()[slaveId];
    if (slv == nullptr)
    {
        return false;
    }

    PDOEntry& obj = slv->getSdoObject(objName);
    PDOSubEntry obj = slv->getSdoObject(objName, objType);

    return sendSdoWrite(slaveId, obj.index, 1, true, objData);
}

bool EctBus::sdoReadObjectData(const uint16_t slaveId, const std::string& objName, std::vector<uint8_t>& objData)
{
    if (objName.size() > MAX_NAME_SIZE)
        return false;

    Slave *slv = getSlave(slaveId);
    if (slv == nullptr)
    {
        return false;
    }

    ServiceObject& obj = slv->getSdoObject(objName);

    objData.clear();
    objData.resize(obj.objSize, 0);
    return sendSdoRead(slaveId, obj.objIndex, 1, true, objData);
}

bool EctBus::sdoWriteItemData(const uint16_t slaveId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData)
{
    if (objName.size() > MAX_NAME_SIZE)
    {
        return false;
    }

    Slave *slv = getSlave(slaveId);  // getslavelist()[slaveId];
    if (slv == nullptr)
    {
        return false;
    }

    ServiceObject& obj = slv->getSdoObject(objName);
    ServiceItem& item = slv->getSdoObjectItem(objName, itemName);
    // check

    return sendSdoWrite(slaveId, obj.objIndex, item.itemIndex, false, itemData);
}


bool EctBus::sdoReadItemData(const uint16_t slaveId, const std::string& objName, const std::string& itemName, std::vector<uint8_t>& itemData)
{
    if (objName.size() > MAX_NAME_SIZE)
        return false;

    Slave *slv = getSlave(slaveId);
    if (slv == nullptr)
    {
        return false;
    }

    ServiceObject& obj = slv->getSdoObject(objName);
    ServiceItem& item = slv->getSdoObjectItem(objName, itemName);
    // check

    itemData.clear();
    itemData.resize(item.itemSize, 0);
    return sendSdoRead(slaveId, obj.objIndex, item.itemIndex, false, itemData);
}
#endif







#if 0
std::map<std::string, std::pair<uint32_t, int32_t>> EctBus::readSdoObjectMap(const uint16_t slaveId, const uint32_t objType)
{
    std::map<std::string, std::pair<uint32_t, int32_t>> objMap;
    Slave *slv = getSlave(slaveId);
    if (slv == nullptr)
    {
        return objMap;
    }

    // from desc, read all obj
    //TODO: objMap = slv->getSdoObjectMap(objType);

    return objMap;
}



std::vector<FoeFlashItem*> getItemList(uint16_t slaveId, std::string& objName) {
    Slave* slv = getslavelist()[slaveId];  // TODO
    meta = slv->metaData;
    itemList = meta->itemList;
    std::vector<FoeFlashItem*> objItemList;
    for (auto item : itemList) {
        if (item->valid && item->name == objName) {
            objItemList.push_back(item);
        }
    }
    return objItemList;
}


int32_t invalidItemList(uint16_t slaveId, std::vector<FoeFlashItem*>& itemList) {
    Slave* slv = getslavelist()[slaveId];  // TODO
    meta = slv->metaData;
    for (auto item : itemList) {
        memset(&item,0,sizeof(item));
    }
}

///
/// \brief newItemList: find invalid item, fill name/index/...
/// \param slaveId
/// \param objName
/// \param dataSize
/// \return
///
std::vector<FoeFlashItem*> newItemList(uint16_t slaveId, std::string& objName, int32_t dataSize) {
    std::vector<FoeFlashItem*> itemList;
    Slave* slv = getslavelist()[slaveId];  // TODO
    meta = slv->metaData;
    itemList = meta->itemList;
    std::vector<FoeFlashItem*> objItemList;
    int32_t remainSize = dataSize;
    int32_t pageIndex = 1;  // start index of item page
    for (int32_t pageIndex =1; pageIndex<COUNT_FLASH_PAGE; pageIndex++) {  // page:1~63
    //for (item : itemList) {
        FoeFlashItem* item = itemList.at(pageIndex);  // start from 1?
        if ((remainSize>0) && (item->valid == false)) {
            int32_t pageSize = (remainSize>SIZE_FLASH_PAGE) ? (SIZE_FLASH_PAGE) : (remainSize);
            memcpy(&item->name[0], objName.data(), objName.size());  // check last '\000'
            item->indexInd = pageIndex;
            item->baseAddr = ADDR_FLASH_BASE+item->pageIndex*SIZE_FLASH_PAGE;
            item->dataSize = pageSize;
            item->magicNumber = 0;
            item->valid = true;
            remainSize -= pageSize;
            objItemList.push_back(item);
        }
        if (remainSize==0)
            break;
        pageIndex++;
    }
    return objItemList;
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
bool EctBus::foe(int slave, const char* filename, unsigned char* data, int length)
{
    //using namespace std;
    //unsigned char* data;
    //int length;
    //int read_size;

    // ecx_FOEwrite_file是阿伟新实现的函数 //
    // TODO
    //int wkc = 0;
    int wkc = ecx_FOEwrite_file(&ecx_context_ins, slave, filename, 0x01234567, length, data, EC_TIMEOUTMON * 10);
    std::cout << "the wkc is " << wkc << std::endl;
    std::cout << "the filename is " << filename << std::endl;
    std::cout << "the length is " << length << std::endl;

//    if (data)
//        free(data);

    if (wkc == 1)
        return true;
    else
        return false;
}
#else
bool EctBus::foe(int slave, const char *filename)
{
    using namespace std;
    unsigned char *data;
    int length;
    int read_size;

    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        std::cout << "can't open the file !!! " << std::endl;
        return false;
    }
    fseek(fp, 0L, SEEK_END);
    length = ftell(fp);
    fclose(fp);

    fp = fopen(filename, "rb");
    if (!fp)
        return false;
    data = (unsigned char *)malloc(length + 32);
    if (!data)
    {
        std::cout << "the file is empty!!!" << std::endl;
        return false;
    }
    // fseek(fp,0L,SEEK_SET);
    read_size = fread(data, 1, length, fp);
    if (read_size != length)
        return false;
    fclose(fp);

    // ecx_FOEwrite_file是阿伟新实现的函数 //
    // TODO
    //int wkc = 0;
    int wkc = ecx_FOEwrite_file(&ecx_context_ins,slave,filename,0x01234567,length,data, EC_TIMEOUTMON * 10);
    std::cout << "the wkc is " << wkc << std::endl;
    std::cout << "the filename is " << filename << std::endl;
    std::cout << "the length is " << length << std::endl;

    if (data)
        free(data);

    if (wkc == 1)
        return true;
    else
        return false;
}
#endif


#if 0
// yinwenbo
// check req.size<sdo.size
// slavenum:from 1 to n
int EctBus::sendSDO(uint16_t slaveId, std::string &sdoData)
{
#if 0
    if (slaveId > ec_slavecount)
        return -1;
    else if (sdoData.size() > MAX_SDO_REQ_SIZE) // 需要确认下，ecx_SDOwrite中是支持多次发送的，MCU未必能接收这么多 //
        return -2;
    else
    {
        int send = ecx_SDOwrite(&ecx_context_ins, (slaveId + 1), INDX_SDO_REQ, 1,
                                true, sdoData.size(), sdoData.data(), EC_TIMEOUTSTATE * 3);
        return send;
    }
#endif
    return 0;
}

int EctBus::recvSDO(uint16_t slaveId, std::string &sdoData)
{
#if 0
    sdoData.clear();
    uint8 buf[2048]; // TBD  MAX_SDO_RES_SIZE
    if (slaveId > ec_slavecount)
        return -1;
    else
    {
        int size = 2048; // maybe MAX_SDO_RES_SIZE=64bytes
        int get = ecx_SDOread(&ecx_context_ins, slaveId + 1, INDX_SDO_RES, 1,
                              true, &size, &buf, EC_TIMEOUTSTATE * 3);
        if (size > 0 & size < 2048)
        { // TODO
            sdoData.resize(size);
            memcpy(sdoData.data(), &buf, size);
            return size;
        }
        else
            return -2; // bad size
    }
#endif
    return 0;
}
#endif

// yinwenbo
// check req.size<sdo.size
// slavenum:from 1 to n
// int EctBus::sendSdo(uint8 slavenum, uint8_t*data, int32_t size)
//{
//    if(slavenum >ec_slavecount)
//        return -1;
//    else if (size>MAX_SDO_REQ_SIZE)
//        return -2;
//    else
//    {
//        int send =ecx_SDOwrite(&ecx_context_ins, (slavenum+1), INDX_SDO_REQ, 1,
//                                true, size, data, EC_TIMEOUTSTATE * 3);
//        return send;
//    }
//}

// slavenum:from 1 to n
// int EctBus::getSdo(uint8 slavenum, uint8_t*data, int32_t& size)
//{
//    //QByteArray resp;
//    //resp.clear();
//    uint8 buf[2048];  // TBD  MAX_SDO_RES_SIZE
//    if(slavenum >ec_slavecount)
//        return -1;
//    else
//    {
//        int size=2048;  // maybe MAX_SDO_RES_SIZE=64bytes
//        int get =ecx_SDOread(&ecx_context_ins,slavenum+1, INDX_SDO_RES,1,
//                              true, &size, &buf, EC_TIMEOUTSTATE * 3);
//        if (size>0 & size<2048){  // TODO
//            //resp.resize(size);
//            memcpy(data, &buf, size);
//            return size;
//        } else
//            return -2;  // bad size
//    }
//}

int EctBus::getslavenum()
{
    return ec_slavecount;
}

void EctBus::testwd()
{
    // 20210801
    // 把这个改了之后就相当都把watchdog开了，
    // 如果不是debug一个断点一个断点的跑的话其实开了也没有任何影响
    // 过程数据看门狗无效：0x0420寄存器写0。
    // PDI看门狗无效：0x410寄存器写0
    // 参考书page63
    uint16 aa = 0x0000;
    int wkc = ecx_BWR(&ecx_port, 0x0000, 0x0420, 2, &aa, EC_TIMEOUTRET);
}

// 获取特定slave列表的PDO状态字状态 //
bool EctBus::checkStatus(std::vector<uint16_t> slaveIdList, uint8_t expectedStatus)
{
    std::vector<uint8_t> expectedData(expectedStatus);
    //expectedData.push_back(expectedStatus);
    for (uint16_t id : slaveIdList)
    {
        // TODO: check valid of slave
        if (m_slaveMap.count(id)==0)
            return false;

        if (m_slaveMap[id]->getPDO(INDX_resp_syncPDO, 1) != expectedData)
            return false;
    }
    return true;
}

bool EctBus::checkAllStatus(uint8_t expectedStatus)
{
    std::vector<uint8_t> expectedData(expectedStatus);
    //expectedData.push_back(expectedStatus);
    for (auto slv : m_slaveMap)
    {
        if (slv.second->getPDO(INDX_resp_syncPDO, 1) != expectedData)
            return false;
    }
    return true;
}

bool EctBus::setAllStatus(uint8_t expectedStatus)
{
    std::vector<uint8_t> data;
    data.push_back(expectedStatus);
    for (auto slv : m_slaveMap)
    {
        slv.second->setPDO(INDX_req_syncPDO, data);
    }
    return true;
}

bool EctBus::canAcceptPDO()
{
    for (auto slv : m_slaveMap)
    {
        if (!slv.second->canAcceptPDO())
            return false;
    }
    return true;
}

// 有个问题：不是所有的device需要设置stmus pattern
// 没有stimus pattern的device，需要设置其dout
// dout需要维持
bool EctBus::setReqPDO(std::map<uint16_t, uint16_t> &reqMode, std::map<uint16_t, std::vector<uint8_t>> &reqDout, std::map<uint16_t, std::vector<uint8_t>> &reqData)
{
    for (auto mode : reqMode)
    {
        if (m_slaveMap.count(mode.first)>0)  // TODO:
        {
            m_slaveMap[mode.first]->setPDO(INDX_SYNC_PDO, mode.second);
        }
    }

    for (auto dout : reqDout)
    {
        if (m_slaveMap.count(dout.first)>0)  // TODO:
        {
            m_slaveMap[dout.first]->setPDO(INDX_STAT_PDO, dout.second);
        }
    }

    for (auto d : reqData)
    {
        if (m_slaveMap.count(d.first)>0)
        {
            // TODO: can't use slavelist as slave manager!!!
            m_slaveMap[d.first]->setPDO(INDX_DATA_PDO, d.second);
        }

        // TODO: for test: set sync
        // m_slaveMap[d.first]->setSyncPDO();
    }
    return true;
}

// 需要所有的slave都ready
// 支持删除slave的机制，那么用slavelist就不合适了！用map比较好 //
// 需要根据有效的slave集合来设计
bool EctBus::isRspReady()
{
    for (auto slv : m_slaveMap)
    {
        if (!slv.second->isRspReady())
            return false;
    }
    return true;
}

bool EctBus::setSyncPDO()
{
    for (auto slv : m_slaveMap)
    {
        slv.second->setSyncPDO();
    }
    return true;
}

bool EctBus::resetSyncPDO()
{
    for (auto slv : m_slaveMap)
    {
        slv.second->resetSyncPDO();
    }

    //if (updateBus())
    {
        return true;
    }
    return true;
}

bool EctBus::getRspPDO(std::map<uint16_t, uint16_t> &rspMode, std::map<uint16_t, std::vector<uint8_t>> &rspDin, std::map<uint16_t, std::vector<uint8_t>> &rspData, std::map<uint16_t, std::vector<uint8_t>> &rspMeasure)
{
    rspMode.clear();
    rspDin.clear();
    rspData.clear();
    rspMeasure.clear();  // SDO相关的数据放到后面读取？ //

    for (auto slv : m_slaveMap)
    {
        std::vector<uint8_t> mode = slv.second->getPDO(INDX_SYNC_PDO, 2);
        uint16_t modeu16 = (mode.data()[1]<<8) | mode.data()[0];
        rspMode.insert({slv.second->getId(), modeu16});
        rspDin.insert({slv.second->getId(), slv.second->getPDO(INDX_STAT_PDO, 2)});  // TODO

        // 根据modeu16读取相关的数据到响应 //
        if (modeu16 & 0x0380) {  // getio
            rspData.insert({slv.second->getId(), slv.second->getPDO(INDX_DATA_PDO, slv.second->getRspPdoSize())}); // TODO
        }

        // 可以用PDO传输Measure，容量不大，每次只能传输一个16bit数据 //
        // TODO: 从SDO读取测量值 //
#if 1
        if (modeu16 & 0x0C00) {  // getmeasure
            //rspMeasure.insert({slv.second->getId(), slv.second->getSDO(INDX_DATA_PDO, slv.second->getRspPdoSize())});
            rspMeasure.insert({ slv.second->getId(), slv.second->getPDO(INDX_DATA_PDO, slv.second->getRspPdoSize()) });
        }
#endif

    }
    return true;
}












// boolean printSDO = FALSE;
// boolean printMAP = FALSE;
char usdo[128];
char hstr[1024];

// no use?
char* EctBus::dtype2string(uint16 dtype)
{
    switch (dtype)
    {
    case ECT_BOOLEAN:
        sprintf(hstr, "BOOLEAN");
        break;
    case ECT_INTEGER8:
        sprintf(hstr, "INTEGER8");
        break;
    case ECT_INTEGER16:
        sprintf(hstr, "INTEGER16");
        break;
    case ECT_INTEGER32:
        sprintf(hstr, "INTEGER32");
        break;
    case ECT_INTEGER24:
        sprintf(hstr, "INTEGER24");
        break;
    case ECT_INTEGER64:
        sprintf(hstr, "INTEGER64");
        break;
    case ECT_UNSIGNED8:
        sprintf(hstr, "UNSIGNED8");
        break;
    case ECT_UNSIGNED16:
        sprintf(hstr, "UNSIGNED16");
        break;
    case ECT_UNSIGNED32:
        sprintf(hstr, "UNSIGNED32");
        break;
    case ECT_UNSIGNED24:
        sprintf(hstr, "UNSIGNED24");
        break;
    case ECT_UNSIGNED64:
        sprintf(hstr, "UNSIGNED64");
        break;
    case ECT_REAL32:
        sprintf(hstr, "REAL32");
        break;
    case ECT_REAL64:
        sprintf(hstr, "REAL64");
        break;
    case ECT_BIT1:
        sprintf(hstr, "BIT1");
        break;
    case ECT_BIT2:
        sprintf(hstr, "BIT2");
        break;
    case ECT_BIT3:
        sprintf(hstr, "BIT3");
        break;
    case ECT_BIT4:
        sprintf(hstr, "BIT4");
        break;
    case ECT_BIT5:
        sprintf(hstr, "BIT5");
        break;
    case ECT_BIT6:
        sprintf(hstr, "BIT6");
        break;
    case ECT_BIT7:
        sprintf(hstr, "BIT7");
        break;
    case ECT_BIT8:
        sprintf(hstr, "BIT8");
        break;
    case ECT_VISIBLE_STRING:
        sprintf(hstr, "VISIBLE_STRING");
        break;
    case ECT_OCTET_STRING:
        sprintf(hstr, "OCTET_STRING");
        break;
    default:
        sprintf(hstr, "Type 0x%4.4X", dtype);
    }
    return hstr;
}

char* EctBus::SDO2string(uint16 slave, uint16 index, uint8 subidx, uint16 dtype)
{
    int l = sizeof(usdo) - 1, i;
    uint8 *u8;
    int8 *i8;
    uint16 *u16;
    int16 *i16;
    uint32 *u32;
    int32 *i32;
    uint64 *u64;
    int64 *i64;
    float *sr;
    double *dr;
    char es[32];

    memset(&usdo, 0, 128);
    // 读取值 //
    ecx_SDOread(&ecx_context_ins, slave, index, subidx, FALSE, &l, &usdo, EC_TIMEOUTRXM);
    if (EcatError)
    {
        return ec_elist2string();
    }
    else
    {
        switch (dtype)
        {
        case ECT_BOOLEAN:
            u8 = (uint8 *)&usdo[0];
            if (*u8)
                sprintf(hstr, "TRUE");
            else
                sprintf(hstr, "FALSE");
            break;
        case ECT_INTEGER8:
            i8 = (int8 *)&usdo[0];
            sprintf(hstr, "0x%2.2x %d", *i8, *i8);
            break;
        case ECT_INTEGER16:
            i16 = (int16 *)&usdo[0];
            sprintf(hstr, "0x%4.4x %d", *i16, *i16);
            break;
        case ECT_INTEGER32:
        case ECT_INTEGER24:
            i32 = (int32 *)&usdo[0];
            sprintf(hstr, "0x%8.8x %d", *i32, *i32);
            break;
        // case ECT_INTEGER64:
        // i64 = (int64*) &usdo[0];
        // sprintf(hstr, "0x%16.16"PRIx64" %"PRId64, *i64, *i64);
        // break;
        case ECT_UNSIGNED8:
            u8 = (uint8 *)&usdo[0];
            sprintf(hstr, "0x%2.2x %u", *u8, *u8);
            break;
        case ECT_UNSIGNED16:
            u16 = (uint16 *)&usdo[0];
            sprintf(hstr, "0x%4.4x %u", *u16, *u16);
            break;
        case ECT_UNSIGNED32:
        case ECT_UNSIGNED24:
            u32 = (uint32 *)&usdo[0];
            sprintf(hstr, "0x%8.8x %u", *u32, *u32);
            break;
        // case ECT_UNSIGNED64:
        // u64 = (uint64*) &usdo[0];
        // sprintf(hstr, "0x%16.16"PRIx64" %"PRIu64, *u64, *u64);
        // break;
        case ECT_REAL32:
            sr = (float *)&usdo[0];
            sprintf(hstr, "%f", *sr);
            break;
        case ECT_REAL64:
            dr = (double *)&usdo[0];
            sprintf(hstr, "%f", *dr);
            break;
        case ECT_BIT1:
        case ECT_BIT2:
        case ECT_BIT3:
        case ECT_BIT4:
        case ECT_BIT5:
        case ECT_BIT6:
        case ECT_BIT7:
        case ECT_BIT8:
            u8 = (uint8 *)&usdo[0];
            sprintf(hstr, "0x%x", *u8);
            break;
        case ECT_VISIBLE_STRING:
            strcpy(hstr, usdo);
            break;
        case ECT_OCTET_STRING:
            hstr[0] = 0x00;
            for (i = 0; i < l; i++)
            {
                sprintf(es, "0x%2.2x ", usdo[i]);
                strcat(hstr, es);
            }
            break;
        default:
            sprintf(hstr, "Unknown type");
        }
        return hstr;
    }
}

// Generate Object Dictionary
// 需要能区分Variable, Array, Record 类型, 怎么区分呢？？ //
#if 1
std::map<std::string, ServiceObject> EctBus::generateObjectDict(uint16_t slaveId)
{
    //PDODescription pdoDescription;

    std::map<std::string, ServiceObject> serviceObjDict;

    ec_OElistt OElist;
    ec_ODlistt ODlist;
    ODlist.Entries = 0;
    memset(&ODlist, 0, sizeof(ODlist));

    uint16_t totalOffsetBits_input = 0;
    uint16_t totalOffsetBits_output = 0;

    if (ecx_readODlist(&ecx_context_ins, slaveId, &ODlist))
    {
        for (unsigned int i = 0; i < ODlist.Entries; i++)
        {
            ecx_readODdescription(&ecx_context_ins, i, &ODlist);
            while (EcatError)  // or if ?
                printf("EcatError: %s", ec_elist2string());

            //printf(" Index: %4.4x Datatype: %4.4x Objectcode: %2.2x Name: %s\n",
            //       ODlist.Index[i], ODlist.DataType[i], ODlist.ObjectCode[i], ODlist.Name[i]);

            // PDO的索引范围 //
            if (ODlist.Index[i] >= 0x6000 && ODlist.Index[i] < 0xA000)
            {
                ServiceObject serviceObj;
                serviceObj.objIndex = ODlist.Index[i];
                serviceObj.objName = ODlist.Name[i];
                serviceObj.objData.clear();
                serviceObj.objSize = 0;
                serviceObj.objType = static_cast<ec_datatype>(ODlist.DataType[i]);;
#if 0
                PDOEntry pdoE;
                pdoE.index = ODlist.Index[i];
                pdoE.name = ODlist.Name[i];
#endif
                memset(&OElist, 0, sizeof(OElist));
                ecx_readOE(&ecx_context_ins, i, &ODlist, &OElist);
                while (EcatError)
                    printf("EcatError: %s", ec_elist2string());

                //EntryType direction = ODlist.Index[i] < 0x7000 ? Output : Input;
                EntryType direction;
                if ((ODlist.Index[i] >= 0x6000) && (ODlist.Index[i] < 0x7000)) {
                    direction = Output;
                } else if ((ODlist.Index[i] >= 0x7000) && (ODlist.Index[i] < 0x8000)) {
                    direction = Input;
                } else if ((ODlist.Index[i] >= 0x8000) && (ODlist.Index[i] < 0x9000)) {
                    direction = Output;
                } else {
                    direction = Input;
                }

                uint16_t id = 0;

                // 要区分不同的subIndex, =0的subIndex需要根据其enties的数量计算byte数量，填充item.size //
                uint16_t offsetBytes = 0;
                bool isSubIndexArray = false;  // 标记是否是ARRAY类型 //
                uint16_t u16SubIndex0Value = 0;  // 用于记录subindex0的value，该value表示sub entries的数目，如果ODlist.MaxSub[i]==2，说明是ARRAY类型 //
                for (uint16_t j = 0; j < (ODlist.MaxSub[i] + 1); j++)  // ODlist.MaxSub[i]是指最大sub index的值，所以要加1控制循环边界 //
                {
                    // just for printing, may crash debugging process
                    if ((OElist.DataType[j] > 0) && (OElist.BitLength[j] > 0))
                    {
                        ;
                        //printf("  Sub: %2.2x Datatype: %4.4x Bitlength: %4.4x Obj.access: %4.4x Name: %s\n",
                        //       j, OElist.DataType[j], OElist.BitLength[j], OElist.ObjAccess[j], OElist.Name[j]);

                        // 首个subIndex, 读取值，value表示个数？ //
                        if ((OElist.ObjAccess[j] & 0x0007))
                        {
                            ;// printf("          Value :%s\n", SDO2string(slaveId, ODlist.Index[i], j, OElist.DataType[j]));
                        }
                    }

                    
                    bool isSubIndex000Entry = std::string(OElist.Name[j]).rfind("SubIndex 000", 0) == 0;
                    bool isPadding = std::string(OElist.Name[j]).rfind("SubIndex", 0) == 0;

                    //
                    //isSubIndexArray = false;  // init
                    if (isSubIndex000Entry) {
                        ServiceItem item;
                        item.itemIndex = j;  // id;
                        item.itemName = OElist.Name[j];
                        item.itemType = static_cast<ec_datatype>(OElist.DataType[j]);
                        item.itemSize = (OElist.BitLength[j]/8);  // byte count
                        item.itemOffset = 0;
                        // read value, the number entries //
                        item.itemData.clear();
                        item.itemData.resize(item.itemSize, 0);
                        // 读取值 //
                        int size=item.itemSize;
                        ecx_SDOread(&ecx_context_ins, slaveId, ODlist.Index[i], j, FALSE, &size, &item.itemData.data()[0], EC_TIMEOUTRXM);
                        if (EcatError)
                        {
                            TERROR("EcatError reading sub index 0");
                        }
                        if (size != item.itemSize) {
                            TERROR("item.itemData size:{} does't equal with size:{}", item.itemData.size(), size);
                        }

                        // item.itemData.data表示sub index0后面的entiries数目 //
                        uint8_t entriesCount = item.itemData[0];  // 只有8bit //
                        if ((entriesCount>1) && (ODlist.MaxSub[i]==1)) {  // ODlist.MaxSub[i]>2说明有多个entries //
                            isSubIndexArray = true;
                            u16SubIndex0Value = entriesCount;  // 实际的ARRAY元素数目 //
                        } else {
                            isSubIndexArray = false;
                            u16SubIndex0Value = entriesCount;  // entriesCount==(ODlist.MaxSub[i]-1)
                        }
                        offsetBytes = 0;
                        serviceObj.entryItem = item;
                    }
                    // 非subIndex=0的item //
                    else if (!isSubIndex000Entry)
                    {
                        //if (!isPadding)
                        {
                            ServiceItem item;
                            item.itemIndex = j;  // id;  // TODO: subindex = (id+1) ARRAY使用itemOffset访问数据 //
                            item.itemName = OElist.Name[j];  // ARRAY类型是“SubIndex 001”，软件不好判断名字 //
                            item.itemType = static_cast<ec_datatype>(OElist.DataType[j]);
                            // 怎么确定ARRAY类型？ //
                            if (isSubIndexArray) {
                                item.itemSize = (OElist.BitLength[j]/8)*u16SubIndex0Value;  // ARRAY类型 //
                                item.itemOffset = 0;  // ARRAY类型应该只有一个数据块 //
                            } else {
                                item.itemSize = (OElist.BitLength[j]/8);  // byte count
                                item.itemOffset = offsetBytes;  // id*sizeof(uint16_t);  //TODO:缺省用uint16_t类型的ARRAY  //
                                offsetBytes += item.itemSize;
                            }
                            serviceObj.objSize += item.itemSize;  // total size of obj //

                            // read value of this entry
                            item.itemData.clear();
                            item.itemData.resize(item.itemSize, 0);
                            // 读取值 //
                            int size=item.itemSize;
                            if (isSubIndexArray) {
                                ecx_SDOread(&ecx_context_ins, slaveId, ODlist.Index[i], 1, TRUE, &size, &item.itemData.data()[0], EC_TIMEOUTRXM);
                            } else {
                                ecx_SDOread(&ecx_context_ins, slaveId, ODlist.Index[i], j, FALSE, &size, &item.itemData.data()[0], EC_TIMEOUTRXM);
                            }
                            if (EcatError)
                            {
                                // TODO: error occurs here! to check!!!!!!!!!!!!!!!!!!!!!!
                                TERROR("EcatError reading sub index 1-x");
                            }
                            if (size != item.itemSize) {
                                TERROR("item.itemData size:{} does't equal with size:{}", item.itemData.size(), size);
                            }

                            serviceObj.itemMap.insert({OElist.Name[j], item});
#if 0
                            pdoE.entries.emplace_back(PDOSubEntry{OElist.Name[j],
                                                                  id,  // subindex, start from 0
                                                                  static_cast<ec_datatype>(OElist.DataType[j]),
                                                                  OElist.BitLength[j],
                                                                  direction,
                                                                  direction == Output ? totalOffsetBits_output : totalOffsetBits_input});
                            //                            pdoE.entries.back().hash = PDOSubEntry::PDOSubEntryHash{}(slaveId,
                            //                                                                                      direction,
                            //                                                                                      pdoE.index,
                            //                                                                                      id);
#endif
                            id++;
                        }

                        if (direction == Output)
                        {
                            totalOffsetBits_output += OElist.BitLength[j];
                        }
                        else
                        {
                            totalOffsetBits_input += OElist.BitLength[j];
                        }
                    }
                }

                // --------------------------------------------
                // IMPORTANT: SOEM internally calls the entries that will be sent to the slave "Outputs"
                // while in our slave-implementation we call them "Inputs" because the Slave-object represents the Slave itself
                // --------------------------------------------
                serviceObjDict.insert({serviceObj.objName, serviceObj});
#if 0
                if (ODlist.Index[i] >= 0x6000 && ODlist.Index[i] < 0x7000)
                {
                    pdoDescription.sendPDO.insert({pdoE.name, pdoE});
                }
                else if (ODlist.Index[i] >= 0x7000 && ODlist.Index[i] < 0x8000)
                {
                    pdoDescription.recvPDO.insert({pdoE.name, pdoE});
                } else if (ODlist.Index[i] >= 0x8000 && ODlist.Index[i] < 0x9000)
                {
                    pdoDescription.sendSDO.insert({pdoE.name, pdoE});
                }
                else
                {
                    pdoDescription.recvSDO.insert({pdoE.name, pdoE});
                }
#endif
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        std::cerr << "Could not read ODList\n";
    }

    return serviceObjDict;
}
#endif

PDODescription EctBus::createSlavePDODescription(uint16_t slaveId)
{
    PDODescription pdoDescription;

    ec_OElistt OElist;
    ec_ODlistt ODlist;
    ODlist.Entries = 0;
    memset(&ODlist, 0, sizeof(ODlist));

    uint16_t totalOffsetBits_input = 0;
    uint16_t totalOffsetBits_output = 0;

    if (ecx_readODlist(&ecx_context_ins, slaveId, &ODlist))
    {
        for (unsigned int i = 0; i < ODlist.Entries; i++)
        {
            ecx_readODdescription(&ecx_context_ins, i, &ODlist);
            while (EcatError)
                printf("%s", ec_elist2string());
            printf(" Index: %4.4x Datatype: %4.4x Objectcode: %2.2x Name: %s\n",
                   ODlist.Index[i], ODlist.DataType[i], ODlist.ObjectCode[i], ODlist.Name[i]);
            // PDO的索引范围 //
            if (ODlist.Index[i] >= 0x6000 && ODlist.Index[i] < 0xA000)
            {
                PDOEntry pdoE;
                pdoE.index = ODlist.Index[i];
                pdoE.name = ODlist.Name[i];

                memset(&OElist, 0, sizeof(OElist));
                ecx_readOE(&ecx_context_ins, i, &ODlist, &OElist);
                while (EcatError)
                    printf("%s", ec_elist2string());

                //EntryType direction = ODlist.Index[i] < 0x7000 ? Output : Input;
                EntryType direction;
                if ((ODlist.Index[i] >= 0x6000) && (ODlist.Index[i] < 0x7000)) {
                    direction = Output;
                } else if ((ODlist.Index[i] >= 0x7000) && (ODlist.Index[i] < 0x8000)) {
                    direction = Input;
                } else if ((ODlist.Index[i] >= 0x8000) && (ODlist.Index[i] < 0x9000)) {
                    direction = Output;
                } else {
                    direction = Input;
                }

                uint16_t id = 0;

                for (uint16_t j = 0; j < ODlist.MaxSub[i] + 1; j++)
                {
                    if ((OElist.DataType[j] > 0) && (OElist.BitLength[j] > 0))
                    {
                        printf("  Sub: %2.2x Datatype: %4.4x Bitlength: %4.4x Obj.access: %4.4x Name: %s\n",
                               j, OElist.DataType[j], OElist.BitLength[j], OElist.ObjAccess[j], OElist.Name[j]);
                        if ((OElist.ObjAccess[j] & 0x0007))
                        {
                            printf("          Value :%s\n", SDO2string(slaveId, ODlist.Index[i], j, OElist.DataType[j]));
                        }
                    }

                    bool isSubIndex000Entry = std::string(OElist.Name[j]).rfind("SubIndex 000", 0) == 0;
                    bool isPadding = std::string(OElist.Name[j]).rfind("SubIndex", 0) == 0;
                    if (!isSubIndex000Entry)
                    {
                        if (!isPadding)
                        {
                            pdoE.entries.emplace_back(PDOSubEntry{OElist.Name[j],
                                                                  id,  // subindex, start from 0
                                                                  static_cast<ec_datatype>(OElist.DataType[j]),
                                                                  OElist.BitLength[j],
                                                                  direction,
                                                                  direction == Output ? totalOffsetBits_output : totalOffsetBits_input});
                            //                            pdoE.entries.back().hash = PDOSubEntry::PDOSubEntryHash{}(slaveId,
                            //                                                                                      direction,
                            //                                                                                      pdoE.index,
                            //                                                                                      id);
                            id++;
                        }

                        if (direction == Output)
                        {
                            totalOffsetBits_output += OElist.BitLength[j];
                        }
                        else
                        {
                            totalOffsetBits_input += OElist.BitLength[j];
                        }
                    }
                }

                // --------------------------------------------
                // IMPORTANT: SOEM internally calls the entries that will be sent to the slave "Outputs"
                // while in our slave-implementation we call them "Inputs" because the Slave-object represents the Slave itself
                // --------------------------------------------
                if (ODlist.Index[i] >= 0x6000 && ODlist.Index[i] < 0x7000)
                {
                    pdoDescription.sendPDO.insert({pdoE.name, pdoE});
                }
                else if (ODlist.Index[i] >= 0x7000 && ODlist.Index[i] < 0x8000)
                {
                    pdoDescription.recvPDO.insert({pdoE.name, pdoE});
                } else if (ODlist.Index[i] >= 0x8000 && ODlist.Index[i] < 0x9000)
                {
                    pdoDescription.sendSDO.insert({pdoE.name, pdoE});
                }
                else
                {
                    pdoDescription.recvSDO.insert({pdoE.name, pdoE});
                }
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        std::cerr << "Could not read ODList\n";
    }

    return pdoDescription;
}

//
int32_t EctBus::createPDODescription()
{
    for (auto slv : m_slaveMap) {
        Slave* slave = slv.second;
        uint16_t slaveId = slv.second->getId();
        std::map<std::string, ServiceObject> serviceObjDict = generateObjectDict(slaveId);
        serviceObjMapDict.insert({slaveId, serviceObjDict});
        PDODescription desc = createSlavePDODescription(slaveId);
        slave->setDesc(desc);
    }
    return 0;
}


int32_t EctBus::getNumberOfSlaves() {
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    return *ecx_context_ins.slavecount;
}

bool EctBus::sendSdoRead(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, std::vector<uint8_t> &value)
{
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    //Expected length of the string. String needs to be preallocated
    int size = value.size();
    //Store for check at the end
    int expected_size = size;
    //Create buffer with the length of the string
    std::vector<uint8_t> buffer(size,0);
    int wkc = 0;
    {
            std::lock_guard<std::recursive_mutex> guard(contextMutex_);
            wkc = ecx_SDOread(&ecx_context_ins, slave, index, subindex, static_cast<boolean>(completeAccess), &size, &buffer.data()[0], EC_TIMEOUTRXM);
            //Convert read data to a std::string
            value = buffer;  // TODO
    }
    if (wkc <= 0) {
            TERROR("Slave {} : Working counter too low ({}) for reading SDO (ID: 0x{}, subIndex:{}", slave, wkc, index, subindex);
            return false;
    }

    if (size != (int)expected_size) {
            TERROR("Slave {} : Size mismatch (expected size {}, read size {}", slave, size, buffer.size());
            return false;
    }
    return true;
}

bool EctBus::sendSdoWrite(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, std::vector<uint8_t>& value)
{
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    const int32_t size = value.size();
    if (value.size()<=0) {
        return false;
    }
    //uint8_t* dataPtr = &value.data()[0];
    int wkc = 0;
    {
            std::lock_guard<std::recursive_mutex> guard(contextMutex_);
            wkc = ecx_SDOwrite(&ecx_context_ins, slave, index, subindex, static_cast<boolean>(completeAccess), size, &value.data()[0], EC_TIMEOUTRXM);
    }
    if (wkc <= 0) {
            TERROR("Slave {} : Working counter too low ({}) for reading SDO (ID: 0x{}, subIndex:{}", slave, wkc, index, subindex);
            return false;
    }
    return true;
}





// EctBus* getInstance()
//{
//     printf("EctBus::getInstance\n");
//     //static SimTbox instance;
//     //return &instance;
//     //return new SimTbox();
// #if 0
//     if (EctBus::instance==nullptr) {

//    }
//    return nullptr;
// #endif
//    return new EctBus();
//}

// int add(int a, int b)
//{
//     return a+b;
// }

#if 0
EctBus::EctBus(const std::string& name) : name_(name), wkc_(0) {
    // Initialize all SOEM context data pointers that are not used with null.
    ecatContext_.elist->head = 0;
    ecatContext_.elist->tail = 0;
    ecatContext_.port->stack.sock = nullptr;
    ecatContext_.port->stack.txbuf = nullptr;
    ecatContext_.port->stack.txbuflength = nullptr;
    ecatContext_.port->stack.tempbuf = nullptr;
    ecatContext_.port->stack.rxbuf = nullptr;
    ecatContext_.port->stack.rxbufstat = nullptr;
    ecatContext_.port->stack.rxsa = nullptr;
    ecatContext_.port->redport = nullptr;
    //  ecatContext_.idxstack->data = nullptr; // This does not compile since SOEM uses a fixed size array of void pointers.
    ecatContext_.FOEhook = nullptr;
}

bool EctBus::busIsAvailable(const std::string& name) {
    ec_adaptert* adapter = ec_find_adapters();
    while (adapter != nullptr) {
        if (name == std::string(adapter->name)) {
            return true;
        }
        adapter = adapter->next;
    }
    return false;
}

void EctBus::printAvailableBusses() {
    MELO_INFO_STREAM("Available adapters:");
    ec_adaptert* adapter = ec_find_adapters();
    while (adapter != nullptr) {
        MELO_INFO_STREAM("- Name: '" << adapter->name << "', description: '" << adapter->desc << "'");
        adapter = adapter->next;
    }
}

bool EctBus::busIsAvailable() const { return busIsAvailable(name_); }

int EctBus::getNumberOfSlaves() const {
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    return *ecatContext_.slavecount;
}

bool EctBus::addSlave(const EthercatSlaveBasePtr& slave) {
    for (const auto& existingSlave : slaves_) {
        if (slave->getAddress() == existingSlave->getAddress()) {
            MELO_ERROR_STREAM("[" << getName() << "] "
                                  << "Slave '" << existingSlave->getName() << "' and slave '" << slave->getName()
                                  << "' have identical addresses (" << slave->getAddress() << ").");
            return false;
        }
    }

    slaves_.push_back(slave);
    return true;
}

bool EctBus::startup(const bool sizeCheck) {
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    /*
   * Followed by start of the application we need to set up the NIC to be used as
   * EtherCAT Ethernet interface. In a simple setup we call ec_init(ifname) and if
   * SOEM comes with support for cable redundancy we call ec_init_redundant that
   * will open a second port as backup. You can send NULL as ifname if you have a
   * dedicated NIC selected in the nicdrv.c. It returns >0 if succeeded.
   */
    if (!busIsAvailable()) {
        MELO_ERROR_STREAM("[" << getName() << "] "
                              << "Bus is not available.");
        printAvailableBusses();
        return false;
    }
    if (ecx_init(&ecatContext_, name_.c_str()) <= 0) {
        MELO_ERROR_STREAM("[" << getName() << "] "
                              << "No socket connection. Execute as root.");
        return false;
    }

    // Initialize SOEM.
    // Note: ecx_config_init(..) requests the slaves to go to PRE-OP.
    for (unsigned int retry = 0; retry <= ecatConfigMaxRetries_; retry++) {
        if (ecx_config_init(&ecatContext_, FALSE) > 0) {
            // Successful initialization.
            break;
        } else if (retry == ecatConfigMaxRetries_) {
            // Too many failed attempts.
            MELO_ERROR_STREAM("[" << getName() << "] "
                                  << "No slaves have been found.");
            return false;
        }
        // Sleep and retry.
        soem_interface::threadSleep(ecatConfigRetrySleep_);
        MELO_INFO_STREAM("No slaves have been found, retrying " << retry + 1 << "/" << ecatConfigMaxRetries_ << " ...");
    }

    // Print the slaves which have been detected.
    MELO_INFO_STREAM("The following " << getNumberOfSlaves() << " slaves have been found and configured:");
    for (int slave = 1; slave <= getNumberOfSlaves(); slave++) {
        MELO_INFO_STREAM("Address: " << slave << " - Name: '" << std::string(ecatContext_.slavelist[slave].name) << "'");
    }

    // Check if the given slave addresses are valid.
    bool slaveAddressesAreOk = true;
    for (const auto& slave : slaves_) {
        auto address = static_cast<int>(slave->getAddress());
        if (address == 0) {
            MELO_ERROR_STREAM("[" << getName() << "] "
                                  << "Slave '" << slave->getName() << "': Invalid address " << address << ".");
            slaveAddressesAreOk = false;
        }
        if (address > getNumberOfSlaves()) {
            MELO_ERROR_STREAM("[" << getName() << "] "
                                  << "Slave '" << slave->getName() << "': Invalid address " << address << ", "
                                  << "only " << getNumberOfSlaves() << " slave(s) found.");
            slaveAddressesAreOk = false;
        }
    }
    if (!slaveAddressesAreOk) {
        return false;
    }

    // Disable symmetrical transfers.
    ecatContext_.grouplist[0].blockLRW = 1;

    // Initialize the communication interfaces of all slaves.
    for (auto& slave : slaves_) {
        if (!slave->startup()) {
            MELO_ERROR_STREAM("[" << getName() << "] "
                                  << "Slave '" << slave->getName() << "' was not initialized successfully.");
            return false;
        }
    }

    // Set up the communication IO mapping.
    // Note: ecx_config_map_group(..) requests the slaves to go to SAFE-OP.
    ecx_config_map_group(&ecatContext_, &ioMap_, 0);

    // Check if the size of the IO mapping fits our slaves.
    bool ioMapIsOk = true;
    // do this check only if 'sizeCheck' is true
    if (sizeCheck) {
        for (const auto& slave : slaves_) {
            const EthercatSlaveBase::PdoInfo pdoInfo = slave->getCurrentPdoInfo();
            if (pdoInfo.rxPdoSize_ != ecatContext_.slavelist[slave->getAddress()].Obytes) {
                MELO_ERROR_STREAM("[" << getName() << "] "
                                      << "RxPDO size mismatch: The slave '" << slave->getName() << "' expects a size of " << pdoInfo.rxPdoSize_
                                      << " bytes but the slave found at its address " << slave->getAddress() << " requests "
                                      << ecatContext_.slavelist[slave->getAddress()].Obytes << " bytes).");
                ioMapIsOk = false;
            }
            if (pdoInfo.txPdoSize_ != ecatContext_.slavelist[slave->getAddress()].Ibytes) {
                MELO_ERROR_STREAM("[" << getName() << "] "
                                      << "TxPDO size mismatch: The slave '" << slave->getName() << "' expects a size of " << pdoInfo.txPdoSize_
                                      << " bytes but the slave found at its address " << slave->getAddress() << " requests "
                                      << ecatContext_.slavelist[slave->getAddress()].Ibytes << " bytes).");
                ioMapIsOk = false;
            }
        }
    }
    if (!ioMapIsOk) {
        return false;
    }

    // Initialize the memory with zeroes.
    for (int slave = 1; slave <= getNumberOfSlaves(); slave++) {
        memset(ecatContext_.slavelist[slave].inputs, 0, ecatContext_.slavelist[slave].Ibytes);
        memset(ecatContext_.slavelist[slave].outputs, 0, ecatContext_.slavelist[slave].Obytes);
    }

    workingCounterTooLowCounter_ = 0;
    initlialized_ = true;

    return true;
}

void EctBus::updateRead() {
    if (!sentProcessData_) {
        MELO_DEBUG_STREAM("No process data to read.");
        return;
    }

    //! Receive the EtherCAT data.
    updateReadStamp_ = std::chrono::high_resolution_clock::now();
    {
        std::lock_guard<std::recursive_mutex> guard(contextMutex_);
        wkc_ = ecx_receive_processdata(&ecatContext_, EC_TIMEOUTRET);
    }
    sentProcessData_ = false;

    //! Check the working counter.
    if (!workingCounterIsOk()) {
        ++workingCounterTooLowCounter_;
        if (!busIsOk()) {
            MELO_WARN_THROTTLE_STREAM(1.0, "Bus is not ok. Too many working counter too low in a row: " << workingCounterTooLowCounter_)
        }
        MELO_DEBUG_STREAM("Working counter too low counter: " << workingCounterTooLowCounter_)
        MELO_WARN_THROTTLE_STREAM(1.0, "Update Read:" << this);
        MELO_WARN_THROTTLE_STREAM(1.0, "Working counter is too low: " << wkc_.load() << " < " << getExpectedWorkingCounter());
        return;
    }
    // Reset working counter too low counter.
    workingCounterTooLowCounter_ = 0;

    //! Each slave attached to this bus reads its data to the buffer.
    for (auto& slave : slaves_) {
        slave->updateRead();
    }
}

void EctBus::updateWrite() {
    if (sentProcessData_) {
        MELO_DEBUG_STREAM("Sending new process data without reading the previous one.");
    }

    //! Each slave attached to this bus write its data to the buffer.
    for (auto& slave : slaves_) {
        slave->updateWrite();
    }

    //! Send the EtherCAT data.
    updateWriteStamp_ = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    ecx_send_processdata(&ecatContext_);
    sentProcessData_ = true;
}

void EctBus::shutdown() {
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    // Set the slaves to state Init.
    if (getNumberOfSlaves() > 0) {
        setState(EC_STATE_INIT);
        waitForState(EC_STATE_INIT);
    }

    for (auto& slave : slaves_) {
        slave->shutdown();
    }

    // Close the port.
    if (ecatContext_.port != nullptr) {
        MELO_INFO_STREAM("Closing socket ...");
        ecx_close(&ecatContext_);
        // Sleep to make sure the socket is closed, because ecx_close is non-blocking.
        soem_interface::threadSleep(0.5);
    }

    initlialized_ = false;
}

void EctBus::setState(const uint16_t state, const uint16_t slave) {
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    if(!initlialized_) {
        MELO_ERROR_STREAM("Bus " << name_ << " was not successfully initialized, skipping operation");
        return;
    }
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    ecatContext_.slavelist[slave].state = state;
    ecx_writestate(&ecatContext_, slave);
    MELO_DEBUG_STREAM("Slave " << slave << ": State " << state << " has been set.");
}

bool EctBus::waitForState(const uint16_t state, const uint16_t slave, const unsigned int maxRetries, const double retrySleep) {
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    for (unsigned int retry = 0; retry <= maxRetries; retry++) {
        if (ecx_statecheck(&ecatContext_, slave, state, static_cast<int>(1e6 * retrySleep)) == state) {
            MELO_DEBUG_STREAM("Slave " << slave << ": State " << state << " has been reached.");
            return true;
        }
        // TODO: Do this for all states?
        ecx_send_processdata(&ecatContext_);
        wkc_ = ecx_receive_processdata(&ecatContext_, EC_TIMEOUTRET);
    }

    MELO_WARN_STREAM("Slave " << slave << ": State " << state << " has not been reached.");
    return false;
}

int EctBus::getExpectedWorkingCounter(const uint16_t slave) const {
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    return ecatContext_.grouplist[slave].outputsWKC * 2 + ecatContext_.grouplist[slave].inputsWKC;
}

std::string EctBus::getErrorString(ec_errort error) {
    std::stringstream stream;
    stream << "Time: " << (static_cast<double>(error.Time.sec) + (static_cast<double>(error.Time.usec) / 1000000.0));

    switch (error.Etype) {
    case EC_ERR_TYPE_SDO_ERROR:
        stream << " SDO slave: " << error.Slave << " index: 0x" << std::setfill('0') << std::setw(4) << std::hex << error.Index << "."
               << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(error.SubIdx) << " error: 0x" << std::setfill('0')
               << std::setw(8) << std::hex << static_cast<unsigned>(error.AbortCode) << " " << ec_sdoerror2string(error.AbortCode);
        break;
    case EC_ERR_TYPE_EMERGENCY:
        stream << " EMERGENCY slave: " << error.Slave << " error: 0x" << std::setfill('0') << std::setw(4) << std::hex << error.ErrorCode;
        break;
    case EC_ERR_TYPE_PACKET_ERROR:
        stream << " PACKET slave: " << error.Slave << " index: 0x" << std::setfill('0') << std::setw(4) << std::hex << error.Index << "."
               << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(error.SubIdx) << " error: 0x" << std::setfill('0')
               << std::setw(8) << std::hex << error.ErrorCode;
        break;
    case EC_ERR_TYPE_SDOINFO_ERROR:
        stream << " SDO slave: " << error.Slave << " index: 0x" << std::setfill('0') << std::setw(4) << std::hex << error.Index << "."
               << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(error.SubIdx) << " error: 0x" << std::setfill('0')
               << std::setw(8) << std::hex << static_cast<unsigned>(error.AbortCode) << " " << ec_sdoerror2string(error.AbortCode);
        break;
    case EC_ERR_TYPE_SOE_ERROR:
        stream << " SoE slave: " << error.Slave << " index: 0x" << std::setfill('0') << std::setw(4) << std::hex << error.Index
               << " error: 0x" << std::setfill('0') << std::setw(8) << std::hex << static_cast<unsigned>(error.AbortCode) << " "
               << ec_soeerror2string(error.ErrorCode);
        break;
    case EC_ERR_TYPE_MBX_ERROR:
        stream << " MBX slave: " << error.Slave << " error: 0x" << std::setfill('0') << std::setw(8) << std::hex << error.ErrorCode << " "
               << ec_mbxerror2string(error.ErrorCode);
        break;
    default:
        stream << " MBX slave: " << error.Slave << " error: 0x" << std::setfill('0') << std::setw(8) << std::hex
               << static_cast<unsigned>(error.AbortCode);
        break;
    }
    return stream.str();
}

void EctBus::printALStatus(const uint16_t slave) {
    std::lock_guard<std::recursive_mutex> guard(contextMutex_);
    assert(static_cast<int>(slave) <= getNumberOfSlaves());

    MELO_INFO_STREAM(" slave: " << slave << " alStatusCode: 0x" << std::setfill('0') <<
                     std::setw(8) << std::hex << ecatContext_.slavelist[slave].ALstatuscode <<
                     " " << ec_ALstatuscode2string(ecatContext_.slavelist[slave].ALstatuscode));
}

bool EctBus::checkForSdoErrors(const uint16_t slave, const uint16_t index) {
    while (ecx_iserror(&ecatContext_)) {
        ec_errort error;
        if (ecx_poperror(&ecatContext_, &error)) {
            std::string errorStr = getErrorString(error);
            MELO_ERROR_STREAM(errorStr);
            if (error.Slave == slave && error.Index == index) {
                soem_interface::common::MessageLog::insertMessage(message_logger::log::levels::Level::Error, errorStr);
                return true;
            }
        }
    }
    return false;
}

bool EctBus::workingCounterIsOk() const { return wkc_ >= getExpectedWorkingCounter(); }

bool EctBus::busIsOk() const { return workingCounterTooLowCounter_ < maxWorkingCounterTooLow_; }

void EctBus::syncDistributedClock0(const uint16_t slave, const bool activate, const double cycleTime, const double cycleShift) {
    MELO_INFO_STREAM("Bus '" << name_ << "', slave " << slave << ":  " << (activate ? "Activating" : "Deactivating")
                             << " distributed clock synchronization...");

    ecx_dcsync0(&ecatContext_, slave, static_cast<uint8_t>(activate), static_cast<uint32_t>(cycleTime * 1e9),
                static_cast<int32_t>(1e9 * cycleShift));

    MELO_INFO_STREAM("Bus '" << name_ << "', slave " << slave << ":  " << (activate ? "Activated" : "Deactivated")
                             << " distributed clock synchronization.");
}

EctBus::PdoSizeMap EctBus::getHardwarePdoSizes() {
    PdoSizeMap pdoMap;

    for (const auto& slave : slaves_) {
        pdoMap.insert(std::make_pair(slave->getName(), getHardwarePdoSizes(slave->getAddress())));
    }

    return pdoMap;
}

EctBus::PdoSizePair EctBus::getHardwarePdoSizes(const uint16_t slave) {
    return std::make_pair(ecatContext_.slavelist[slave].Obytes, ecatContext_.slavelist[slave].Ibytes);
}

template<>
bool EctBus::sendSdoRead<std::string>(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, std::string& value) {
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    //Expected length of the string. String needs to be preallocated
    int size = value.length();
    //Store for check at the end
    int expected_size = size;
    //Create buffer with the length of the string
    //char buffer[size];
    char* pbuffer = new char[size];
    int wkc = 0;
    {
        std::lock_guard<std::recursive_mutex> guard(contextMutex_);
        wkc = ecx_SDOread(&ecatContext_, slave, index, subindex, static_cast<boolean>(completeAccess), &size, pbuffer, EC_TIMEOUTRXM);
        //Convert read data to a std::string
        //value = std::string(buffer,size);
        value = std::string(*pbuffer,size);
        delete pbuffer;
    }
    if (wkc <= 0) {
        MELO_ERROR_STREAM("Slave " << slave << ": Working counter too low (" << wkc << ") for reading SDO (ID: 0x" << std::setfill('0')
                                   << std::setw(4) << std::hex << index << ", SID 0x" << std::setfill('0') << std::setw(2) << std::hex
                                   << static_cast<uint16_t>(subindex) << ").");
        return false;
    }

    if (size != (int)expected_size) {
        MELO_ERROR_STREAM("Slave " << slave << ": Size mismatch (expected " << expected_size << " bytes, read " << size
                                   << " bytes) for reading SDO (ID: 0x" << std::setfill('0') << std::setw(4) << std::hex << index
                                   << ", SID 0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(subindex) << ").");
        return false;
    }
    return true;
}

template<>
bool EctBus::sendSdoWrite<std::string>(const uint16_t slave, const uint16_t index, const uint8_t subindex, const bool completeAccess, const std::string value) {
    assert(static_cast<int>(slave) <= getNumberOfSlaves());
    const int size = value.length();
    const char* dataPtr = value.data();
    int wkc = 0;
    {
        std::lock_guard<std::recursive_mutex> guard(contextMutex_);
        wkc = ecx_SDOwrite(&ecatContext_, slave, index, subindex, static_cast<boolean>(completeAccess), size, &dataPtr, EC_TIMEOUTRXM);
    }
    if (wkc <= 0) {
        MELO_ERROR_STREAM("Slave " << slave << ": Working counter too low (" << wkc << ") for writing SDO (ID: 0x" << std::setfill('0')
                                   << std::setw(4) << std::hex << index << ", SID 0x" << std::setfill('0') << std::setw(2) << std::hex
                                   << static_cast<uint16_t>(subindex) << ").");
        return false;
    }
    return true;
}
#endif
