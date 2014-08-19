#include "EventFilter/Phase2TrackerRawToDigi/interface/Phase2TrackerFEDHeader.h"
#include "EventFilter/Phase2TrackerRawToDigi/interface/utils.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

namespace Phase2Tracker
{

  Phase2TrackerFEDHeader::Phase2TrackerFEDHeader(const uint8_t* headerPointer) 
    : trackerHeader_(headerPointer)
  {
    header_first_word_  = read64(0,trackerHeader_);
    header_second_word_ = read64(8,trackerHeader_);
    // decode the Tracker Header and store info
    init();
  }
  
  void Phase2TrackerFEDHeader::init() 
  {
    dataFormatVersion_ = dataFormatVersion();
    debugMode_ = debugMode();
    // WARNING: eventType must be called before 
    // readoutMode, conditionData and dataType
    // as this info is stored in eventType
    eventType_ = eventType();
    readoutMode_ = readoutMode();
    conditionData_ = conditionData();
    dataType_ = dataType();

    glibStatusCode_ = glibStatusCode();
    // numberOfCBC must be called before pointerToData
    numberOfCBC_ = numberOfCBC();
    pointerToData_ = pointerToData();
    
    LogTrace("Phase2TrackerFEDBuffer")
        << "[Phase2Tracker::Phase2TrackerFEDHeader::"<<__func__<<"]: \n"
        <<" Tracker Header contents:\n"
        <<"  -- Data Format Version : " << uint32_t(dataFormatVersion_) << "\n"
        <<"  -- Debug Level         : " << debugMode_ << "\n"
        <<"  -- Operating Mode      : " << readoutMode_ << "\n"
        <<"  -- Condition Data      : " << ( conditionData_ ? "Present" : "Absent") << "\n"
        <<"  -- Data Type           : " << ( dataType_ ? "Real" : "Fake" ) << "\n"
        <<"  -- Glib Stat registers : " <<  std::hex << std::setw(16) << glibStatusCode_ << "\n"
        <<"  -- connected CBC       : " <<  std::dec << numberOfCBC_ << "\n";
  }

  uint8_t Phase2TrackerFEDHeader::dataFormatVersion() const
  {
    uint8_t Version = static_cast<uint8_t>(read_n_at_m(trackerHeader_,VERSION_L,VERSION_S));
    if (Version != 1)
    {
      std::ostringstream ss;
      ss << "[Phase2Tracker::Phase2TrackerFEDHeader::"<<__func__<<"] ";
      ss << "Invalid Data Format Version in Traker Header : ";
      printHex(&header_first_word_,1,ss);
      throw cms::Exception("Phase2TrackerFEDBuffer") << ss.str();
    }
    return Version;
  }
  
  READ_MODE Phase2TrackerFEDHeader::debugMode() const
  {
    // Read debugMode in Tracker Header
    uint8_t mode = static_cast<uint8_t>(read_n_at_m(trackerHeader_,HEADER_FORMAT_L,HEADER_FORMAT_S));
    
    switch (mode)
    { // check if it is one of correct modes
      case SUMMARY:
        return READ_MODE(SUMMARY);
      case FULL_DEBUG:
        return READ_MODE(FULL_DEBUG);
      case CBC_ERROR:
        return READ_MODE(CBC_ERROR);
      default: // else create Exception
        std::ostringstream ss;
        ss << "[Phase2Tracker::Phase2TrackerFEDHeader::"<<__func__<<"] ";
        ss << "Invalid Header Format in Traker Header : ";
        printHex(&header_first_word_,1,ss);
        throw cms::Exception("Phase2TrackerFEDBuffer") << ss.str();
    }

    return READ_MODE(READ_MODE_INVALID);
  }
  
  uint8_t Phase2TrackerFEDHeader::eventType() const
  {
    return static_cast<uint8_t>(read_n_at_m(trackerHeader_,EVENT_TYPE_L,EVENT_TYPE_S));
  }

