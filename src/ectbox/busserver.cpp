#include <ethercat/busserver.h>

BusServer::BusServer()
{
    //connect(this, SIGNAL(connectionLost()), this, SLOT(disConnectNic()));
}

// how to quit???
BusServer::~BusServer()
{
    //m_runThread.detach();
    disConnectNic();
}

// user args: type, nicName, if reconnect
int BusServer::loadDriver(std::string type, std::string name, bool reConnect) {
    // close?
    if (mBus!=nullptr) {
        if ((mBus->type()==type) && (mBus->name()==name)) { // if connected
            if (reConnect) {
                //mBus->connect(name);
                mBus->open();
            } else {
                return 0;  // connected
            }
        }
    }
    if (type == "SIMBUS") {
        TINFO("load simbus");
        mBus = new SimBus(name);
    } else if (type == "ECTBUS") {
        TINFO("load ectbus");
        mBus = new EctBus(name);
    }
    mBus->open();
    return 0;
}

// create slaveList
int BusServer::open(const std::string& name) {
    if (mBus==nullptr) {
        return -1;
    }
    //mBus->connectNIC();
    return 0;
}

int BusServer::startEctLoop() {
    // variables to control start/stop/connect?
    mBus->startup();
    return 0;
}

// init SimPlatform
// start

//static std::vector<std::string> BusServer::getAdapterNames(const std::string type) {

//    adapterNames.clear();
//    if (mBus!=nullptr) {
//        adapterNames = mBus->getAdapterNames();
//        return 0;
//    } else {
//        return -1;
//    }
//}

#if 0
int BusServer::startEctLoop()
{
    if (mBus!=nullptr) {
//        if (mBus->isConnected()) {
//        }
    }
    return 0;
}
#endif

int BusServer::stopEctLoop() {
    return 0;
}





//QStringList BusServer::findNic()
//{
//    QStringList list;
//#if 0
//    for (const auto& name : BusBase::getAdapterNames())
//    {
//        list.push_back(QString::fromStdString(name));
//    }
//#endif
//    return list;
//}

bool BusServer::connectStatus()
{
    return m_runThreadRunning;
}



// 需要支持一下两种情况
// 1、带电插入新的tpu，连接状态是不变的
// 2、断开网线/移除tpu，连接是断开的
bool BusServer::connectNic(const QString ifname)
{
#if 0
    //qDebug()<<"BusServer::connectNic begin";

    // 带电插入新tpu/重新connect：disConnectNic要阻值对线程断开signal的响应？？
    // 本来就处于连接状态下的重连，不需要发送signal给其他模块做处理
    //if (connectStatus()) {
        this->blockSignals(true);
        disConnectNic();
        this->blockSignals(false);
    //}
    if (!m_ethercatbus.openBus(ifname.toStdString()))
        return false;
    if(!m_ethercatbus.waitUntilAllSlavesReachedOP())
        return false;
    m_ethercatbus.testwd();
    m_ethercatbus.createSlaves();
    NicName = ifname.toStdString();

    // yinwenbo,如果已经在运行
//    if (m_runThreadRunning) {
//        return true;
//    }

    m_runThreadRunning = true;

    // yinwenbo,20211020
    // 如果是硬件排错，需要重新扫描硬件的情况，在该进程已经处于运行的情况下？

    m_runThread = std::thread
    {
        [this]
        {
            run();
        }
    };
    qDebug() << "connect successfully" ;
    qDebug()<<QString("find %1 slaves").arg(getAllSlaves());

    //qDebug()<<"BusServer::connectNic end";
#endif
    return true;
}


void BusServer::run()
{
#if 0
    while (m_runThreadRunning)
    {
        bool isrun;
        {
            std::scoped_lock lock(m_update_mutex);
            if (m_ethercatbus.updateBus())
            {
                isrun =  true;
            }
            else
            {
                qDebug()<<"Could not update bus.\n";  // 说明有TPU掉线了！！！硬件故障！！
                isrun =  false;
                emit updateBusFail();
                qDebug()<<"signal:updateBusFail";
            }
        }
        if (isrun)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
        else
        {
            m_runThreadRunning = false;
        }
    }
#endif
    // do something
    emit connectionLost();  // 该信号要发给上层用
    //disConnectNic();
    qDebug()<<"signal:connectionLost";
}

