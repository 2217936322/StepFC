#include "sfc_famicom.h"
#include <assert.h>
#include <string.h>

// ����Ĭ��ROM
static sfc_ecode sfc_load_default_rom(void*, sfc_rom_info_t*);
// �ͷ�Ĭ��ROM
static sfc_ecode sfc_free_default_rom(void*, sfc_rom_info_t*);
// �ͷ�Ĭ��ROM
static void sfc_before_execute(void*, sfc_famicom_t*);
// �����µ�ROM
static sfc_ecode sfc_load_new_rom(sfc_famicom_t* famicom);
// ����mapper
extern sfc_ecode sfc_load_mapper(sfc_famicom_t* famicom, uint8_t);


// ����һ�����(SB)�ĺ���ָ������
typedef void(*sfc_funcptr_t)();

/// <summary>
/// StepFC: ��ʼ��famicom
/// </summary>
/// <param name="famicom">The famicom.</param>
/// <param name="argument">The argument.</param>
/// <param name="interfaces">The interfaces.</param>
/// <returns></returns>
sfc_ecode sfc_famicom_init(
    sfc_famicom_t* famicom, 
    void* argument, 
    const sfc_interface_t* interfaces) {
    assert(famicom && "bad famicom");
    // �������
    memset(famicom, 0, sizeof(sfc_famicom_t));
    // ��������
    famicom->argument = argument;
    // ����Ĭ�Ͻӿ�
    famicom->interfaces.load_rom = sfc_load_default_rom;
    famicom->interfaces.free_rom = sfc_free_default_rom;
    famicom->interfaces.before_execute = sfc_before_execute;
    // ����BANK
    famicom->prg_banks[0] = famicom->main_memory;
    famicom->prg_banks[3] = famicom->save_memory;
    // �ṩ�˽ӿ�
    if (interfaces) {
        const int count = sizeof(*interfaces) / sizeof(interfaces->load_rom);
        // ������Ч�ĺ���ָ��
        // UB: C��׼����һ����֤sizeof(void*)��ͬsizeof(fp) (�Ƿ���ϵ)
        //     ��������������һ��sfc_funcptr_t
        sfc_funcptr_t* const func_src = (sfc_funcptr_t*)interfaces;
        sfc_funcptr_t* const func_des = (sfc_funcptr_t*)&famicom->interfaces;
        for (int i = 0; i != count; ++i) if (func_src[i]) func_des[i] = func_src[i];
    }
    // һ��ʼ�������ROM
    return sfc_load_new_rom(famicom);
    return SFC_ERROR_OK;
}

/// <summary>
/// StepFC: ����ʼ��famicom
/// </summary>
/// <param name="famicom">The famicom.</param>
void sfc_famicom_uninit(sfc_famicom_t* famicom) {
    // �ͷ�ROM
    famicom->interfaces.free_rom(famicom->argument, &famicom->rom_info);
}


/// <summary>
/// StepFC: �������ʱ��òֿ�
/// </summary>
/// <param name="famicom">The famicom.</param>
static inline void sfc_setup_nametable_bank(sfc_famicom_t* famicom) {
    // 4��
    if (famicom->rom_info.four_screen) {
        famicom->ppu.banks[0x8] = famicom->video_memory + 0x400 * 0;
        famicom->ppu.banks[0x9] = famicom->video_memory + 0x400 * 1;
        famicom->ppu.banks[0xa] = famicom->video_memory_ex + 0x400 * 0;
        famicom->ppu.banks[0xb] = famicom->video_memory_ex + 0x400 * 1;
    }
    // ���
    else if (famicom->rom_info.vmirroring) {
        famicom->ppu.banks[0x8] = famicom->video_memory + 0x400 * 0;
        famicom->ppu.banks[0x9] = famicom->video_memory + 0x400 * 1;
        famicom->ppu.banks[0xa] = famicom->video_memory + 0x400 * 0;
        famicom->ppu.banks[0xb] = famicom->video_memory + 0x400 * 1;
    }
    // �ݰ�
    else {
        famicom->ppu.banks[0x8] = famicom->video_memory + 0x400 * 0;
        famicom->ppu.banks[0x9] = famicom->video_memory + 0x400 * 0;
        famicom->ppu.banks[0xa] = famicom->video_memory + 0x400 * 1;
        famicom->ppu.banks[0xb] = famicom->video_memory + 0x400 * 1;
    }
}

/// <summary>
/// StepFC: ����famicom
/// </summary>
/// <param name="famicom">The famicom.</param>
/// <returns></returns>
sfc_ecode sfc_famicom_reset(sfc_famicom_t* famicom) {
    // ���PPU����
    memset(&famicom->ppu, 0, sizeof(famicom->ppu));
    // ����mapper
    sfc_ecode ecode = famicom->mapper.reset(famicom);
    if (ecode) return ecode;
    // ��ʼ���Ĵ���
    const uint8_t pcl = sfc_read_cpu_address(SFC_VERCTOR_RESET + 0, famicom);
    const uint8_t pch = sfc_read_cpu_address(SFC_VERCTOR_RESET + 1, famicom);
    famicom->registers.program_counter = (uint16_t)pcl | (uint16_t)pch << 8;
    famicom->registers.accumulator = 0;
    famicom->registers.x_index = 0;
    famicom->registers.y_index = 0;
    famicom->registers.stack_pointer = 0xff;
    famicom->registers.status = 0
        | SFC_FLAG_R    //  һֱΪ1
        ;
    // ��ɫ��
    // ���Ʊ�
    sfc_setup_nametable_bank(famicom);
    // ����
    famicom->ppu.banks[0xc] = famicom->ppu.banks[0x8];
    famicom->ppu.banks[0xd] = famicom->ppu.banks[0x9];
    famicom->ppu.banks[0xe] = famicom->ppu.banks[0xa];
    famicom->ppu.banks[0xf] = famicom->ppu.banks[0xb];

    return SFC_ERROR_OK;
}


