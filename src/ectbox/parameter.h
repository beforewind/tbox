#ifndef PARAMETER_H
#define PARAMETER_H


//#define REQ_PDO_MODE
//#define RESP_PDO_MODE

//pdo index
#define INDX_req_syncPDO  0x00
#define INDX_req_sequence 0x01  // no use
#define INDX_req_cmd      0x02
#define INDX_req_bank     0x03
#define INDX_req_index    0x04
#define INDX_req_reserved 0x05
#define INDX_req_data0    0x06  // 32bit, should be no use
#define INDX_req_data1    0x07


#define INDX_resp_syncPDO   0x00
#define INDX_resp_sequence  0x01  // no use
#define INDX_resp_status    0x02
#define INDX_resp_result    0x03
#define INDX_resp_data0     0x04
#define INDX_resp_data1     0x05
#define INDX_resp_data2     0x06
#define INDX_resp_data3     0x07



// new pdo index
#define INDX_SYNC_PDO   (0)
#define INDX_STAT_PDO   (2)  // TODO
#define INDX_DATA_PDO   (4)  // TODO



//sdo index
#define INDX_SDO_RES 0x9000
//#define MAX_SDO_RES_SIZE  (16*4)  // 64bytes

#define INDX_SDO_REQ 0x8000
//#define MAX_SDO_REQ_SIZE  (16*4)  // 64bytes

#define INDX_PARA_SDO  0x8000  // 参数块 //



//parameter
#define EC_TIMEOUTMON 2000

#endif // PARAMETER_H
