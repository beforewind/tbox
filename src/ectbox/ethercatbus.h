#ifndef ETHERCATBUS_H
#define ETHERCATBUS_H

#include <vector>
#include "slave.h"
#include "ethercat.h"

#include <QtCore>

class ethercatbus
{
public:
    ethercatbus();
    ~ethercatbus();
    static std::vector<std::string> getAdapterNames();
    bool openBus(const std::string ifname);
    void closeBus();
    bool updateBus();
    bool waitUntilAllSlavesReachedOP();
    void createSlaves();
    std::vector<slave*> getslavelist();
    bool updateSlave(int slave,const char *filename);
    //int sendSdo(uint8 slavenum, slave::REQUEST_T Sdo);//暂时不实现
    int sendSdo(uint8 slavenum, QByteArray req);  // yinwenbo
    int getSdo(uint8 slavenum, QByteArray& resp);  // yinwenbo
    int getslavenum();
    void testwd();



private:
    bool m_busReady = false;//这个bus在第一次进入op状态之后变为true，之后一直保持true直到关闭网口，即使期间不再是op状态也依然是true。
    bool slavesCreated = false;
    int m_expectedWKC;
    std::vector<slave*> slavelist;
    char m_IOmap[4096];
    void init();
    bool foe(int slave,const char *filename);





    //       ecx_context ecx_context_ins;
           ec_slavet               ec_slave[EC_MAXSLAVE];
           /** number of slaves found on the network */
           int                     ec_slavecount = 0;
           /** slave group structure */
           ec_groupt               ec_group[EC_MAXGROUP];

           /** cache for EEPROM read functions */
           uint8            ec_esibuf[EC_MAXEEPBUF];
           /** bitmap for filled cache buffer bytes */
           uint32           ec_esimap[EC_MAXEEPBITMAP];
           /** current slave for EEPROM cache buffer */
           ec_eringt        ec_elist;
           ec_idxstackT     ec_idxstack;//

           /** SyncManager Communication Type struct to store data of one slave */
           ec_SMcommtypet   ec_SMcommtype[EC_MAX_MAPT];
           /** PDO assign struct to store data of one slave */
           ec_PDOassignt    ec_PDOassign[EC_MAX_MAPT];
           /** PDO description struct to store data of one slave */
           ec_PDOdesct      ec_PDOdesc[EC_MAX_MAPT];

           /** buffer for EEPROM SM data */
           ec_eepromSMt     ec_SM;//
           /** buffer for EEPROM FMMU data */
           ec_eepromFMMUt   ec_FMMU;//
           /** Global variable TRUE if error available in error stack */
           boolean                 EcatError = FALSE;

           int64                   ec_DCtime=0;

           ecx_portt               ecx_port;
           ecx_redportt            ecx_redport;

           ecx_context ecx_context_ins ={
               &ecx_port,          // .port          =
               &ec_slave[0],       // .slavelist     =
               &ec_slavecount,     // .slavecount    =
               EC_MAXSLAVE,        // .maxslave      =
               &ec_group[0],       // .grouplist     =
               EC_MAXGROUP,        // .maxgroup      =
               &ec_esibuf[0],      // .esibuf        =
               &ec_esimap[0],      // .esimap        =
               0,                  // .esislave      =
               &ec_elist,          // .elist         =
               &ec_idxstack,       // .idxstack      =
               &EcatError,         // .ecaterror     =
               &ec_DCtime,         // .DCtime        =
               &ec_SMcommtype[0],  // .SMcommtype    =
               &ec_PDOassign[0],   // .PDOassign     =
               &ec_PDOdesc[0],     // .PDOdesc       =
               &ec_SM,             // .eepSM         =
               &ec_FMMU,           // .eepFMMU       =
               NULL,               // .FOEhook()
               NULL,               // .EOEhook()
               0                   // .manualstatechange
           };

};

#endif // ETHERCATBUS_H
