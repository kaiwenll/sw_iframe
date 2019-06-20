#ifndef IFRA_H
#define IFRA_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>


/* There is some amount of overlap with <sys/types.h> as known by inet code */
#ifndef __int8_t_defined  
# define __int8_t_defined  
typedef signed char             int8_t;
typedef short int               int16_t;
typedef int                     int32_t;
# if __WORDSIZE == 64  
typedef long int                int64_t;
# else
typedef long long int           int64_t;
# endif
#endif

/* Unsigned.  */
typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
#ifndef __uint32_t_defined  
typedef unsigned int            uint32_t;
# define __uint32_t_defined  
#endif
#if __WORDSIZE == 64  
typedef unsigned long int       uint64_t;
#else
typedef unsigned long long int  uint64_t;
#endif


//TS packet
typedef struct {
    uint32_t sync_byte                   : 8; /* 8bit 同步字节字段 0x47         */
    uint32_t transport_error_indicator   : 1; /* 1bit 传输错误指示符            */
    uint32_t payload_unit_start_indicator: 1; /* 1bit 有效负载数据单元起始指示符*/
    uint32_t transport_priority          : 1; /* 1bit 传输优先级                */
    uint32_t pid                         :13; /*13bit 包标示符                  */
    uint32_t transport_scrambling_control: 2; /* 2bit 传输加扰控制              */
    uint32_t adaption_field_control      : 2; /* 2bit 适应控制                  */
    uint32_t continuity_counter          : 4; /* 4bit 连续性计数器              */
    uint32_t adaptation_field_length     : 8; /* 8bit 自适应字段长度            */
    uint32_t pointer_field               : 8; /* 8bit 指针                      */

    uint32_t playload_offset             :16; /* 32bit 有效负载偏移             */
}sw_transport_packet_t;
void sw_open_ts(void);//打开文件
int sw_show_ts_info(sw_transport_packet_t *tspkt);//输出TS包包头信息
int sw_ts_packet_parse(sw_transport_packet_t *ptspkt, uint8_t *data);//把ts包头信息读入到sw_transport_packet_t结构体中
int sw_reset_ts_struct(sw_transport_packet_t *ptspkt);//把结构体初始化
#endif
