
#ifndef _KERNEL_DEV_AHCI_H
#define _KERNEL_DEV_AHCI_H

#include <stdint.h>
#include <pci/pci.h>

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SATAPI 4
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3

#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

#define HBA_PORT_DEV_PRESENT 3
#define HBA_PORT_IPM_ACTIVE 1

#define HBA_PxIS_TFES (1 << 30)
#define HBA_PxCMD_CR (1 << 15)
#define HBA_PxCMD_FR (1 << 14)
#define HBA_PxCMD_FRE (1 << 4)
#define HBA_PxCMD_SUD (1 << 1)
#define HBA_PxCMD_ST (1 << 0)

#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08


enum {
	GHC_AE			= (1U << 31),	// AHCI Enable
	GHC_MRSM		= (1 << 2),		// MSI Revert to Single Message
	GHC_IE			= (1 << 1),		// Interrupt Enable
	GHC_HR			= (1 << 0),		// HBA Reset **RW1**
};

enum {
	CAP_S64A		= (1U << 31),	// Supports 64-bit Addressing
	CAP_SNCQ		= (1 << 30),	// Supports Native Command Queuing
	CAP_SSNTF		= (1 << 29),	// Supports SNotification Register
	CAP_SMPS		= (1 << 28),	// Supports Mechanical Presence Switch
	CAP_SSS			= (1 << 27),	// Supports Staggered Spin-up
	CAP_SALP		= (1 << 26),	// Supports Aggressive Link Power Management
	CAP_SAL			= (1 << 25),	// Supports Activity LED
	CAP_SCLO		= (1 << 24),	// Supports Command List Override
	CAP_ISS_MASK	= 0xf,			// Interface Speed Support
	CAP_ISS_SHIFT	= 20,
	CAP_SNZO		= (1 << 19),	// Supports Non-Zero DMA Offsets
	CAP_SAM			= (1 << 18),	// Supports AHCI mode only
	CAP_SPM			= (1 << 17),	// Supports Port Multiplier
	CAP_FBSS		= (1 << 16),	// FIS-based Switching Supported
	CAP_PMD			= (1 << 15),	// PIO Multiple DRQ Block
	CAP_SSC			= (1 << 14),	// Slumber State Capable
	CAP_PSC			= (1 << 13),	// Partial State Capable
	CAP_NCS_MASK	= 0x1f,			// Number of Command Slots
									// (zero-based number)
	CAP_NCS_SHIFT	= 8,
	CAP_CCCS		= (1 << 7),		// Command Completion Coalescing Supported
	CAP_EMS			= (1 << 6),		// Enclosure Management Supported
	CAP_SXS			= (1 << 5),		// Supports External SATA
	CAP_NP_MASK		= 0x1f,			// Number of Ports (zero-based number)
	CAP_NP_SHIFT	= 0,
};

typedef enum {
    None = 0,
    SATA = 1,
    SEMB = 2,
    PM = 3,
    SATAPI = 4,
} port_type_t;

typedef enum {
    FIS_TYPE_REG_H2D = 0x27,
    FIS_TYPE_REG_D2H = 0x34,
    FIS_TYPE_DMA_ACT = 0x39,
    FIS_TYPE_DMA_SETUP = 0x41,
    FIS_TYPE_DATA = 0x46,
    FIS_TYPE_BIST = 0x58,
    FIS_TYPE_PIO_SETUP = 0x5F,
    FIS_TYPE_DEV_BITS = 0xA1,
} FIS_TYPE;

typedef struct {
    uint32_t commandListBase;
    uint32_t commandListBaseUpper;
    uint32_t fisBaseAddress;
    uint32_t fisBaseAddressUpper;
    uint32_t interruptStatus;
    uint32_t interruptEnable;
    uint32_t cmdSts;
    uint32_t rsv0;
    uint32_t taskFileData;
    uint32_t signature;
    uint32_t sataStatus;
    uint32_t sataControl;
    uint32_t sataError;
    uint32_t sataActive;
    uint32_t commandIssue;
    uint32_t sataNotification;
    uint32_t fisSwitchControl;
    uint32_t rsv1[11];
    uint32_t vendor[4];
} hba_port_t;

typedef struct {
    uint32_t hostCapability;
    uint32_t globalHostControl;
    uint32_t interruptStatus;
    uint32_t portsImplemented;
    uint32_t version;
    uint32_t cccControl;
    uint32_t cccPorts;
    uint32_t enclosureManagementLocation;
    uint32_t enclosureManagementControl;
    uint32_t hostCapabilitiesExtended;
    uint32_t biosHandoffCtrlSts;
    uint8_t rsv0[0x74];
    uint8_t vendor[0x60];
    hba_port_t ports[1];
} hba_mem_t;

typedef struct HBACommandHeader {
    uint8_t commandFISLength : 5;
    uint8_t atapi : 1;
    uint8_t write : 1;
    uint8_t prefetchable : 1;
    uint8_t reset : 1;
    uint8_t bist : 1;
    uint8_t clearBusy : 1;
    uint8_t rsv0 : 1;
    uint8_t portMultiplier : 4;
    uint16_t prdtLength;
    uint32_t prdbCount;
    uint32_t commandTableBaseAddress;
    uint32_t commandTableBaseAddressUpper;
    uint32_t rsv1[4];
} hba_cmd_hdr_t;

typedef struct {
    uint32_t dataBaseAddress;
    uint32_t dataBaseAddressUpper;
    uint32_t rsv0;
    uint32_t byteCount : 22;
    uint32_t rsv1 : 9;
    uint32_t interruptOnCompletion : 1;
} hba_prdt_entry_t;

typedef struct  {
    uint8_t commandFIS[64];
    uint8_t atapiCommand[16];
    uint8_t rsv[48];
    hba_prdt_entry_t prdtEntry[];
} hba_cmd_table_t;

typedef struct {
    uint8_t fisType;
    uint8_t portMultiplier : 4;
    uint8_t rsv0 : 3;
    uint8_t commandControl : 1;
    uint8_t command;
    uint8_t featureLow;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t deviceRegister;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureHigh;
    uint8_t countLow;
    uint8_t countHigh;
    uint8_t isoCommandCompletion;
    uint8_t control;
    uint8_t rsv1[4];
} FIS_REG_H2D;

typedef struct {
    hba_port_t *hbaPort;
    port_type_t portType;
    uint8_t *buffer;
    uint8_t portNumber;
} ahci_port_t;

int dev_ahci_read(ahci_port_t *port, uint64_t sector, uint32_t sectorCount);
int dev_ahci_write(ahci_port_t *port, uint64_t sector, uint32_t sectorCount);
ahci_port_t *dev_ahci_get_port(int idx);
void dev_ahci_init(pci_hdr0_t *hdr, uint64_t cur_bus, uint64_t cur_dev, uint64_t cur_func);

#endif