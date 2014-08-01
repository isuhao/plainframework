/**
 * PLAIN FRAMEWORK ( https://github.com/viticm/plainframework )
 * $Id define.h
 * @link https://github.com/viticm/plainframework for the canonical source repository
 * @copyright Copyright (c) 2014- viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm<viticm.ti@gmail.com>
 * @date 2014/07/30 17:23
 * @uses the sharememory define file
 */
#ifndef COMMON_SHAREMEMORY_DEFINE_H_
#define COMMON_SHAREMEMORY_DEFINE_H_

#include "common/sharememory/config.h"
#include "common/db/define.h"
#include "pf/sys/memory/share.h"

extern bool g_commond_exit = false;

namespace common {

namespace sharememory { //共享内存存储数据结构体

//-- static define
extern const uint32_t kManagerUnitDataMax = 5000; //如果不需要引入外部，就不要使用宏
extern const uint8_t kObjMax = 20;

typedef struct dataheader_struct {
  uint64_t key;
  uint32_t size;
  uint32_t version;
  dataheader_struct();
  ~dataheader_struct();
} dataheader_t;

typedef enum {
  kSmptShareMemory,
  kSmptCenter,
  kSmptServer,
} pooltype_t;

typedef enum {
  kUseStatusFree = 0,
  kUseStatusReadyFree = 1,
  kUseStatusFreed = 2,
  kUseStatusHoldData = 3,
} usestatus_t; //共享内存的使用状态

typedef enum {
  kFlagFree = 0x00, //共享内存空闲
  kFlagSelfRead = 0x01, //共享内存自己读取
  kFlagSelfWrite = 0x02, //共享内存自己写
  kFlagServerRead = 0x03, //游戏服务器读
  kFlagServerWrite = 0x04, //游戏服务器写
  kFlagCenterRead = 0x05, //世界服务器读
  kFlagCenterWrite = 0x06, //世界服务器写
} flag_t; //共享内存的当前状态

typedef enum {
  kTypeInvaild = -1,
  kTypePlayer = 1, //玩家
  kTypeGuild = 2, //帮会
  kTypeGlobal = 3, //全局数据
} type_t; //共享内存的类型

//static define --

typedef struct head_struct {
  uint32_t poolid;
  int32_t id;
  int32_t usestatus;
  char flag;
  int16_t playerid;
  uint32_t savetime;
  head_struct();
  ~head_struct();
  void cleanup();
} head_t;

template <typename T>
struct data_template {
  head_t head;
  T data;
  void lock(char type) {
    pf_sys::memory::share::lock(&head.flag, type);
  }
  void unlock(char type) {
    pf_sys::memory::share::unlock(&head.flag, type);
  }
  void set_poolid(uint32_t id) {
    head.poolid = id;
  }
  uint32_t get_poolid() {
    return head.poolid;
  }
  void set_usestatus(int32_t status, char type) {
    lock(type);
    head.usestatus = status;
    unlock(type);
  }
  int32_t get_usestatus(char type) {
    int32_t status;
    lock(type);
    status = head.usestatus;
    unlock(type);
    return status;
  }
  uint32_t get_savetime(char type) {
    uint32_t savetime;
    lock(type);
    savetime = head.savetime;
    unlock(type);
    return savetime;
  }
  void set_savetime(uint32_t time, char type) {
    lock(type);
    head.savetime = time;
    unlock(type);
  }
  void init() {
    data.clear();
  }
};

typedef data_template<db::globaldata_t> globaldata_t;

}; //namespace sharememory

}; //namespace common

#endif //COMMON_SHAREMEMORY_DEFINE_H_