// by zhuguowei, 20211021
void BusServer::disConnectNic()
{
    //qDebug()<<"BusServer::disConnectNic begin";
#if 0
    if(NicName== "")
        return ;
    NicName = "";

    m_runThreadRunning = false;

    // 这是要把线程再 加入执行吗？
    if(m_runThread.joinable())
        m_runThread.join();

     m_ethercatbus.closeBus();
     qDebug() << "disconnect successfully" ;
     //qDebug()<<"BusServer::disConnectNic end";
#endif
     return;
}

#if 0
bool BusServer::writeReqDataPDOs(QMap<uint8_t, QByteArray> reqlist)
{
    std::scoped_lock lock(m_update_mutex);
    QByteArray ba;

    QMap<uint8_t, QByteArray>::const_iterator req;
    for (req = reqlist.constBegin(); req != reqlist.constEnd(); ++req){
        if(req.key()>m_ethercatbus.getslavenum())
            return false;
        ba = req.value();
        // TBD：将req.data()拷贝到IoMap的slave的DataPDO的位置
        m_ethercatbus.getslavelist()[req.key()]->sendDataPdo(ba);
    }
    m_ethercatbus.updateBus();
    return true;
}

// yinwenbo
bool BusServer::writeReqDataMail(QMap<uint8_t, QByteArray> reqlist)
{
    QMap<uint8_t, QByteArray>::const_iterator req;
    for (req = reqlist.constBegin(); req != reqlist.constEnd(); ++req){
        if (m_ethercatbus.sendSdo(req.key(), req.value())<0)
            return false;
    }
    return true;
}

bool BusServer::waitRespSyncPDOs(QList<uint8_t> slaveList, uint8_t respCode)
{
    float time = clock();
    bool com = false;
    uint8_t slaveID;

    //qDebug()<<slaveList.size()<<slaveList;

    while (!com)
    {
        m_ethercatbus.updateBus();

        com = true;
        for (int i=0;i<slaveList.size();i++){
        //for (uint8_t slaveID : slaveList)
        //{
            slaveID = slaveList[i];
            //qDebug()<<slaveID;

            if(slaveID>m_ethercatbus.getslavenum())
                return false;

            // 有没有更好的机制？？
            //if(m_ethercatbus.getslavelist()[slaveID]->getPdo(INDX_resp_status) == 0 |m_ethercatbus.getslavelist()[slaveID]->getPdo(INDX_resp_sequence1) !=sequence)
            //    com =false;
            if(m_ethercatbus.getslavelist()[slaveID]->getPdo(INDX_resp_syncPDO) != respCode)
                com =false;

            if((clock()- time)>1000 )  // clock is ms
                return false;
        }
    }
    return true;
}

// 更通用的情况：reqCode使用QMap<slave, reqCode>，但是这样的话太复杂化了
// 最好是一组slave执行的是同样的状态转换动作
bool BusServer::writeReqSyncPDOs(QList<uint8_t>slaveList, uint8_t reqCode)
{
    // commet for test 20211027
    std::scoped_lock lock(m_update_mutex);
    for (const auto& slaveID : slaveList)
    {
        if(slaveID>m_ethercatbus.getslavenum())
            return false;
        m_ethercatbus.getslavelist()[slaveID]->sendPdo(INDX_req_syncPDO,reqCode);
        //m_ethercatbus.getslavelist()[slaveID]->sendPdo(INDX_req_sequence,sequence);
    }
//	m_ethercatbus.updateBus();

    return true;
}

// yinwenbo
QMap<uint8_t, QByteArray> BusServer::readRespDataPDOs(QList<uint8_t> slaveList)
{
    QMap<uint8_t, QByteArray> respList;

    m_ethercatbus.updateBus();

    for (const auto& slaveID : slaveList)
    {
        if(slaveID>m_ethercatbus.getslavenum())
            break;
        QByteArray respData = m_ethercatbus.getslavelist()[slaveID]->getDataPdo();
        respList.insert(slaveID, respData);
    }
    return respList;
}

