#include "slave.h"

Slave::Slave(ec_slavet *slaveT_, uint16_t slaveId_, uint8_t *readmap_, uint8_t *writemap_)
{
    //readmap = readmap_;
    //writemap = writemap_;
    //slavenum = slaveId_;

    //reqDataPdo = (REQUEST_T *)((uint8 *)writemap + sizeof(REQUEST_SYNC_T));
    //respDataPdo = (RESPONSE_T *)((uint8 *)readmap + sizeof(RESPONSE_SYNC_T));

    slaveT = slaveT_;
    slaveId = slaveId_;

    // FoE FLASH
    metaPage.clear();
    metaPage.resize(SIZE_FLASH_PAGE, 0);
    itemList.clear();
    for (int i = 0; i < COUNT_FLASH_PAGE; i++)
    {
        itemList.push_back(pItem(i));
    }

    // CoE SDO & PDO


    // 根据slave的参数初始化 //
    reqSyncPDO = (uint16_t *)&(slaveT->outputs[0]);
    rspSyncPDO = (uint16_t *)&(slaveT->inputs[0]);
    reqStatPDO = (uint16_t *)&(slaveT->outputs[2]);
    rspStatPDO = (uint16_t *)&(slaveT->inputs[2]);
    reqDataPDO = (uint16_t *)&(slaveT->outputs[4]);
    rspDataPDO = (uint16_t *)&(slaveT->inputs[4]);
    reqDataSize = (slaveT->Obytes-4);  //
    rspDataSize = (slaveT->Ibytes-4);
}

////////////////////////////////////////////////////////////////
// FoE FLASH

std::vector<uint8_t> &Slave::meta()
{
    return metaPage;
}

FoeFlashMeta *Slave::pMeta()
{
    if (metaPage.size() > 0)
    {
        return reinterpret_cast<FoeFlashMeta *>(&metaPage.data()[0]);
    }
    return nullptr;
}

// TODO: need more strict check
bool Slave::checkMeta()
{
    if ((pMeta()->magicNumber == FLASH_MAGIC_NUMBER) && (pMeta()->magicNumber2 == FLASH_MAGIC_NUMBER))
    {
        // TODO: more check here!
        return true;
    }
    return false;
}

bool Slave::createMeta()
{
    memset(pMeta(), 0, metaPage.size());
    pMeta()->magicNumber = FLASH_MAGIC_NUMBER;
    pMeta()->magicNumber2 = FLASH_MAGIC_NUMBER;
    pMeta()->createDate = 20231029;
    pMeta()->updateDate = 20231029;
    pMeta()->updateTime = 20231029;
    pMeta()->itemCount = 0;

    return true;
}

FoeFlashItem *Slave::pItem(int i)
{
    FoeFlashItem *p;
    if (pMeta() != nullptr)
        return &pMeta()->itemList[i];
    return nullptr;
}

/*
std::vector<FoeFlashItem*> Slave::getItemList() {
    std::vector<FoeFlashItem*> itemList;
    for (int i=0;i<COUNT_FLASH_PAGE;i++) {
        itemList.push_back(pItem(i));
    }
    return itemList;
}
*/

std::vector<FoeFlashItem *> Slave::getFlashItemList(const std::string &objName)
{
    std::vector<FoeFlashItem *> objItemList;
    // must from 0-max, for sequence
    for (int i = 0; i < COUNT_FLASH_PAGE; i++)
    {
        FoeFlashItem *pItem = &pMeta()->itemList[i];
        if (strcmp(pItem->name, objName.c_str()) == 0)
        { // TODO: check
            objItemList.push_back(pItem);
        }
    }
    return objItemList;
}

int32_t Slave::invalidFlashItemList(std::vector<FoeFlashItem *> &itemList)
{
    for (auto item : itemList)
    {
        memset(item, 0, sizeof(FoeFlashItem)); // TODO: check len
    }
    return 0;
}

int32_t Slave::invalidFlashObject(const std::string &objName)
{
    std::vector<FoeFlashItem *> objItemList;
    objItemList = getFlashItemList(objName);
    invalidFlashItemList(objItemList);
    //
    if (pMeta()->itemCount>0)
        pMeta()->itemCount--;
    pMeta()->updateDate = 20231029;
    pMeta()->updateTime = 20231029;
    return 0;
}

