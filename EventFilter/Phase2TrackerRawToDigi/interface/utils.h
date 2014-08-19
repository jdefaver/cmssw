#ifndef EventFilter_Phase2TrackerRawToDigi_utils_H // {
#define EventFilter_Phase2TrackerRawToDigi_utils_H

// common tools
#include <iomanip>
#include <ostream>
#include <iostream>
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"

namespace Phase2Tracker {

  // TODO: set this in a common include file.
  // see DataFormats/Phase2TrackerCommon/interface/Constants.h

  // -------------------- FED ids --------------------

  static const uint16_t FED_ID_MIN     = static_cast<uint16_t>( FEDNumbering::MINSiStripFEDID);
  static const uint16_t FED_ID_MAX     = static_cast<uint16_t>( FEDNumbering::MAXSiStripFEDID);
  static const uint16_t CMS_FED_ID_MAX = static_cast<uint16_t>( FEDNumbering::MAXFEDID);
  static const uint16_t NUMBER_OF_FEDS = static_cast<uint16_t>( FED_ID_MAX - FED_ID_MIN + 1 );

  // Assumptions for phase 2

  static const int MAX_FE_PER_FED = 16;
  static const int MAX_CBC_PER_FE = 16;
  static const int STRIPS_PER_CBC = 254;
  static const int STRIPS_PADDING = 2;
  static const int TRIGGER_SIZE = 0; 
  static const int P_CLUSTER_SIZE_BITS = 18;
  static const int S_CLUSTER_SIZE_BITS = 15;

  // definition

  static const uint8_t INVALID=0xFF;

  // utils

  inline void printNibbleValue(uint8_t value, std::ostream& os)
  { 
    const std::ios_base::fmtflags originalFormatFlags = os.flags();
    os << std::hex <<  std::setw(1) << value;
    os.flags(originalFormatFlags);
  }

  inline void printHexValue(const uint8_t value, std::ostream& os)
  {
    const std::ios_base::fmtflags originalFormatFlags = os.flags();
    os << std::hex << std::setfill('0') << std::setw(2);
    os << uint16_t(value);
    os.flags(originalFormatFlags);
  }

  inline void printHexWord(const uint8_t* pointer, const size_t lengthInBytes, std::ostream& os)
  {
    size_t i = lengthInBytes-1;
    do{
      printHexValue(pointer[i],os);
      if (i != 0) os << " ";
    } while (i-- != 0);
  }

  inline void printHex(const void* pointer, const size_t lengthInBytes, std::ostream& os)
  {
    const uint8_t* bytePointer = reinterpret_cast<const uint8_t*>(pointer);
    //if there is one 64 bit word or less, print it out
    if (lengthInBytes <= 8) {
      printHexWord(bytePointer,lengthInBytes,os);
    }
    //otherwise, print word numbers etc
    else {
      //header
      os << "word\tbyte\t                       \t\tbyte" << std::endl;;
      const size_t words = lengthInBytes/8;
      const size_t extraBytes = lengthInBytes - 8*words;
      //print full words
      for (size_t w = 0; w < words; w++) {
        const size_t startByte = w*8;
        os << w << '\t' << startByte+8 << '\t';
        printHexWord(bytePointer+startByte,8,os);
        os << "\t\t" << startByte << std::endl;
      }
      //print part word, if any
      if (extraBytes) {
        const size_t startByte = words*8;
        os << words << '\t' << startByte+8 << '\t';
        //padding
        size_t p = 8;
        while (p-- > extraBytes) {
          os << "00 ";
        }
        printHexWord(bytePointer+startByte,extraBytes,os);
        os << "\t\t" << startByte << std::endl;
      }
      os << std::endl;
    }
  }


  //enum values are values which appear in FED buffer. DO NOT CHANGE!
  enum FEDReadoutMode { READOUT_MODE_INVALID=INVALID,
                        READOUT_MODE_SCOPE=0x1,
                        READOUT_MODE_VIRGIN_RAW=0x2,
                        READOUT_MODE_PROC_RAW=0x6,
                        READOUT_MODE_ZERO_SUPPRESSED=0xA,
                        READOUT_MODE_ZERO_SUPPRESSED_LITE=0xC,
                        READOUT_MODE_SPY=0xE
                      };

