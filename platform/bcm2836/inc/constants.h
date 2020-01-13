#define MAX_DEV_NAME 128
#define MAX_CPUS 16
#define KRN_GPIO_BASE (MMIO_VADDR+0x200000)
#define KRN_MAILBOX_BASE (MMIO_VADDR+0xb880)
#define BCM2835_PIC (MMIO_VADDR+0x00b000)
#define IPI_MAILBOX_BASE (MMIO_VADDR+0x01000080)
#define KRN_SERIAL_BASE (MMIO_VADDR+0x215000)
#define BCM2835_TIMER (MMIO_VADDR+0x01003000)
#define ARM_MAILBOX_OFFSET 0x40000000
#define ACPI_HDR_SIZE 36
#define ACPI_LO (0xb0000000)
#define ACPI_HI (0xbfffffff)
#define LAPIC_MMIO (0xfee00000)
#define PC_TIMER 0x40
#define X86_PHYS_GDT (0x10000)
#define X86_VIRT_GDT (0xff410000)
#define X86_PHYS_IDT (0x9000)
#define X86_VIRT_IDT (0xff401000)
#define X86_TRAMPOLINE 0x8000
#define STACK_SIZE 0x1000
#define STACK_BASE 0xff800000
#define STACK_LOCATION (STACK_BASE+STACK_SIZE)
#define PAGE_SIZE 0x1000
#define FRAME_SIZE 0x1000
#define MMU_HEAP_START 0x90000000
#define MMU_FRAMEBUFFER 0xfb000000
#define MMU_CLEAR_FRAME 0xff400000
#define MMU_NEW_TABLE_INIT 0xff408000
#define MMU_STACK_INIT_VADDR 0xff40a000
#define ARMV7_TTL1_TABLE_VADDR 0xff404000
#define ARMV7_TTL2_TABLE_VADDR 0xffc00000
#define ARMV7_TTL2_MGMT 0xfffff000
#define ARMV7_MMU_FAULT 0
#define ARMV7_MMU_TTL2 0b01
#define ARMV7_MMU_CODE_PAGE 0b10
#define ARMV7_MMU_DATA_PAGE 0b11
#define ARMV7_MMU_BUFFERED (1<<2)
#define ARMV7_MMU_UNBUFFERED 0
#define ARMV7_MMU_CACHED (1<<3)
#define ARMV7_MMU_UNCACHED 0
#define ARMV7_MMU_SHARABLE (1<<10)
#define ARMV7_MMU_NOT_SHARABLE 0
#define ARMV7_MMU_GLOBAL 0
#define ARMV7_MMU_NOT_GLOBAL (1<<11)
#define ARMV7_MMU_ACCESS_PERMISSIONS (0b11<<4)
#define ARMV7_MMU_TEX (0b001<<6)
#define ARMV7_MMU_APX (1<<9)
#define ARMV7_MMU_TTL1_ENTRY ARMV7_MMU_TTL2
#define ARMV7_MMU_MGMT (ARMV7_MMU_GLOBAL|ARMV7_MMU_DATA_PAGE|ARMV7_MMU_SHARABLE|ARMV7_MMU_ACCESS_PERMISSIONS|ARMV7_MMU_TEX|ARMV7_MMU_CACHED|ARMV7_MMU_BUFFERED)
#define ARMV7_MMU_KRN_CODE (ARMV7_MMU_GLOBAL|ARMV7_MMU_SHARABLE|ARMV7_MMU_CODE_PAGE|ARMV7_MMU_ACCESS_PERMISSIONS|ARMV7_MMU_TEX|ARMV7_MMU_CACHED|ARMV7_MMU_BUFFERED)
#define ARMV7_MMU_KRN_DATA (ARMV7_MMU_GLOBAL|ARMV7_MMU_SHARABLE|ARMV7_MMU_DATA_PAGE|ARMV7_MMU_ACCESS_PERMISSIONS|ARMV7_MMU_TEX|ARMV7_MMU_CACHED|ARMV7_MMU_BUFFERED)
#define ARMV7_MMU_KRN_ANY (ARMV7_MMU_GLOBAL|ARMV7_MMU_SHARABLE|ARMV7_MMU_CODE_PAGE|ARMV7_MMU_ACCESS_PERMISSIONS|ARMV7_MMU_TEX|ARMV7_MMU_CACHED|ARMV7_MMU_BUFFERED)
#define X86_MMU_PRESENT (1<<0)
#define X86_MMU_READ 0
#define X86_MMU_WRITE (1<<1)
#define X86_MMU_SUPERVISOR 0
#define X86_MMU_USER (1<<2)
#define X86_MMU_PWT (1<<3)
#define X86_MMU_PCD (1<<4)
#define X86_MMU_ACCESSED (1<<5)
#define X86_MMU_DIRTY (1<<6)
#define X86_MMU_PAT (1<<7)
#define X86_MMU_GLOBAL (1<<8)
#define X86_MMU_KERNEL (1<<9)
#define X86_MMU_BASE (X86_MMU_PRESENT|X86_MMU_WRITE|X86_MMU_SUPERVISOR)
#define X86_MMU_KRN_CODE (X86_MMU_PRESENT|X86_MMU_SUPERVISOR|X86_MMU_READ|X86_MMU_KERNEL)
#define X86_MMU_KRN_DATA (X86_MMU_PRESENT|X86_MMU_SUPERVISOR|X86_MMU_WRITE|X86_MMU_KERNEL)
#define MAGIC1 0x1badb002
#define MAGIC2 0xe85250d6
#define MBFLAGS ((1<<1)|(1<<2))
#define MB1SIG 0x2badb002
#define MB2SIG 0x36d76289
#define MODE_TYPE 0
#define WIDTH 1024
#define HEIGHT 768
#define DEPTH 16
#define RPI2B_HEIGHT 400
#define RPI2B_WIDTH 800
#define EXCEPT_VECTOR_TABLE 0xff401000
#define MMIO_VADDR 0xf8000000
#define MMIO_LOADER_LOC 0x3f000
#define MMIO_LOADER_END 0x40040