std::vector<FoeFlashItem *> Slave::addFlashObjectToItemList(const std::string &objName, const uint32_t objType, const int32_t objSize)
{
    std::vector<FoeFlashItem *> objItemList;
    int32_t remainSize = objSize;
    int32_t pageIndex = 1; // start index of item page
    int32_t dataOffset = 0;
    for (int32_t pageIndex = 1; pageIndex < COUNT_FLASH_PAGE; pageIndex++)
    { // page:1~63
        // for (item : itemList) {
        FoeFlashItem *item = pItem(pageIndex); // start from 1?
        if ((remainSize > 0) && (item->valid == false))
        {
            int32_t pageSize = (remainSize > SIZE_FLASH_PAGE) ? (SIZE_FLASH_PAGE) : (remainSize);
            memcpy(&item->name[0], objName.data(), objName.size()); // check last '\000'
            item->pageIndex = pageIndex;
            item->dataSize = pageSize;
            item->valid = true;
            item->dataType = objType;
            item->dataOffset = dataOffset;
            dataOffset += pageSize;
            remainSize -= pageSize;
            objItemList.push_back(item);
        }
        if (remainSize == 0)
            break;
        pageIndex++;
    }
    pMeta()->itemCount++;
    pMeta()->updateDate = 20231029;
    pMeta()->updateTime = 20231029;
    return objItemList;
}

int32_t Slave::getFlashObjectSize(const std::string &objName)
{
    int32_t objSize = 0;
    for (auto item : getFlashItemList(objName))
    {
        objSize += item->dataSize;
    }
    return objSize;
}

// objName+(objType+objSize)
std::map<std::string, std::pair<uint32_t, int32_t>> Slave::getFlashObjectMap()
{
    std::map<std::string, std::pair<uint32_t, int32_t>> objMap;
    FoeFlashItem *pItem;
    // traverse itemList
    for (int i = 0; i < pMeta()->itemCount; i++)
    {
        FoeFlashItem *pItem = &pMeta()->itemList[i];
        if (pItem->valid)
        {
            std::string objName = pItem->name;
            uint32_t objType = pItem->dataType;
            if (objMap.count(objName) != 0)
            { // exists
                // TODO: double check
                continue;
            }
            else
            {
                int32_t objSize = getFlashObjectSize(objName);
                objMap.insert({pItem->name, std::make_pair(objType, objSize)});
            }
        }
    }
    return objMap;
}


////////////////////////////////////////////////////////////////
// CoE SDO
int32_t Slave::setDesc(PDODescription& desc)
{
    pdoDesc = desc;
    return 0;
}

#if 0
PDOSubEntry& Slave::getSdoProject(const std::string& objName, const uint32_t objType)
{
    // TODO
    PDOSubEntry entry;

    return entry;
}
#endif



////////////////////////////////////////////////////////////////
// CoE PDO

// start new req
bool Slave::setSyncPDO()
{
    //uint16_t tmp = *reqSyncPDO;
    //uint8_t reqS = (*rspSyncPDO);
    *reqSyncPDO |= 0x0001;  //
    return true;
}

bool Slave::resetSyncPDO()
{
    *reqSyncPDO &= 0x0000;
    // *rspSyncPDO需要reset吗？如果这里reset，那么处理完rsp表示可以处理新的req，即canAcceptPDO函数满足条件 //

    //*reqSyncPDO &= 0xFC;  // & 1111_1100 , clear least 2bit
    //*reqSyncPDO = *rspSyncPDO;
    return true;
}


bool Slave::canAcceptPDO()
{
    uint16_t reqS = (*reqSyncPDO);
    uint16_t rspS = (*rspSyncPDO);
    // TODO:
    return (((reqS&0x0001)==0) && ((rspS&0x0001)==0));  // no req and no rsp pending
    //return true;
}

#if 0
// no use
bool Slave::isBusy()
{
    // TODO:
    uint8_t reqS = (*reqSyncPDO);
    uint8_t rspS = (*rspSyncPDO);
    return ((reqS==1) && (rspS==0));  // req is pending
}
#endif

bool Slave::isRspReady()
{
    // TODO: (255+1)
    uint16_t reqS = (*reqSyncPDO);
    uint16_t rspS = (*rspSyncPDO);
    // TODO: multiple slave with diff reqSyncPDO/rspSyncPDO
    return ((reqS & 0x0001) && (rspS & 0x0001));  // rsp is pending
}