  // decode eventType_. Read: readoutMode, conditionData and dataType
  FEDReadoutMode Phase2TrackerFEDHeader::readoutMode() const
  {
    // readout mode is first bit of event type
    uint8_t mode = static_cast<uint8_t> (eventType_ >> 2) & 0x3;
    
    switch (mode)
    { // check if it is one of correct modes
      case 2:
        return FEDReadoutMode(READOUT_MODE_PROC_RAW);
      case 1:
        return FEDReadoutMode(READOUT_MODE_ZERO_SUPPRESSED);
      default: // else create Exception
        std::ostringstream ss;
        ss << "[Phase2Tracker::Phase2TrackerFEDHeader::"<<__func__<<"] ";
        ss << "Invalid Readout Mode in Traker Header : ";
        printHex(&header_first_word_,1,ss);
        throw cms::Exception("Phase2TrackerFEDBuffer") << ss.str();
    }
  }

  uint8_t Phase2TrackerFEDHeader::conditionData() const
  {
    return static_cast<uint8_t> (eventType_ >>1) & 0x1;
  }

  uint8_t Phase2TrackerFEDHeader::dataType() const
  {
    return static_cast<uint8_t> (eventType_) & 0x1;
  }
  
  uint64_t Phase2TrackerFEDHeader::glibStatusCode() const
  {
    return read_n_at_m(trackerHeader_,GLIB_STATUS_L,GLIB_STATUS_S);
  }
  
  std::vector<bool> Phase2TrackerFEDHeader::frontendStatus() const
  {
    uint8_t   fe_status_0 = (uint8_t)(read_n_at_m(trackerHeader_,8,0));
    uint64_t  fe_status_1 = *(uint64_t*)(trackerHeader_+8);
    std::vector<bool> status(72,false);
    for(int i = 0; i < 72; i++)
    {
      if(i<8) 
      { 
        status[i] = (fe_status_0>>i)&0x1; 
      }
      else
      {
        status[i] = (fe_status_1>>(i-8))&0x1;
      }
    }
    return status;
  }
  
  uint16_t Phase2TrackerFEDHeader::numberOfCBC() const
  {
    if(debugMode_!=SUMMARY)
    {
      return static_cast<uint16_t>(read_n_at_m(trackerHeader_,CBC_NUMBER_L,CBC_NUMBER_S));
    }
    else
    {
      return 0;
    }
  }
  
  std::vector<uint8_t> Phase2TrackerFEDHeader::CBCStatus() const
  {
    // set offset and data to begining 
    int offset_bits = 128;
    // number of CBC:
    uint16_t cbc_num = numberOfCBC();
    // size of data per CBC (in bits)
    int status_size = 0;
    if (debugMode_==FULL_DEBUG)
    {
      status_size = 8;
    }
    else if (debugMode_==CBC_ERROR)
    {
      status_size = 1;
    }
    // starting byte for CBC status bits
    std::vector<uint8_t> cbc_status;
    while(cbc_num>0)
    {
        cbc_status.push_back(static_cast<uint8_t>(read_n_at_m(trackerHeader_,status_size,offset_bits)));
        cbc_num--;
        offset_bits += status_size;
    }
    return cbc_status;
  }
  
  const uint8_t* Phase2TrackerFEDHeader::pointerToData()
  {
    int status_size = 0;
    int cbc_num = numberOfCBC();
    // all sizes in bits here
    if (debugMode_==FULL_DEBUG)
    {
      status_size = 8;
    }
    else if (debugMode_==CBC_ERROR)
    {
      status_size = 1;
    }
    // compute number of additional 64 bit words before payload
    int num_add_words64 = (cbc_num * status_size + 64 - 1) / 64 ;
    // back to bytes
    trackerHeaderSize_ = (2 + num_add_words64) * 8;
    return &trackerHeader_[trackerHeaderSize_];
  }

}  // end of Phase2Tracker namespace
