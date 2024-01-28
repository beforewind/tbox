
#ifndef SLAVEBASE_H
#define SLAVEBASE_H

// std
#include <cstdint>
#include <memory>
#include <mutex>
#include <typeinfo>

#include <ectbox/busbase.h>

/**
 * @brief      Base class for generic ethercat slaves using soem
 */
class SlaveBase {
 public:
  /**
   * @brief      Struct defining the Pdo characteristic
   */
  struct PdoInfo {
    // The id of the rx pdo
    uint16_t rxPdoId_ = 0;
    // The id of the tx pdo
    uint16_t txPdoId_ = 0;
    // The size of the rx pdo
    uint16_t rxPdoSize_ = 0;
    // The size of the tx pdo
    uint16_t txPdoSize_ = 0;
    // The value referencing the current pdo type on slave side
    uint32_t moduleId_ = 0;
  };

  SlaveBase(BusBase* bus, const uint32_t address);
  SlaveBase();
  virtual ~SlaveBase() = default;

  /**
   * @brief      Returns the name of the slave.
   *
   * @return     Name of the ethercat bus
   */
  virtual std::string getName() const = 0;

  /**
   * @brief      Startup non-ethercat specific objects for the slave
   *
   * @return     True if succesful
   */
  virtual bool startup() = 0;

  /**
   * @brief      Called during reading the ethercat bus. Use this method to
   *             extract readings from the ethercat bus buffer
   */
  virtual int updateRead() = 0;

  /**
   * @brief      Called during writing to the ethercat bus. Use this method to
   *             stage a command for the slave
   */
  virtual void updateWrite() = 0;

  /**
   * @brief      Used to shutdown slave specific objects
   */
  virtual void shutdown() = 0;

  /**
   * @brief      Gets the current pdo information.
   *
   * @return     The current pdo information.
   */
  virtual PdoInfo getCurrentPdoInfo() const = 0;

  /**
   * @brief      Set BusBase pointer
   */
  void setBusBasePointer(BusBase* bus){bus_ = bus;}

  /**
   * @brief      Set physical EtherCAT address
   */
  void setEthercatAddress(uint32_t address){address_ = address;}

  /**
   * @brief      Returns the bus address of the slave
   *
   * @return     Bus address.
   */
  uint32_t getAddress() const { return address_; }

