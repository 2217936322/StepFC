#include "sfc_6502.h"
#include "sfc_cpu.h"
#include "sfc_famicom.h"
#include <assert.h>
#include <string.h>

/// <summary>
/// StepFC: ��ȡCPU��ַ����4020
/// </summary>
/// <param name="address">The address.</param>
/// <param name="famicom">The famicom.</param>
/// <returns></returns>
extern inline uint8_t sfc_read_cpu_address4020(uint16_t address, sfc_famicom_t* famicom) {
    uint8_t data = 0;
    switch (address & (uint16_t)0x1f)
    {
    case 0x16:
        // �ֱ��˿�#1
        data = (famicom->button_states+0)[famicom->button_index_1 & famicom->button_index_mask];
        ++famicom->button_index_1;
        break;
    case 0x17:
        // �ֱ��˿�#2
        data = (famicom->button_states+8)[famicom->button_index_2 & famicom->button_index_mask];
        ++famicom->button_index_2;
        break;
    }
    return data;
}


/// <summary>
/// StepFC: ��ȡDMA��ַ
/// </summary>
/// <param name="data">The data.</param>
/// <returns></returns>
static inline const uint8_t* sfc_get_dma_address(uint8_t data, const sfc_famicom_t* famicom) {
    const uint16_t offset = ((uint16_t)(data & 0x07) << 8);
    switch (data >> 5)
    {
    default:
    case 1:
        // PPU�Ĵ���
        assert(!"PPU REG!");
    case 2:
        // ��չ��
        assert(!"TODO");
    case 0:
        // ϵͳ�ڴ�
        return famicom->main_memory + offset;
    case 3:
        // �浵 SRAM��
        return famicom->save_memory + offset;
    case 4: case 5: case 6: case 7:
        // ��һλΪ1, [$8000, $10000) ����PRG-ROM��
        return famicom->prg_banks[data >> 5] + offset;
    }
}

/// <summary>
/// StepFC: д��CPU��ַ����4020
/// </summary>
/// <param name="address">The address.</param>
/// <param name="data">The data.</param>
/// <param name="famicom">The famicom.</param>
extern inline void sfc_write_cpu_address4020(uint16_t address, uint8_t data, sfc_famicom_t* famicom) {
    switch (address & (uint16_t)0x1f)
    {
    case 0x14:
        // ����RAMֱ�Ӵ���������
        memcpy(famicom->ppu.sprites, sfc_get_dma_address(data, famicom), 256);
        famicom->cycle_count += 513;
        famicom->cycle_count += famicom->cycle_count & 1;
        break;
    case 0x16:
        // �ֱ��˿�
        famicom->button_index_mask = (data & 1) ? 0x0 : 0x7;
        if (data & 1) {
            famicom->button_index_1 = 0;
            famicom->button_index_2 = 0;
        }
        break;
    }
}