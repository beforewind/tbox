#ifndef TBOX_HELPER_H

void updateFirmware(Tbox *tbox, const std::string &fileName);
void test_foeFlashMM(Tbox *tbox);

void initDevice(Tbox *tbox);
int readDevice(Tbox *tbox);
#define TBOX_HELPER_H

#include "include/tbox.h"

std::string getFileName(const std::string &filePathName);
bool isFileExists(const std::string &filePathName);
void readFile(const std::string &filePathName, std::vector<uint8_t> &fileData);

void printArray(std::vector<uint8_t> array);
void printInt(std::vector<uint8_t> array);
uint32_t convert2Int(std::vector<uint8_t> array);
void printServiceObject(Tbox *tbox, std::string &objName);

void reqCallback(Request req);
void rspCallback(Response rsp);
void reqIdCallback(int32_t reqId);
void rspIdCallback(int32_t rspId);
void printRsp(Response rsp);
void waitSync();
void readHardware(Tbox *tbox);
void writeParameter(Tbox *tbox);
void writeParameter2(Tbox *tbox, uint32_t curr);
void readPdoInfo(Tbox *tbox);
Request generateSetReqest(Tbox *tbox);
Request generateGetRequest(Tbox *tbox);

/////////////////////////////////////////////////////////

typedef struct
{
	uint16_t id;

	// device info
	// ServiceObject deviceObj;
	uint32_t deviceSN;
	int32_t adapterCount;
	uint32_t adapterSN[8];

	// hardware info
	// ServiceObject hardObj;
	uint32_t hversion;
	uint32_t fversion;
	uint32_t dinCount;
	uint32_t doutCount;
	uint32_t bankCount;
	uint32_t ioCount;

	// measure obj
	ServiceObject measureObj;
	ServiceObject generalObj;

	// parameter info
	ServiceObject paraObj;
	uint32_t current;
	uint32_t voltage;
	uint32_t delayus;
	uint32_t rsample;

	// pdo info
	// ServiceObject doObj;
	int32_t doSize;

	// ServiceObject diObj;
	int32_t diSize;

	// ServiceObject txStimuliObj;
	int32_t txStimuliSize;

	// ServiceObject rxFeedbackObj;
	int32_t rxFeedbackSize;

} device_t;

uint32_t getDeviceObjectItem(Tbox *tbox, uint16_t deviceId, const std::string &objName, const std::string &itemName);
int32_t getDeviceObjectSize(Tbox *tbox, uint16_t deviceId, const std::string &objName);
void readDeviceList(Tbox *tbox, std::map<uint32_t, device_t> &deviceMap);
void saveDeviceList(Tbox *tbox, std::map<uint32_t, device_t> &deviceMap);

/////////////////////////////////////////////////////////
// demo of adapters
typedef struct
{
	uint32_t adapterSN;
	uint32_t deviceSN;
	// device_t device;

	uint32_t ioCount;
	uint32_t diCount;
	uint32_t doCount;

	std::map<uint32_t, uint32_t> ioMap; // adapter.io -> device.io
	std::map<uint32_t, uint32_t> diMap; // adapter.di <- device.di
	std::map<uint32_t, uint32_t> doMap; // adapter.do -> device.do

} adapter_t;

void generateAdapterExample(std::map<uint32_t, adapter_t> &adapterMap, std::map<uint32_t, device_t> &deviceMap);
void generateAdapterMappingExample(std::map<uint32_t, adapter_t> &adapterMap);

/////////////////////////////////////////////////////////
typedef struct
{
	std::string name;
	adapter_t adpFrom;
	adapter_t adpTo;
	uint32_t pinFrom;
	uint32_t pinTo;
} connection_t;

void generateWireExample(std::map<uint32_t, adapter_t> &adapterMap, std::vector<connection_t> &wireMap);

/////////////////////////////////////////////////////////
// example for generate request
void generatePatternExample(std::map<uint32_t, device_t> &deviceMap, std::map<uint16_t, std::vector<uint8_t>> &doutPattern, std::map<uint16_t, std::vector<uint8_t>> &stimuliPattern);
Request generateSetReqestExample(Tbox *tbox, std::map<uint16_t, std::vector<uint8_t>> &doutPattern, std::map<uint16_t, std::vector<uint8_t>> &stimuliPattern);
Request generateGetRequestExample(Tbox *tbox, std::map<uint16_t, std::vector<uint8_t>> &doutPattern);

/////////////////////////////////////////////////////////

typedef struct
{
	uint32_t type; // 0x100-resistor,0x200-capacitor,0x300-diode...
	uint16_t deviceFrom;
	uint16_t deviceTo;
	uint16_t pinFrom;
	uint16_t pinTo;
	uint32_t nominalValue;
	uint32_t measuredValue;
	uint32_t unit; // TODO: 标称单位，如电阻-欧姆，电容-pF //
} component2t_t;   // 2-terminal components, resistor,capacitor,diode

// components test
void testComponent(Tbox *tbox, std::vector<component2t_t> &resList);
// void testRelay(Tbox* tbox);

#endif