// yinwenbo
QMap<uint8_t, QByteArray> BusServer::readRespDataMail(QList<uint8_t>slaveList)
{
    QMap<uint8_t, QByteArray> respList;

//    m_ethercatbus.updateBus();  // no need?

    for (const auto& slaveID : slaveList)
    {
        if(slaveID>m_ethercatbus.getslavenum())
            break;
        QByteArray resp;
        if (m_ethercatbus.getSdo(slaveID, resp)>0){
            respList.insert(slaveID, resp);
        }
    }
    return respList;
}

int BusServer::getAllSlaves()
{
    return m_ethercatbus.getslavenum();
}


// 非同步的数据传输，如参数设置、读写config、复位等
// 只要不是跨节点采集的动作，都不需要同步？非也
bool BusServer::doAsyncRequest(QList<uint8_t> slaveList, QMap<uint8_t, QByteArray> reqlist, QMap<uint8_t, QByteArray> &resplist)
{

    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_IDLE);

    if(waitRespSyncPDOs(slaveList, TPU_SYNC_PDO_IDLE)==false){
        qDebug()<<"waitSyncPDOs TimeOut fail1";
        return false;  // 空的list
    }

    // send
#ifdef REQ_PDO_MODE
    writeReqDataPDOs(reqlist);  // PDO本身就是同步的，因此TPU也是同步动作
    // TPU收到reqDataPDO后，执行动作，等待延时，将结果和状态更新到respDataPDO和
#else
    writeReqDataMail(reqlist);
    // TPU收到reqDataMail后，执行动作，等待延时，将结果和状态更新到respDataMail和
#endif

    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_SET);

    if(waitRespSyncPDOs(slaveList, TPU_SYNC_PDO_GETREADY)==false){
        qDebug()<<"waitSyncPDOs TimeOut fail2";
        return false;  // 空的list
    }

    // receive
#ifdef RESP_PDO_MODE
    resplist = readRespDataPDOs(slaveList);
#else
    resplist = readRespDataMail(slaveList);
#endif
    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_IDLE);

    return true;
}

// slaveList = reqlist.keys()
bool BusServer::doSyncRequest(QList<uint8_t> slaveList, QMap<uint8_t, QByteArray> reqlist, QMap<uint8_t, QByteArray> &resplist)
{

    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_IDLE);

    if(waitRespSyncPDOs(slaveList, TPU_SYNC_PDO_IDLE)==false){
        qDebug()<<"waitSyncPDOs TimeOut fail3";
        return false;  // 空的list
    }

    // send
#ifdef REQ_PDO_MODE
    writeReqDataPDOs(reqlist);  // PDO本身就是同步的，因此TPU也是同步动作
    // TPU收到reqDataPDO后，执行动作，等待延时，将结果和状态更新到respDataPDO和
#else
    writeReqDataMail(reqlist);
    // TPU收到reqDataMail后，执行动作，等待延时，将结果和状态更新到respDataMail和
#endif

    // start req in TPU
    // reqSyncPDO = TPU_SYNC_PDO_RUN
    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_SET);

    // for test
    //resplist = readRespDataPDOs(slaveList);

//#ifdef TPU_SYNC_MODE
    // 判断 (respSyncPDO == TPU_SYNC_PDO_SETREADY)
    if(waitRespSyncPDOs(slaveList, TPU_SYNC_PDO_SETREADY)==false){
        qDebug()<<"waitSyncPDOs TimeOut fail8";
        return false;  // 空的list
    }
    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_GET);  // start
//#else

//#endif
    // 判断 (respSyncPDO == TPU_SYNC_PDO_IDLE)
    if(waitRespSyncPDOs(slaveList, TPU_SYNC_PDO_GETREADY)==false){
        qDebug()<<"waitSyncPDOs TimeOut fail9";
        return false;  // 空的list
    }

    // receive
#ifdef RESP_PDO_MODE
    resplist = readRespDataPDOs(slaveList);
#else
    resplist = readRespDataMail(slaveList);
#endif
    writeReqSyncPDOs(slaveList, TPU_SYNC_PDO_IDLE);

    return true;
}
#endif
