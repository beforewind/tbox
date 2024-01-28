
#include <iostream>
#include "utils/cmdline.h"
#include "include/tbox.h"
#include "example/tbox_helper.h"

int main(int argc, char **argv)
{
    uint16_t deviceId = 0;  // 0 for all devices
    std::string filePathName = "";
    uint32_t nicId = 3;

    // create an arg parser
    cmdline::parser a;
    // add specified type of variable.
    a.add<std::string>("firmware", 'f', "firmware path and name", true, "");
    a.add<uint16_t>("device", 'd', "id of device to update, 0 for all", true, 0, cmdline::range(0, 65535));
    a.add<uint32_t>("nic", 'n', "id of nic to connect", true, 3, cmdline::range(0, 65535));
    // Run parser.
    a.parse_check(argc, argv);
    // use flag values
    filePathName = a.get<std::string>("firmware");
    deviceId = a.get<uint16_t>("device");
    nicId = a.get<uint32_t>("nic");


    // first connect nic, and print deviceCount
    Tbox* tbox = loadTbox("EcTbox");  // EcTbox.dll
    if (tbox == nullptr) {
        std::cout << "Failed loadTbox" << std::endl;
        return 0;
    }

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
    
    int32_t rt = tbox->connect(nicNameList[nicId]);
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
    if (deviceId > deviceCount) {
        std::cout << "no device with id:" << deviceId << std::endl;

        unloadTbox(&tbox);
        return 0;
    }
    
    std::string fileName = getFileName(filePathName);
    //int32_t fileSize;
    std::vector<uint8_t> fileData;
    readFile(filePathName, fileData);
    if (fileData.size() <= 0) {
        std::cout << "Failed read file:" << filePathName << std::endl;

        unloadTbox(&tbox);
        return 0;
    }

    if (deviceId == 0) {  // update all devices
        for (uint16_t id = 1; id <= tbox->getDeviceCount(); id++) {
            std::cout << "writing firmware:" << fileName << " into device:" << id << std::endl;
            tbox->updateFirmware(id, fileName, fileData);
        }
    }
    else {  // update the selected device
        std::cout << "writing firmware:" << fileName << " into device:" << deviceId << std::endl;
        tbox->updateFirmware(deviceId, fileName, fileData);
    }
	
    unloadTbox(&tbox);
    return 0;
}