  //to make enums printable
  std::ostream& operator<<(std::ostream& os, const FEDReadoutMode& value);
  inline std::ostream& operator<<(std::ostream& os, const FEDReadoutMode& value)
  {
    switch (value) {
    case READOUT_MODE_SCOPE:
      os << "Scope mode";
      break;
    case READOUT_MODE_VIRGIN_RAW:
      os << "Virgin raw";
      break;
    case READOUT_MODE_PROC_RAW:
      os << "Processed raw";
      break;
    case READOUT_MODE_ZERO_SUPPRESSED:
      os << "Zero suppressed";
      break;
    case READOUT_MODE_ZERO_SUPPRESSED_LITE:
      os << "Zero suppressed lite";
      break;
    case READOUT_MODE_SPY:
      os << "Spy channel";
      break;
    case READOUT_MODE_INVALID:
      os << "Invalid";
      break;
    default:
      os << "Unrecognized";
      os << " (";
      printHexValue(value,os);
      os << ")";
      break;
    }
    return os;
  }

  // tracker header read modes
  enum READ_MODE
  {
    READ_MODE_INVALID = INVALID,
    SUMMARY    = 0,
    FULL_DEBUG = 1,
    CBC_ERROR  = 2
  };

  // module types
  enum MODULE_TYPE
  {
      UNUSED = -1,
      MODULE_2S = 0,
      MODULE_PS = 1
  };

  //to make enums printable
  std::ostream& operator<<(std::ostream& os, const READ_MODE& value);
  inline std::ostream& operator<<(std::ostream& os, const READ_MODE& value)
  {
    switch (value) {
    case SUMMARY:
      os << "Summary mode";
      break;
    case FULL_DEBUG:
      os << "Full debug mode";
      break;
    case CBC_ERROR:
      os << "CBC error mode";
      break;
    default:
      os << "Unrecognized mode";
      os << " (";
      printHexValue(value,os);
      os << ")";
      break;
    }
    return os;
  }



  // tracker header masks
  enum trackerHeader_m { VERSION_M       = 0xF000000000000000,
                         HEADER_FORMAT_M = 0x0C00000000000000,
                         EVENT_TYPE_M    = 0x03C0000000000000,
                         GLIB_STATUS_M   = 0x003FFFFFFFFF0000,
                         FRONTEND_STAT_M = 0x000000000000FFFF,
                         CBC_NUMBER_M    = 0xFFFF000000000000 
                       };

  // position of first bit
  enum trackerHeader_s { VERSION_S       = 60,
                         HEADER_FORMAT_S = 58,
                         EVENT_TYPE_S    = 54,
                         GLIB_STATUS_S   = 24,
                         CBC_NUMBER_S    = 8,
                         FRONTEND_STAT_S = 0
                       };

  // number of bits (replaces mask)
  enum trackerheader_l { VERSION_L       = 4,
                         HEADER_FORMAT_L = 2,
                         EVENT_TYPE_L    = 4,
                         GLIB_STATUS_L   = 30,
                         CBC_NUMBER_L    = 16,
                         FRONTEND_STAT_L = 0
                       };

  // get 64 bits word from data with given offset : only use if at beginning of 64 bits word 
  inline uint64_t read64(int offset, const uint8_t* buffer)
  {
    return *reinterpret_cast<const uint64_t*>(buffer+offset);
  }

  // extract data from a 64 bits word using mask and shift
  inline uint64_t extract64(trackerHeader_m mask,trackerHeader_s shift, uint64_t data)
  {  
    data = (data & mask) >> shift;
    return data;
  }

  inline uint64_t read_n_at_m(const uint8_t* buffer, int size, int pos_bit)
  {
    // 1) determine which 64 bit word to read
    int iword = pos_bit/64;
    uint64_t data = *(uint64_t*)(buffer+(iword*8));
    data >>= pos_bit % 64;

    // 2) determine if you need to read another
    int end_bit = pos_bit % 64 + size;
    if(end_bit > 64) {
        data |=  *(uint64_t*)(buffer+((iword+1)*8)) << (end_bit - 64);
    }
    
    // 3) mask according to expected size
    data &= (uint64_t)((1LL<<size)-1);
    return data;
  }

} // end of Phase2Tracker namespace

#endif // } end def utils