/// <summary>
/// StepFC: ��ʼ��ֱ�հ�
/// </summary>
/// <param name="famicom">The famicom.</param>
extern inline void sfc_vblank_start(sfc_famicom_t* famicom) {
    famicom->ppu.status |= (uint8_t)SFC_PPUFLAG_VBlank;
}

/// <summary>
/// StepFC: ������ֱ�հ�
/// </summary>
/// <param name="famicom">The famicom.</param>
extern inline void sfc_vblank_end(sfc_famicom_t* famicom) {
    famicom->ppu.status &= ~(uint8_t)SFC_PPUFLAG_VBlank;
}

#include <stdio.h>
#include <stdlib.h>

/// <summary>
/// ����Ĭ�ϲ���ROM
/// </summary>
/// <param name="arg">The argument.</param>
/// <param name="info">The information.</param>
/// <returns></returns>
sfc_ecode sfc_load_default_rom(void* arg, sfc_rom_info_t* info) {
    assert(info->data_prgrom == NULL && "FREE FIRST");
    FILE* const file = fopen("nestest.nes", "rb");
    //FILE* const file = fopen("01-basics.nes", "rb");
    // �ı�δ�ҵ�
    if (!file) return SFC_ERROR_FILE_NOT_FOUND;
    sfc_ecode code = SFC_ERROR_ILLEGAL_FILE;
    // ��ȡ�ļ�ͷ
    sfc_nes_header_t nes_header;
    if (fread(&nes_header, sizeof(nes_header), 1, file)) {
        // ��ͷ4�ֽ�
        union { uint32_t u32; uint8_t id[4]; } this_union;
        this_union.id[0] = 'N';
        this_union.id[1] = 'E';
        this_union.id[2] = 'S';
        this_union.id[3] = '\x1A';
        // �Ƚ������ֽ�
        if (this_union.u32 == nes_header.id) {
            const size_t size1 = 16 * 1024 * nes_header.count_prgrom16kb;
            const size_t size2 =  8 * 1024 * nes_header.count_chrrom_8kb;
            uint8_t* const ptr = (uint8_t*)malloc(size1 + size2);
            // �ڴ�����ɹ�
            if (ptr) {
                code = SFC_ERROR_OK;
                // TODO: ʵ��Trainer
                // ����Trainer����
                if (nes_header.control1 & SFC_NES_TRAINER) fseek(file, 512, SEEK_CUR);
                // �ⶼ���˾Ͳ����ҵ�������
                fread(ptr, size1 + size2, 1, file);

                // ��дinfo���ݱ��
                info->data_prgrom = ptr;
                info->data_chrrom = ptr + size1;
                info->count_prgrom16kb = nes_header.count_prgrom16kb;
                info->count_chrrom_8kb = nes_header.count_chrrom_8kb;
                info->mapper_number 
                    = (nes_header.control1 >> 4) 
                    | (nes_header.control2 & 0xF0)
                    ;
                info->vmirroring    = (nes_header.control1 & SFC_NES_VMIRROR) > 0;
                info->four_screen   = (nes_header.control1 & SFC_NES_4SCREEN) > 0;
                info->save_ram      = (nes_header.control1 & SFC_NES_SAVERAM) > 0;
                assert(!(nes_header.control1 & SFC_NES_TRAINER) && "unsupported");
                assert(!(nes_header.control2 & SFC_NES_VS_UNISYSTEM) && "unsupported");
                assert(!(nes_header.control2 & SFC_NES_Playchoice10) && "unsupported");
            }
            // �ڴ治��
            else code = SFC_ERROR_OUT_OF_MEMORY;
        }
        // �Ƿ��ļ�
    }
    fclose(file);
    return code;
}

/// <summary>
/// �ͷ�Ĭ�ϲ���ROM
/// </summary>
/// <param name="arg">The argument.</param>
/// <param name="info">The information.</param>
/// <returns></returns>
sfc_ecode sfc_free_default_rom(void* arg, sfc_rom_info_t* info) {
    // �ͷŶ�̬���������
    free(info->data_prgrom);
    info->data_prgrom = NULL;

    return SFC_ERROR_OK;
}


/// <summary>
/// Ĭ��ִ��ǰ����Ϊ
/// </summary>
/// <param name="">The .</param>
/// <param name="">The .</param>
/// <returns></returns>
void sfc_before_execute(void* arg, sfc_famicom_t* info) {

}


/// <summary>
/// StepFC: ����ROM
/// </summary>
/// <param name="famicom">The famicom.</param>
/// <returns></returns>
sfc_ecode sfc_load_new_rom(sfc_famicom_t* famicom) {
    // ���ͷžɵ�ROM
    sfc_ecode code = famicom->interfaces.free_rom(
        famicom->argument,
        &famicom->rom_info
    );
    // �������
    memset(&famicom->rom_info, 0, sizeof(famicom));
    // ����ROM
    if (code == SFC_ERROR_OK) {
        code = famicom->interfaces.load_rom(
            famicom->argument,
            &famicom->rom_info
        );
    }
    // �����µ�Mapper
    if (code == SFC_ERROR_OK) {
        code = sfc_load_mapper(
            famicom,
            famicom->rom_info.mapper_number
        );
    }
    // �״�����
    if (code == SFC_ERROR_OK) {
        code = sfc_famicom_reset(famicom);
    }
    return code;
}