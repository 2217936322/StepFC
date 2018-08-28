#pragma once
#include <stdint.h>

/// <summary>
/// PPU�ñ�־ - $2000 - CTRL
/// </summary>
enum sfc_ppu_flag_2000 {
    SFC_PPU2000_NMIGen  = 0x80, // [0x2000]VBlank�ڼ��Ƿ����NMI
    SFC_PPU2000_Sp8x16  = 0x20, // [0x2000]����Ϊ8x16(1), ����8x8(0)
    SFC_PPU2000_BgTabl  = 0x10, // [0x2000]������ɫ����ַ$1000(1), $0000(0)
    SFC_PPU2000_SpTabl  = 0x08, // [0x2000]�����ɫ����ַ$1000(1), $0000(0), 8x16ģʽ�±�����
    SFC_PPU2000_VINC32  = 0x04, // [0x2000]VRAM��д����ֵ32(1), 1(0)
};

/// <summary>
/// PPU�ñ�־ - $2001 - MASK
/// </summary>
enum sfc_ppu_flag_2001 {
    SFC_PPU2001_Grey    = 0x01, // �ҽ�ʹ��
    SFC_PPU2001_BackL8  = 0x02, // ��ʾ����ߵ�8���ر���
    SFC_PPU2001_SpriteL8= 0x04, // ��ʾ����ߵ�8���ؾ���
    SFC_PPU2001_Back    = 0x08, // ������ʾʹ��
    SFC_PPU2001_Sprite  = 0x10, // ������ʾʹ��

    SFC_PPU2001_NTSCEmR = 0x20, // NTSC ǿ����ɫ
    SFC_PPU2001_NTSCEmG = 0x40, // NTSC ǿ����ɫ
    SFC_PPU2001_NTSCEmB = 0x80, // NTSC ǿ����ɫ

    SFC_PPU2001_PALEmG  = 0x20, // PAL ǿ����ɫ
    SFC_PPU2001_PALEmR  = 0x40, // PAL ǿ����ɫ
    SFC_PPU2001_PALEmB  = 0x80, // PAL ǿ����ɫ
};


/// <summary>
/// PPU�ñ�־ - $2002 - STATUS
/// </summary>
enum sfc_ppu_flag_2002 {
    SFC_PPU2002_VBlank = 0x80, // [0x2002]��ֱ�հ׼�϶��־
    SFC_PPU2002_Sp0Hit = 0x40, // [0x2002]��ž������б�־
    SFC_PPU2002_SpOver = 0x20, // [0x2002]���������־
};


/// <summary>
/// PPU�ñ�־ - ��������
/// </summary>
enum sfc_ppu_flag_sprite_attr {
    SFC_SPATTR_FlipV   = 0x80, // ��ֱ��ת
    SFC_SPATTR_FlipH   = 0x40, // ˮƽ��ת
    SFC_SPATTR_Priority= 0x20, // ����λ
};

/// <summary>
/// 
/// </summary>
typedef struct {
    // �ڴ��ַ��
    uint8_t*        banks[0x4000 / 0x0400];
    // ���Ʊ�ѡ��(PPUCTRL��2λ, �Լ���Ⱦ��VRAMָ��ABλ)
    uint16_t        nametable_select;
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
    // ��������: 256B - ��32λ����
    union {
        // ��32λ����
        uint32_t    aligned_buffer[0x100 / 4];
        // ��������: 256B
        uint8_t     sprites[0x100];
    };
} sfc_ppu_t;


// read ppu register via cpu address space
uint8_t sfc_read_ppu_register_via_cpu(uint16_t, sfc_ppu_t*);
// write ppu register via cpu address space
void sfc_write_ppu_register_via_cpu(uint16_t, uint8_t, sfc_ppu_t*);

