/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc)
{
    int reg_num = ARRLEN(cpu.gpr);
    int i;
    for (i = 0; i < reg_num; i++) {
        if (ref_r->gpr[i] != cpu.gpr[i]) {
            Log("reg %s not equal, ref value is %u, duc value is %u",
                reg_name(i), ref_r->gpr[i], cpu.gpr[i]);
            Log("mstatus: %u\tmcause: %u\tmtvec: %u\tmepc: %u\n", cpu.mstatus,
                cpu.mcause, cpu.mtvec, cpu.mepc);
            return false;
        }
    }
    if (ref_r->pc != cpu.pc) {
        return false;
    }
    return true;
}

void isa_difftest_attach() {}
