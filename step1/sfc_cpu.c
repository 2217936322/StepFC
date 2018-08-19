#include "sfc_cpu.h"
#include "sfc_famicom.h"
#include <assert.h>
#include <string.h>

/// <summary>
/// StepFC: ��ȡCPU��ַ����
/// </summary>
/// <param name="address">The address.</param>
/// <param name="famicom">The famicom.</param>
/// <returns></returns>
uint8_t sfc_read_cpu_address(uint16_t address, const sfc_famicom_t* famicom) {
    /* 
    CPU ��ַ�ռ�
    +---------+-------+-------+-----------------------+
    | ��ַ    | ��С  | ���  |         ����          |
    +---------+-------+-------+-----------------------+
    | $0000   | $800  |       | RAM                   |
    | $0800   | $800  | M     | RAM                   |
    | $1000   | $800  | M     | RAM                   |
    | $1800   | $800  | M     | RAM                   |
    | $2000   | 8     |       | Registers             |
    | $2008   | $1FF8 | R     | Registers             |
    | $4000   | $20   |       | Registers             |
    | $4020   | $1FDF |       | Expansion ROM         |
    | $6000   | $2000 |       | SRAM                  |
    | $8000   | $4000 |       | PRG-ROM               |
    | $C000   | $4000 |       | PRG-ROM               |
    +---------+-------+-------+-----------------------+
    ���ͼ��: M = $0000�ľ���
              R = $2000-2008 ÿ 8 bytes �ľ���
            (e.g. $2008=$2000, $2018=$2000, etc.)
    */
    switch (address >> 13)
    {
    case 0:
        // ����λΪ0: [$0000, $2000): ϵͳ���ڴ�, 4�ξ���
        return famicom->main_memory[address & (uint16_t)0x07ff];
    case 1:
        // ����λΪ1, [$2000, $4000): PPU�Ĵ���, 8�ֽڲ�������
        assert(!"NOT IMPL");
        return 0;
    case 2:
        // ����λΪ2, [$4000, $6000): pAPU�Ĵ��� ��չROM��
        assert(!"NOT IMPL");
        return 0;
    case 3:
        // ����λΪ3, [$6000, $8000): �浵 SRAM��
        return famicom->save_memory[address & (uint16_t)0x1fff];
    case 4: case 5: case 6: case 7:
        // ��һλΪ1, [$8000, $10000) ����PRG-ROM��
        return famicom->prg_banks[address >> 13][address & (uint16_t)0x1fff];
    }
    assert(!"invalid address");
    return 0;
}


/// <summary>
/// SFCs the write cpu address.
/// </summary>
/// <param name="address">The address.</param>
/// <param name="data">The data.</param>
/// <param name="famicom">The famicom.</param>
void sfc_write_cpu_address(uint16_t address, uint8_t data, sfc_famicom_t* famicom) {
    /* 
    CPU ��ַ�ռ�
    +---------+-------+-------+-----------------------+
    | ��ַ    | ��С  | ���  |         ����          |
    +---------+-------+-------+-----------------------+
    | $0000   | $800  |       | RAM                   |
    | $0800   | $800  | M     | RAM                   |
    | $1000   | $800  | M     | RAM                   |
    | $1800   | $800  | M     | RAM                   |
    | $2000   | 8     |       | Registers             |
    | $2008   | $1FF8 | R     | Registers             |
    | $4000   | $20   |       | Registers             |
    | $4020   | $1FDF |       | Expansion ROM         |
    | $6000   | $2000 |       | SRAM                  |
    | $8000   | $4000 |       | PRG-ROM               |
    | $C000   | $4000 |       | PRG-ROM               |
    +---------+-------+-------+-----------------------+
    ���ͼ��: M = $0000�ľ���
              R = $2000-2008 ÿ 8 bytes �ľ���
            (e.g. $2008=$2000, $2018=$2000, etc.)
    */
    switch (address >> 13)
    {
    case 0:
        // ����λΪ0: [$0000, $2000): ϵͳ���ڴ�, 4�ξ���
        famicom->main_memory[address & (uint16_t)0x07ff] = data;
        return;
    case 1:
        // ����λΪ1, [$2000, $4000): PPU�Ĵ���, 8�ֽڲ�������
        assert(!"NOT IMPL");
        return;
    case 2:
        // ����λΪ2, [$4000, $6000): pAPU�Ĵ��� ��չROM��
        assert(!"NOT IMPL");
        return;
    case 3:
        // ����λΪ3, [$6000, $8000): �浵 SRAM��
        famicom->save_memory[address & (uint16_t)0x1fff] = data;
        return;
    case 4: case 5: case 6: case 7:
        // ��һλΪ1, [$8000, $10000) ����PRG-ROM��
        assert(!"WARNING: PRG-ROM");
        famicom->prg_banks[address >> 13][address & (uint16_t)0x1fff] = data;
        return;
    }
    assert(!"invalid address");
}
