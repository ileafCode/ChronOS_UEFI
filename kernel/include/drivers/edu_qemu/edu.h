
#ifndef _KERNEL_DEV_EDU_H
#define _KERNEL_DEV_EDU_H

#include <stdint.h>
#include <pci/pci.h>

#define EDU_ID_REG          0x00
#define EDU_LIVENESS_REG    0x04
#define EDU_FACTORIAL_REG   0x08
#define EDU_STATUS_REG      0x20
#define EDU_INT_STATUS_REG  0x24
#define EDU_INT_RAISE_REG   0x60
#define EDU_INT_ACK_REG     0x64

#define EDU_STATUS_COMPUTING 0x01
#define EDU_STATUS_RAISE_INT 0x80

void dev_edu_init(pci_hdr0_t *hdr);

#endif