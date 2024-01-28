#include "ethercatbus.h"

ethercatbus::ethercatbus()
{

}
ethercatbus::~ethercatbus()
{   for(size_t i = 0; i < slavelist.size(); i++)
    {
    delete slavelist[i];
    }
    slavelist.clear();
}

void ethercatbus::init()
{
    m_busReady = false;
    slavesCreated = false;
    m_expectedWKC=0;
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

std::vector<std::string> ethercatbus::getAdapterNames()
{
    std::vector<std::string> result;
    ec_adaptert* adapter = ec_find_adapters();
    while (adapter != nullptr)
    {
        result.emplace_back(adapter->name);
        adapter = adapter->next;
    }
    return result;
}

bool ethercatbus::openBus(const std::string ifname)
{
    init();
    if (ecx_init(&ecx_context_ins, ifname.data()) >0)
    {
        std::cerr << "Initialized bus on adapter " << ifname.data() << "\n";
        return true;
    }

    std::cerr << "Could not init EtherCAT on adpater " << ifname.data() << "\n";
    return false;
}

bool ethercatbus::waitUntilAllSlavesReachedOP()
{
//    IO_offset.clear();
//     IO_num = 0;
//      m_busReady = false;
    if (ec_config_mul(&ecx_context_ins,FALSE, &m_IOmap) > 0)
    {
        ecx_configdc(&ecx_context_ins);
        while(*(ecx_context_ins.ecaterror)) printf("%s", ecx_elist2string(&ecx_context_ins));
        printf("%d slaves found and configured.\n",*(ecx_context_ins.slavecount));
        /* wait for all slaves to reach SAFE_OP state */
        ecx_statecheck(&ecx_context_ins,0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 3);
        if (ecx_context_ins.slavelist[0].state != EC_STATE_SAFE_OP )
        {
            printf("Not all slaves reached safe operational state.\n");
            ecx_readstate (&ecx_context_ins);
            for(int i = 1; i<=*(ecx_context_ins.slavecount); i++)
            {
                if(ecx_context_ins.slavelist[i].state != EC_STATE_SAFE_OP)
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

            for ( int slave = 1; slave <= *(ecx_context_ins.slavecount) ; slave++)
            {
                ecx_context_ins.slavelist[slave].state = EC_STATE_OPERATIONAL;
                ecx_writestate(&ecx_context_ins,slave);
            }
            ecx_statecheck(&ecx_context_ins,0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE * 3);
            for (int i = 1; i <= *(ecx_context_ins.slavecount) ; i++)
            {
                if(ecx_context_ins.slavelist[i].state != EC_STATE_OPERATIONAL)
                {
                    printf("Slave %d State=%2x StatusCode=%4x : %s\n",
                    i, ecx_context_ins.slavelist[i].state, ecx_context_ins.slavelist[i].ALstatuscode, ec_ALstatuscode2string(ecx_context_ins.slavelist[i].ALstatuscode));
                }
                else
                {
                    m_busReady = true;
                    printf("All slaves reached OP\n");
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

bool ethercatbus::updateBus()
{
    if (m_busReady)
    {
        ecx_send_processdata_group (&ecx_context_ins, 0);
        int lastWorkCounter = ecx_receive_processdata_group (&ecx_context_ins, 0,EC_TIMEOUTMON * 10);
        int aa = 1;
        if (lastWorkCounter >= m_expectedWKC)  // 是不是要更新？？
        {
            return true;
        }
//       return true;//pigfly
    }
    return false;
}

void ethercatbus::closeBus()
{
    ecx_close(&ecx_context_ins);
    m_busReady = false;
    {
        for(size_t i = 0; i < slavelist.size(); i++)
        {
            delete slavelist[i];
        }
        slavelist.clear();
    }
    std::cerr << "Closed bus\n";
}

// 需要支持重新创建，尤其是硬件排错阶段
void ethercatbus::createSlaves()
{
    // yinwenbo,20211020
    if(slavesCreated)
    {
        // 需要reset已经建立的数据结构，应该不需要修改连接进程的状态变量？
        //slavelist.clear();  // TODO
        //qDebug()<<"Slave have already been created! There must be a logic error.";
        throw std::logic_error("Slave have already been created! There must be a logic error.");
    }

    if (!m_busReady)
    {
        std::cerr << "Bus not ready!\n";
    }
    else
    {
        for (int i = 1; i <= *(ecx_context_ins.slavecount) ; i++)
        {
            slave* slave_ = new slave((RESPONSE_SYNC_T*)ecx_context_ins.slavelist[i].inputs,(REQUEST_SYNC_T*)ecx_context_ins.slavelist[i].outputs,i);
            slavelist.push_back(slave_);
        }
    }

    slavesCreated = true;
}


std::vector<slave*> ethercatbus::getslavelist()
{
//    if(slavesCreated)
//        std::cout<<"the slavelist has not been not created" << std::endl;
//    else
//        std::cout<<"the slavelist has been created" << std::endl;
    return slavelist;
}

bool ethercatbus::updateSlave(int slave,const char *filename)
{
    if(m_busReady)
    {
        ecx_context_ins.slavelist[slave].state = EC_STATE_INIT;
        ecx_writestate(&ecx_context_ins,slave);
        ecx_statecheck(&ecx_context_ins,slave, EC_STATE_INIT,  EC_TIMEOUTSTATE * 3);
        ecx_context_ins.slavelist[slave].state = EC_STATE_BOOT;
        ecx_writestate(&ecx_context_ins,slave);
        ecx_statecheck(&ecx_context_ins,slave, EC_STATE_BOOT,  EC_TIMEOUTSTATE * 3);
        bool loadsuccess = foe( slave,filename);  // 这里需要foe吗？
        ecx_context_ins.slavelist[slave].state = EC_STATE_INIT;
        ecx_writestate(&ecx_context_ins,slave);
        ecx_statecheck(&ecx_context_ins,slave, EC_STATE_INIT,  EC_TIMEOUTSTATE * 3);
        if(loadsuccess)
            return true;
       else
            return false;
     }
    else return false;
}

bool ethercatbus::foe(int slave,const char *filename)
{
        using namespace std;
        unsigned char* data;
        int length;
        int read_size;

        FILE *fp=fopen(filename,"rb");
        if(!fp)
        { std::cout <<"can't open the file !!! "<< std::endl;
            return false;}
        fseek(fp,0L,SEEK_END);
        length=ftell(fp);
        fclose(fp);

         fp=fopen(filename,"rb");
         if(!fp)
           return false;
        data = (unsigned char *)malloc(length+32);
        if (!data)
        {std::cout <<"the file is empty!!!"<< std::endl;
            return false;}
        //fseek(fp,0L,SEEK_SET);
        read_size = fread(data, 1, length, fp);
        if (read_size!=length)
          return false;
        fclose(fp);

         int wkc = ecx_FOEwrite_file(&ecx_context_ins,slave,filename,0x01234567,length,data, EC_TIMEOUTMON * 10);
          std::cout <<"the wkc is "<<wkc<< std::endl;
          std::cout <<"the filename is "<<filename<< std::endl;
          std::cout <<"the length is "<<length<< std::endl;

            if (data)
                free(data);

          if(wkc==1)
              return true;
          else
              return false;


}

//bool ethercatbus::sendSdo(uint8 slavenum, slave::REQUEST_T Sdo)
//{
//    if(slavenum >ec_slavecount)
//        return false;
//    else
//    {
//        bool send =ecx_SDOwrite(&ecx_context_ins, slavenum, INDX_SDO_REQ, 1,
//                                true, sizeof(Sdo), &Sdo, EC_TIMEOUTSTATE * 3);
//        return send;
//    }
//}

// yinwenbo
// check req.size<sdo.size
// slavenum:from 1 to n
int ethercatbus::sendSdo(uint8 slavenum, QByteArray req)
{
    if(slavenum >ec_slavecount)
        return -1;
    else if (req.size()>MAX_SDO_REQ_SIZE)
        return -2;
    else
    {
        int send =ecx_SDOwrite(&ecx_context_ins, (slavenum+1), INDX_SDO_REQ, 1,
                                true, req.size(), req.data(), EC_TIMEOUTSTATE * 3);
        return send;
    }
}

// slavenum:from 1 to n
int ethercatbus::getSdo(uint8 slavenum, QByteArray& resp)
{
    //QByteArray resp;
    resp.clear();
    uint8 buf[2048];  // TBD  MAX_SDO_RES_SIZE
    if(slavenum >ec_slavecount)
        return -1;
    else
    {
        int size=2048;  // maybe MAX_SDO_RES_SIZE=64bytes
        int get =ecx_SDOread(&ecx_context_ins,slavenum+1, INDX_SDO_RES,1,
                              true, &size, &buf, EC_TIMEOUTSTATE * 3);
        if (size>0 & size<2048){  // TODO
            resp.resize(size);
            memcpy(resp.data(), &buf, size);
            return size;
        } else
            return -2;  // bad size
    }
}

int ethercatbus::getslavenum()
{
    return ec_slavecount;
}

void ethercatbus::testwd()
{
	// 20210801
	// 把这个改了之后就相当都把watchdog开了，
	// 如果不是debug一个断点一个断点的跑的话其实开了也没有任何影响
    uint16 aa = 0x0000;
    int wkc =ecx_BWR(&ecx_port,0x0000, 0x0420,2, &aa,EC_TIMEOUTRET);
}
//bool ethercatbus::sendSdo(uint8 Sdonum, uint16 Sdo)
//{
//   return t
//}

//uint16 ethercatbus::getSdo(uint8 Sdonum)
//{

//}
