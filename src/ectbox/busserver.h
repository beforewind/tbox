#ifndef BUSSERVER_H
#define BUSSERVER_H

#include <QObject>
#include <QThread>
#include <QDebug>

//#include <ethercat/ethercatbus.h>  // temp use
#include <ethercat/ectbus.h>
#include <ethercat/simbus.h>

#include <cassert>
// std
#include <atomic>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include <chrono>
#include <mutex>

//#include "request.h"


class BusWorker:public QObject  // work定义了线程要执行的工作
{
    Q_OBJECT
public:
    BusWorker(QObject* parent = nullptr){}
public slots:
    void doWork(int parameter)  // doWork定义了线程要执行的操作
    {
        qDebug()<<"receive the execute signal---------------------------------";
        qDebug()<<"     current thread ID:"<<QThread::currentThreadId();
        for(int i = 0;i!=1000000;++i)
        {
            ++parameter;
        }
        qDebug()<<"      finish the work and sent the resultReady signal\n";
        emit resultReady(parameter);  // emit啥事也不干，是给程序员看的，表示发出信号发出信号
    }

signals:
    void resultReady(const int result);  // 线程完成工作时发送的信号
};



class BusServer : public QObject
{
    Q_OBJECT
    QThread busWorkerThread;
public:
    enum BusStatus {DISCONNECTED, CONNECTED, LOOPRUNNING, PAUSED};
    BusServer();
    ~BusServer();
    static std::map<std::string, std::string> findNic() {
        return EctBus::findAvailableNics();
    }
    int loadDriver(std::string type, std::string nicName, bool reConnect);
    static std::vector<std::string> getAdapterNames(const std::string type) {
        if (type=="SIMBUS") {
            return SimBus::getAdapterNames();
        } else if (type == "ECTBUS") {
            return EctBus::getAdapterNames();
        }
    }

    int open(const std::string& name);

    //QStringList findNic();
    bool connectNic(const QString ifname);
    bool connectStatus();
    bool isConnected();
    bool isInEctLoop();  // running

    // control the loop thread
    int status();

    int disconnect();
    int startEctLoop();
    int stopEctLoop();
    int updatePDO();
    int writeSDO();
    int readSDO();
    int writeFile();
    int readFile();


//    QList<ContinuityResult> testContinuity(QList<Link> linkList,ContinuityParameter parameter,QString ifname,int failnum);//先不实现，得确定传输的数据形式
    bool loadfile(QString ifname,uint16 slave,const char *filename);//不急着实现
    void deleteNic(QString ifname);//不急着实现
    int getAllSlaves();

#if 0
    //现在通信只涉及到这几个参数syncPDO req sequence resp sequence;
    //bool writeReqPDOs(std::map<uint8,uint8>, uint8 sequence);
    //bool writeReqPDOs(QMap<uint8_t, QByteArray> reqlist);  // yinwenbo
    bool writeReqDataPDOs(QMap<uint8_t, QByteArray> reqlist);  // yinwenbo
    //bool writeReqMail(std::map<uint8,uint8>, uint8 sequence);
    bool writeReqDataMail(QMap<uint8_t, QByteArray> reqlist);  // yinwenbo
    //bool waitSyncPDOs(std::vector<uint8>, uint8 sequence);
    //bool waitSyncPDOs(QList<uint8_t> slaveList);  // yinwenbo
    bool waitRespSyncPDOs(QList<uint8_t> slaveList, uint8_t respCode);  // yinwenbo
    //bool writeSyncPDOs(std::vector<uint8>, uint8 sequence);
    //bool writeSyncPDOs(QList<uint8_t>slaveList);  // yinwenbo
    bool writeReqSyncPDOs(QList<uint8_t>slaveList, uint8_t reqCode);  // yinwenbo
    //std::map<uint8,uint8> readRespPDOs(std::vector<uint8>, uint8 sequence);
    QMap<uint8_t, QByteArray> readRespDataPDOs(QList<uint8_t>slaveList);  // yinwenbo
    //std::map<uint8,uint8> readRespMail(std::vector<uint8>, uint8 sequence);
    QMap<uint8_t, QByteArray> readRespDataMail(QList<uint8_t>slaveList);  // yinwenbo

    bool doAsyncRequest(QList<uint8_t> slaveList, QMap<uint8_t, QByteArray> reqlist, QMap<uint8_t, QByteArray> &resplist);
    bool doSyncRequest(QList<uint8_t> slaveList, QMap<uint8_t, QByteArray> reqlist, QMap<uint8_t, QByteArray> &resplist);
#endif

public slots:
    void disConnectNic();

signals:
    void updateBusFail();
    void connectionLost();

private:
    void run();
     //std::string NicName;
     std::string NicName{""};
     std::mutex m_update_mutex{};
     //ethercatbus m_ethercatbus;
     BusBase *m_ethercatbus;
     BusBase *mBus {nullptr};
     bool m_runThreadRunning {false};
     std::thread m_runThread;
};

#endif // BUSSERVER_H