bool Slave::setPDO(uint16_t start_index, uint16_t pdo)
{
    if ((slaveT->Obytes - start_index) < sizeof(pdo)) {
        return false;
    }
    memcpy(slaveT->outputs + start_index, &pdo, sizeof(pdo));
    return true;
}

// TODO
bool Slave::setPDO(uint16_t start_index, std::vector<uint8_t> &pdoList)
{
    // pdoList.size可能会超出slave的data pdo 尺寸，需要削减至可用容量，否则传输失败 //
    size_t byteSize = pdoList.size();
    if ((slaveT->Obytes - start_index) < pdoList.size()) {
        byteSize = (slaveT->Obytes - start_index);
        //return false;
    }
    memcpy(slaveT->outputs + start_index, pdoList.data(), byteSize);
    return true;
}

// 是可以通过slvave的结构获取动态PDO的size的 //
std::vector<uint8_t> Slave::getPDO(uint16_t start_index, uint16_t byteSize)
{
    std::vector<uint8_t> pdoList;
    if ((slaveT->Ibytes - start_index) < byteSize)
    {
        return pdoList;
    }
    pdoList.resize(byteSize);
    memcpy(&pdoList.data()[0], slaveT->inputs + start_index, byteSize);
    return pdoList;
}


 uint16_t Slave::getReqPdoSize() {
    return reqDataSize;
 }

 uint16_t Slave::getRspPdoSize() {
    return rspDataSize;
 }



////////////////////////////////////////////////////////////////
//
#if 0
Slave::Slave(RESPONSE_SYNC_T *readmap_, REQUEST_SYNC_T *writemap_, int slavenum_)
{
    readmap = readmap_;
    writemap = writemap_;
    slavenum = slavenum_;

    reqDataPdo = (REQUEST_T *)((uint8 *)writemap + sizeof(REQUEST_SYNC_T));
    respDataPdo = (RESPONSE_T *)((uint8 *)readmap + sizeof(RESPONSE_SYNC_T));
}
#endif

#if 0
void Slave::sendPdo(uint8 Pdonum, uint32 Pdo)
{
    switch (Pdonum)
    {
    case INDX_req_syncPDO:
        writemap->reqSyncPDO = (uint8)Pdo;
        break;
    case INDX_req_sequence:
        writemap->sequence = (uint8)Pdo;
        break;
    case INDX_req_cmd:
        reqDataPdo->cmd = (uint8)Pdo;
        break;
    case INDX_req_bank:
        reqDataPdo->bank = (uint8)Pdo;
        break;
    case INDX_req_index:
        reqDataPdo->index = (uint8)Pdo;
        break;
    case INDX_req_data0:
        reqDataPdo->data[0] = Pdo;
        break;
    case INDX_req_data1:
        reqDataPdo->data[1] = Pdo;
        break;
    default:
        std::cout << "the index is inavailable" << std::endl;
    }
}

// yinwenbo
void Slave::sendDataPdo(uint8_t *ba, int32_t size)
{
    // memcpy
    memcpy((uint8_t *)reqDataPdo, (uint8_t *)ba, size);
}

// yinwenbo
void Slave::getDataPdo(uint8_t *ba, int32_t &size)
{
    // QByteArray ba;
    uint16_t dataPdoSize = sizeof(RESPONSE_T); // readmap->dataLength;
    // ba.resize(dataPdoSize);
    memcpy(ba, respDataPdo, size);
    // return ba;
}

uint16 Slave::getPdo(uint8 Pdonum)
{
#if 0
    switch (Pdonum)
    {
    case INDX_resp_syncPDO:
        return readmap->respSyncPDO;
    case INDX_resp_sequence:
        return readmap->sequence;
    case INDX_resp_status:
        return respDataPdo->status;
    case INDX_resp_result:
        return respDataPdo->result;
    case INDX_resp_data0:
        return respDataPdo->data[0];
    case INDX_resp_data1:
        return respDataPdo->data[1];
    case INDX_resp_data2:
        return respDataPdo->data[2];
    case INDX_resp_data3:
        return respDataPdo->data[3];
    default:
        std::cout << "the index is inavailable" << std::endl;
        return 0;
    }
#endif
    return 0;
}
#endif

uint16_t Slave::getId()
{
    return slaveId;
}
