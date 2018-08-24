#pragma once
#include <stdint.h>

// PPU�ñ�־
enum sfc_ppu_flag {
    SFC_PPUFLAG_NMIGen  = 0x80, // [0x2000]VBlank�ڼ��Ƿ����NMI
    SFC_PPUFLAG_Sp8x16  = 0x20, // [0x2000]����Ϊ8x16(1), ����8x8(0)
    SFC_PPUFLAG_BgTabl  = 0x10, // [0x2000]������ɫ����ַ$1000(1), $0000(0)
    SFC_PPUFLAG_SpTabl  = 0x08, // [0x2000]�����ɫ����ַ$1000(1), $0000(0), 8x16ģʽ�±�����
    SFC_PPUFLAG_VINC32  = 0x04, // [0x2000]VRAM��д����ֵ32(1), 1(0)
        
    SFC_PPUFLAG_VBlank  = 0x80, // [0x2002]��ֱ�հ׼�϶��־
    SFC_PPUFLAG_Sp0Hit  = 0x40, // [0x2002]��ž������б�־
    SFC_PPUFLAG_SpOver  = 0x20, // [0x2002]���������־
};

/// <summary>
/// 
/// </summary>
typedef struct {
    // �ڴ��ַ��
    uint8_t*        banks[0x4000 / 0x0400];
    // VRAM ��ַ
    uint16_t        vramaddr;
    // �Ĵ��� PPUCTRL      @$2000
    uint8_t         ctrl;
    // �Ĵ��� PPUMASK      @$2001
    uint8_t         mask;
    // �Ĵ��� PPUSTATUS    @$2002
    uint8_t         status;
    // �Ĵ��� OAMADDR      @$2003
    uint8_t         oamaddr;
    // ����ƫ��
    uint8_t         scroll[2];
    // ����ƫ��˫дλ�ü�¼
    uint8_t         writex2;
    // �Դ��ȡ����ֵ
    uint8_t         pseudo;
    // �����ɫ������
    uint8_t         spindexes[0x20];
    // ��������: 256B
    uint8_t         sprites[0x100];
} sfc_ppu_t;


// read ppu register via cpu address space
uint8_t sfc_read_ppu_register_via_cpu(uint16_t, sfc_ppu_t*);
// write ppu register via cpu address space
void sfc_write_ppu_register_via_cpu(uint16_t, uint8_t, sfc_ppu_t*);

