#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

class List {
  public:
    int16_t list[64];
    uint16_t length = 0;

    void clear() { 
      length = 0; 
    }

    int16_t get_item(int index) { 
      if(index < length) {
        return list[index];
      }
      else {
        return -1;
      } 
    }


    int get_index(int16_t item) {
      if(length == 0)
      { return -1; }

      for(int i=0; i<length; i++)
      { 
        if(list[i] == item)
        { return i; }
      }
      return -1;
    }


    void append(int16_t item) {
      if(length<64)
      {
        list[length] = item;
        length++;
      }
    }

    void clear_item(int index) {
      if(index < length) {
        list[index] = -1;
      }
    }
};