// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Helper functions for using the last page of flash for non-volatile storage.

#ifndef STMLIB_SYSTEM_STORAGE_H_
#define STMLIB_SYSTEM_STORAGE_H_

#include <stm32f10x_conf.h>

#include "stmlib/stmlib.h"
#include "stmlib/system/flash_programming.h"

namespace stmlib {

template<uint32_t last_address = 0x8020000>
class Storage {
 public:
  enum {
    FLASH_STORAGE_BASE = last_address - PAGE_SIZE
  };
  
  template<typename T>
  static void Save(const T& data) {
    FLASH_Unlock();
    FLASH_ErasePage(FLASH_STORAGE_BASE);
    
    // Write argument.
    const uint32_t* words = (const uint32_t*)(&data);
    size_t size = sizeof(T);
    uint32_t address = FLASH_STORAGE_BASE;
    while (size >= 4) {
      FLASH_ProgramWord(address, *words++);
      address += 4;
      size -= 4;
    }
    // If the size is not a multiple of 4, write tail.
    if (size) {
      FLASH_ProgramWord(address, *(const uint16_t*)(words));
    }
    
    // Write checksum.
    uint16_t checksum = Checksum((const void*)(&data), sizeof(T));
    FLASH_ProgramHalfWord(FLASH_STORAGE_BASE + sizeof(T), checksum);
  };
  
  template<typename T>
  static bool Load(T* data) {
    memcpy(data, (void*)(FLASH_STORAGE_BASE), sizeof(T));
    uint16_t checksum = (*(uint16_t*)(FLASH_STORAGE_BASE + sizeof(T)));
    return checksum == Checksum(data, sizeof(T));
  };
  
 private:
  static uint16_t Checksum(const void* data, uint16_t size) {
    uint16_t s = 0;
    const uint8_t* d = static_cast<const uint8_t*>(data);
    while (size--) {
      s += *d++;
    }
    return s;
  }
};

};  // namespace stmlib

#endif  // STMLIB_SYSTEM_STORAGE_H_