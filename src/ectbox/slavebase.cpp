
//  soem_interface
#include <ectbox/slavebase.h>

SlaveBase::SlaveBase(BusBase* bus, const uint32_t address) : bus_(bus), address_(address) {}
SlaveBase::SlaveBase() : bus_(nullptr), address_(0) {}


bool SlaveBase::sendSdoReadGeneric(const std::string& indexString, const std::string& subindexString,
                                           const std::string& valueTypeString, std::string& valueString) {
  printWarnNotImplemented();
  return false;
}

bool SlaveBase::sendSdoWriteGeneric(const std::string& indexString, const std::string& subindexString,
                                            const std::string& valueTypeString, const std::string& valueString) {
  printWarnNotImplemented();
  return false;
}

bool SlaveBase::sendSdoReadVisibleString(const uint16_t index, const uint8_t subindex, std::string& value) {
//  std::lock_guard<std::recursive_mutex> lock(mutex_);
//  return bus_->sendSdoReadVisibleString(address_, index, subindex, value);
  printWarnNotImplemented();
  return false;
}

