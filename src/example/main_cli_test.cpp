

#include <iostream>
#include <functional>
#include <chrono>

#include "utils/cmdline.h"
#include "include/tbox.h"
#include "example/tbox_helper.h"

#ifdef TBOX_DLL
#else
#include "ectbox/ectbox.h"
#endif

///////////////////////////////////////////////////
// follow the execution progress of req/rsp
extern int32_t reqCounter;
extern int32_t rspCounter;

///////////////////////////////////////////////////

int main(int argc, char **argv)
{
    uint32_t nicId = 3;
    int test_count = 3;

    // create an arg parser
    cmdline::parser a;
    // add specified type of variable.
    // a.add<std::string>("firmware", 'f', "firmware path and name", true, "");
    // a.add<uint16_t>("device", 'd', "id of device to update, 0 for all", true, 0, cmdline::range(0, 65535));
    a.add<uint32_t>("nic", 'n', "id of nic to connect", true, 3, cmdline::range(0, 65535));
    a.add<uint32_t>("count", 'c', "count number to test", false, 3, cmdline::range(3, 10000));

    // Run parser.
    a.parse_check(argc, argv);
    // use flag values
    // filePathName = a.get<std::string>("firmware");
    // deviceId = a.get<uint16_t>("device");
    nicId = a.get<uint32_t>("nic");
    test_count = a.get<uint32_t>("count");

#ifdef TBOX_DLL
    Tbox *tbox = loadTbox("EcTbox"); // load EcTbox.dll, instantiate tbox
    if (tbox == nullptr)
    {
        std::cout << "Failed loadTbox" << std::endl;
        return 0;
    }
#else
    EcTbox *tbox = new EcTbox();
#endif

    // find all nics, select and connect to the EtherCAT bus //
    std::vector<std::string> nicNameList;
    nicNameList = tbox->findNic();
    if (nicId >= nicNameList.size())
    {
        std::cout << "no nic with id:" << nicId << std::endl;
        std::cout << "available nic list:" << std::endl;
        for (int i = 0; i < nicNameList.size(); i++)
        {
            std::cout << "    nic[" << i << "]:" << nicNameList.at(i) << std::endl;
        }

        unloadTbox(&tbox);
        return 0;
    }

    int32_t rt = tbox->connect(nicNameList[nicId]); // select your EtherCAT nic
    if (rt < 0)
    {
        std::cout << "Failed connecting Nic:" << nicNameList[nicId];
        std::cout << "available nic list:" << std::endl;
        for (int i = 0; i < nicNameList.size(); i++)
        {
            std::cout << "    nic[" << i << "]:" << nicNameList.at(i) << std::endl;
        }

        unloadTbox(&tbox);
        return 0;
    }

    int32_t deviceCount = tbox->getDeviceCount();
    std::cout << "find deviceCount=" << deviceCount << std::endl;


#if 1
    // user可用的存储器测试 //
    int32_t cnt = 3;
    while (cnt >0)
    {
        testUserMemory(tbox);
        cnt--;
    }
#endif

#if 0
    // write and read device info: deviceSN, adapterCount, adapterSN0, adapterSN1, ... //
    // initDevice(tbox);
    if (readDevice(tbox) < 0)
    {
        initDevice(tbox);
        readDevice(tbox);
    }
#endif

#if 1
    // read hardware info: ioCount, bankCount ...
    readHardware(tbox);

    // write and read parameters: current, voltage, delayus ...
//    writeParameter(tbox);

    // read PDO info: digital input/output, tx ioStimuli / rx ioFeedback
    readPdoInfo(tbox);
#endif

    // 示例使用uint32_t作为device和adapter的序列号SN，这些SN是存储在MCU的FLASH中的 //
    std::map<uint32_t, device_t> deviceMap;   // deviceSN -> device_t
    std::map<uint32_t, adapter_t> adapterMap; // adapterSN -> adapter_t

    readDeviceList(tbox, deviceMap);

    // 测试flash读写 //
//    test_foeFlashMM(tbox);

#if 0
    // should be edited/import from Adapter database
    generateAdapterExample(adapterMap, deviceMap);
    saveDeviceList(tbox, deviceMap);
    readDeviceList(tbox, deviceMap);

    // should be edited by GUI
    generateAdapterMappingExample(adapterMap);  // mapping between device and adapter

    std::vector<connection_t> wireList;
    // should be generated from wire harness
    generateWireExample(adapterMap, wireList);
#endif

    std::map<uint16_t, std::vector<uint8_t>> stimuliPattern; // deviceId -> stimuliPattern
    std::map<uint16_t, std::vector<uint8_t>> doutPattern;    // deviceId -> doPattern
    generatePatternExample(deviceMap, doutPattern, stimuliPattern);

#if 1
    // set call back functions for req/rsp //
    tbox->setCallback(
        reqCallback,
        rspCallback);
    tbox->setIdCallback(
        reqIdCallback,
        rspIdCallback);
#endif

    // int test_count = 10;
    while (--test_count > 0)
    {
        std::cout << "test_count:" << test_count << std::endl;

        //
//        writeParameter(tbox);

        // start run thread of tbox //
        tbox->start();

        // 请求来自上层，上层程序根据前面的四种obj填充pattern //
        // 两类请求：set,get, set用于设置dout和io，get用于获取响应 ，可以连续多次set，之后可以连续多次get. set之后，所有device进入同步状态 //
        std::chrono::time_point<std::chrono::high_resolution_clock> timeStart = std::chrono::high_resolution_clock::now();
        int isSet = 1;
        int count = 0;
        while (count < 10)
        {
            // if (tbox->canAccept()>0)
            // while(1)
            {
                std::cout << "============================= count =" << count << " =============================" << std::endl;

                if (isSet == 1)
                {
                    std::chrono::time_point<std::chrono::high_resolution_clock> timeBeignSetReq = std::chrono::high_resolution_clock::now();
#if 0
                    Request req = generateSetReqest(tbox);
#else
                    Request req = generateSetReqestExample(tbox, doutPattern, stimuliPattern);
#endif
                    tbox->sendRequest(req);
                    isSet = 0;
                    waitSync(); // wait until the response is received, to measure timing for a pair of req/rsp //
                    std::chrono::time_point<std::chrono::high_resolution_clock> timeEndSetRsp = std::chrono::high_resolution_clock::now();
                    auto durationSet = (timeEndSetRsp - timeBeignSetReq);
                    std::cout << "time of set request/response:" << double(durationSet.count()) / 1000000.0 << " ms" << std::endl;
                }
                else
                {
                    std::chrono::time_point<std::chrono::high_resolution_clock> timeBeignGetReq = std::chrono::high_resolution_clock::now();
#if 0
                    Request req = generateGetRequest(tbox);
#else
                    Request req = generateGetRequestExample(tbox, doutPattern);
#endif
                    tbox->sendRequest(req);
                    isSet = 1;
                    count++;
                    waitSync();
                    std::chrono::time_point<std::chrono::high_resolution_clock> timeEndGetRsp = std::chrono::high_resolution_clock::now();
                    auto durationGet = (timeEndGetRsp - timeBeignGetReq);
                    std::cout << "time of get request/response:" << double(durationGet.count()) / 1000000.0 << " ms" << std::endl;
                }
            }
#if 1
            waitSync();

            // get dinStatus
            tbox->getDinStatus();

            // drop rsp
            if (isSet == 0)
            {
                tbox->dropResponse();
            }
            else
            {
                // print rsp
                Response rsp;
                while (tbox->recvResponse(rsp) == 1)
                {
                    printRsp(rsp);
                }
            }
#endif
        }

#if 1
        // rsp
        Response rsp;
        while (tbox->recvResponse(rsp) == 1)
        {
            printRsp(rsp);
        }
#endif

#if 0
        Response rsp;
        if (tbox->recvResponse(rsp) == 1) {
            std::cout << "rsp:" << rsp.reqId << std::endl;
        }
        Response rsp2;
        if (tbox->recvResponse(rsp2) == 1) {
            std::cout << "rsp2:" << rsp2.reqId << std::endl;
        }
        Response rsp3;
        if (tbox->recvResponse(rsp3) == 1) {
            std::cout << "rsp3:" << rsp3.reqId << std::endl;
        }
#endif
        std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd = std::chrono::high_resolution_clock::now();
        auto duration = (timeEnd - timeStart);
        std::cout << "total time:" << double(duration.count()) / 1000000.0 << " ms" << std::endl;
        std::cout << "average time per request:" << double(duration.count() / 1000000.0 / (count * 1.0)) << " ms" << std::endl;

        //
        writeParameter2(tbox, 5 + test_count);
        // writeParameter2(tbox, 20);

        // components test
        std::vector<component2t_t> resList;
        testComponent(tbox, resList);
        for (auto &comp : resList)
        {
            std::cout << "From device.pin:" << comp.deviceFrom << "." << comp.pinFrom << "  To device.pin:" << comp.deviceTo << "." << comp.pinTo << "  comp type:" << comp.type << ", measuredValue float:" << *(float_t *)&comp.measuredValue << ", measuredValue:0x" << comp.measuredValue  << ",  nominalValue:" << comp.nominalValue << std::endl;
        }

        // read parameter
        std::string paraName = "parameter";
        printServiceObject(tbox, paraName);

        // wait the last response
        // while(rspCounter!=reqCounter);
        // waitSync();

        // release test
        testRelease(tbox);

        tbox->stop();

        // test read obj after stop
#if 0
        std::string objName = "parameter";
        printServiceObject(tbox, objName);
#endif
    }

#if 0
    // free mode 测试
    // 调用set请求，设置IO状态
    // 进入free mode，周期获取IO状态
    {
        std::cout << "free mode test" << std::endl;
    }
#endif


    tbox->disconnect();

#ifdef TBOX_DLL
    unloadTbox(&tbox);
#else
#endif

    return 0;
}
