#ifndef MMU_H
#define MMU_H


#include "comm/types.h"

#define PDE_CNT         1024

typedef union _pde_t {
    uint32_t v;
    struct {
        uint32_t present : 1;
        uint32_t write_enable : 1;
        uint32_t user_mode_acc : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t : 1;
        uint32_t ps : 1;
        uint32_t : 4;
        uint32_t phy_pt_addr : 20;
    };
}pde_t;


typedef union _pte_t {
    uint32_t v;
    struct {
        uint32_t present : 1;
        uint32_t write_enable : 1;
        uint32_t user_mode_acc : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t pat : 1;
        uint32_t global : 1;
        uint32_t : 3;
        uint32_t phy_pt_addr : 20;
    };
}pte_t;


#endif