  /*!
   * Send a writing SDO.
   * @param index          Index of the SDO.
   * @param subindex       Sub-index of the SDO.
   * @param completeAccess Access all sub-inidices at once.
   * @param value          Value to write.
   * @return True if successful.
   */
  template <typename Value>
  bool sendSdoWrite(const uint16_t index, const uint8_t subindex, const bool completeAccess, const Value value) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    //const bool success =  bus_->sendSdoWrite(address_, index, subindex, completeAccess, value);
    const bool success = false;
    if(!success) {
      MELO_ERROR_STREAM("Error writing SDO.\tAddress: " << address_ << "Index: " << (int)index
                        << "\nSubindex: " << (int)subindex << "\n Complete Access: "
                        << (int)completeAccess << "\nType: " << typeid(value).name());
    }
    return success;
  }

  /*!
   * Send a reading SDO.
   * @param index          Index of the SDO.
   * @param subindex       Sub-index of the SDO.
   * @param completeAccess Access all sub-inidices at once.
   * @param value          Return argument, will contain the value which was read.
   * @return True if successful.
   */
  template <typename Value>
  bool sendSdoRead(const uint16_t index, const uint8_t subindex, const bool completeAccess, Value& value) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    //const bool success = bus_->sendSdoRead(address_, index, subindex, completeAccess, value);
    const bool success = false;
    if(!success) {
      MELO_ERROR_STREAM("Error reading SDO.\tAddress: " << address_ << "Index: " << (int)index
                        << "\nSubindex: " << (int)subindex << "\n Complete Access: "
                        << (int)completeAccess << "\nType: " << typeid(value).name());
    }
    return success;
  }

  /**
   * Send a generic reading SDO.
   * @warning Not implemented!
   */
  virtual bool sendSdoReadGeneric(const std::string& indexString, const std::string& subindexString, const std::string& valueTypeString,
                                  std::string& valueString);
  /**
   * Send a generic writing SDO.
   * @warning Not implemented!
   */
  virtual bool sendSdoWriteGeneric(const std::string& indexString, const std::string& subindexString, const std::string& valueTypeString,
                                   const std::string& valueString);

  /**
   * Send a special reading SDO to read SDOs of type visible string.
   * @param index          Index of the SDO.
   * @param subindex       Sub-index of the SDO.
   * @param value          Return argument, will contain the value which was read.
   * @return True if successful.
   */
  virtual bool sendSdoReadVisibleString(const uint16_t index, const uint8_t subindex, std::string& value);

  /**
   * Type-suffixed SDO calls.
   * @deprecated Use the templated sendSdoRead<...> and sendSdoWrite<...> instead.
   */
  virtual bool sendSdoReadInt8(const uint16_t index, const uint8_t subindex, const bool completeAccess, int8_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadInt16(const uint16_t index, const uint8_t subindex, const bool completeAccess, int16_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadInt32(const uint16_t index, const uint8_t subindex, const bool completeAccess, int32_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadInt64(const uint16_t index, const uint8_t subindex, const bool completeAccess, int64_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadUInt8(const uint16_t index, const uint8_t subindex, const bool completeAccess, uint8_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadUInt16(const uint16_t index, const uint8_t subindex, const bool completeAccess, uint16_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadUInt32(const uint16_t index, const uint8_t subindex, const bool completeAccess, uint32_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadUInt64(const uint16_t index, const uint8_t subindex, const bool completeAccess, uint64_t& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadFloat(const uint16_t index, const uint8_t subindex, const bool completeAccess, float& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadDouble(const uint16_t index, const uint8_t subindex, const bool completeAccess, double& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoReadString(const uint16_t index, const uint8_t subindex, const bool completeAccess, std::string& value) {
    return sendSdoRead(index, subindex, completeAccess, value);
  }

  virtual bool sendSdoWriteInt8(const uint16_t index, const uint8_t subindex, const bool completeAccess, const int8_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteInt16(const uint16_t index, const uint8_t subindex, const bool completeAccess, const int16_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteInt32(const uint16_t index, const uint8_t subindex, const bool completeAccess, const int32_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteInt64(const uint16_t index, const uint8_t subindex, const bool completeAccess, const int64_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteUInt8(const uint16_t index, const uint8_t subindex, const bool completeAccess, const uint8_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteUInt16(const uint16_t index, const uint8_t subindex, const bool completeAccess, const uint16_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteUInt32(const uint16_t index, const uint8_t subindex, const bool completeAccess, const uint32_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteUInt64(const uint16_t index, const uint8_t subindex, const bool completeAccess, const uint64_t value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteFloat(const uint16_t index, const uint8_t subindex, const bool completeAccess, const float value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteDouble(const uint16_t index, const uint8_t subindex, const bool completeAccess, const double value) {
    return sendSdoWrite(index, subindex, false, value);
  }

  virtual bool sendSdoWriteString(const uint16_t index, const uint8_t subindex, const bool completeAccess, const std::string value) {
      return sendSdoWrite(index, subindex, false, value);
  }

 protected:
  /**
   * @brief      Prints a warning. Use this method to suppress compiler warnings
   */
  void printWarnNotImplemented() { MELO_WARN_STREAM("Functionality is not implemented."); }

  // Mutex prohibiting simultaneous access to EtherCAT slave.
  mutable std::recursive_mutex mutex_;
  // Non owning pointer to the ethercat bus
    BusBase* bus_{nullptr};
  // The bus address
  uint32_t address_{0};
};

//using SlaveBasePtr = std::shared_ptr<SlaveBase>;


#endif  // SLAVEBASE_H
