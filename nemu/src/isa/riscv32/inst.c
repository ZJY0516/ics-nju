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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
    TYPE_I,
    TYPE_U,
    TYPE_S,
    TYPE_J,
    TYPE_R,
    TYPE_B,
    TYPE_N, // none
};

#define src1R()                                                                \
    do {                                                                       \
        *src1 = R(rs1);                                                        \
    } while (0)
#define src2R()                                                                \
    do {                                                                       \
        *src2 = R(rs2);                                                        \
    } while (0)
#define immI()                                                                 \
    do {                                                                       \
        *imm = SEXT(BITS(i, 31, 20), 12);                                      \
    } while (0)
#define immU()                                                                 \
    do {                                                                       \
        *imm = SEXT(BITS(i, 31, 12), 20) << 12;                                \
    } while (0)
#define immS()                                                                 \
    do {                                                                       \
        *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);               \
    } while (0)
#define immJ()                                                                 \
    do {                                                                       \
        *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 19, 12) << 12) |    \
               (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1);               \
    } while (0)
#define immB()                                                                 \
    do {                                                                       \
        *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 7, 7) << 11) |      \
               (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1);                 \
    } while (0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2,
                           word_t *imm, int type)
{
    uint32_t i = s->isa.inst.val;
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    *rd = BITS(i, 11, 7);
    switch (type) {
    case TYPE_I:
        src1R();
        immI();
        break;
    case TYPE_U:
        immU();
        break;
    case TYPE_S:
        src1R();
        src2R();
        immS();
        break;
    case TYPE_J:
        immJ();
        break;
    case TYPE_R:
        src1R();
        src2R();
        break;
    case TYPE_B:
        src1R();
        src2R();
        immB();
    }
}

void ftrace(paddr_t pc, paddr_t dnpc, bool is_call);
word_t *choose_csr(word_t imm)
{
    switch (imm) {
    case 0x305:
        return &cpu.mtvec;
        break;
    case 0x300:
        return &cpu.mstatus;
        break;
    case 0x341:
        return &cpu.mepc;
        break;
    case 0x342:
        return &cpu.mcause;
        break;
    default:
        assert(0);
        break;
    }
}

static int decode_exec(Decode *s)
{
    int rd = 0;
    word_t src1 = 0, src2 = 0, imm = 0;
    word_t *csr;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)                   \
    {                                                                          \
        decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type));       \
        __VA_ARGS__;                                                           \
    }

    INSTPAT_START();
    INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R,
            R(rd) = src1 + src2);
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I,
            R(rd) = src1 + imm);
    INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R,
            R(rd) = src1 & src2);
    INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I,
            R(rd) = src1 & imm);
    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U,
            R(rd) = s->pc + imm);
    INSTPAT(
        "??????? ????? ????? 000 ????? 11000 11", beq, B,
        if (src1 == src2) { s->dnpc = s->pc + imm; });
    INSTPAT(
        "??????? ????? ????? 101 ????? 11000 11", bge, B,
        if ((sword_t)src1 >= (sword_t)src2) { s->dnpc = s->pc + imm; });
    INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B,
            if (src1 >= src2) s->dnpc = s->pc + imm);
    INSTPAT(
        "??????? ????? ????? 100 ????? 11000 11", blt, B,
        if ((sword_t)src1 < (sword_t)src2) { s->dnpc = s->pc + imm; });
    INSTPAT(
        "??????? ????? ????? 110 ????? 11000 11", bltu, B,
        if (src1 < src2) { s->dnpc = s->pc + imm; });
    INSTPAT(
        "??????? ????? ????? 001 ????? 11000 11", bne, B,
        if (src1 != src2) { s->dnpc = s->pc + imm; });
    INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs, I,
            csr = choose_csr(imm);
            word_t t = *csr; *csr = t | src1; R(rd) = t;);
    INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw, I,
            csr = choose_csr(imm);
            word_t t = *csr; *csr = src1; R(rd) = t;);
    INSTPAT(
        "0000001 ????? ????? 100 ????? 01100 11", div, R,
        if (src2 == 0) { R(rd) = -1; } else {
            R(rd) = (sword_t)src1 / (sword_t)src2;
        });
    INSTPAT(
        "0000001 ????? ????? 101 ????? 01100 11", divu, R,
        if (src2 == 0) { R(rd) = -1; } else {
            R(rd) = src1 / src2;
        }); // same with div?
    INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall, I, bool tmp;
            word_t NO = isa_reg_str2val("a7", &tmp);
            s->dnpc = isa_raise_intr(NO, s->pc););
    INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J, R(rd) = s->pc + 4;
            IFDEF(CONFIG_FTRACE,
                  {
                      if (rd == 1)
                          ftrace(s->pc, s->dnpc, true);
                  });
            s->dnpc = s->pc + imm);
    INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I,
            word_t tmp = s->pc + 4;
            IFDEF(CONFIG_FTRACE,
                  {
                      if (s->isa.inst.val == 0x00008067)
                          ftrace(s->pc, s->dnpc, false);
                      else
                          ftrace(s->pc, s->dnpc, true);
                  });
            s->dnpc = (src1 + imm) & ~1; R(rd) = tmp);
    INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I,
            R(rd) = SEXT(Mr(src1 + imm, 1), 8));
    INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I,
            R(rd) = Mr(src1 + imm, 1));
    INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I,
            R(rd) = SEXT(Mr(src1 + imm, 2), 16));
    INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I,
            R(rd) = Mr(src1 + imm, 2));
    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);
    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I,
            R(rd) = Mr(src1 + imm, 4));
    INSTPAT("0011000 00010 00000 000 00000 1110011", mret, I,
            s->dnpc = cpu.mepc ;);
    INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R,
            R(rd) = src1 * src2);
    INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh, R,
            R(rd) = ((int64_t)(sword_t)src1 * (sword_t)src2) >> 32);
    INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu, R,
            R(rd) = (uint64_t)src1 * src2 >>
                    32); // R(rd) = (uint64_t)(src1 * src2) >> 32 doesn't work
    INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R,
            R(rd) = src1 | src2);
    INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I,
            R(rd) = src1 | imm);
    INSTPAT(
        "0000001 ????? ????? 110 ????? 01100 11", rem, R,
        if (src2 == 0) { R(rd) = src1; } else {
            R(rd) = (sword_t)src1 % (sword_t)src2;
        });
    INSTPAT(
        "0000001 ????? ????? 111 ????? 01100 11", remu, R,
        if (src2 == 0) { R(rd) = src1; } else { R(rd) = src1 % src2; });
    INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S,
            Mw(src1 + imm, 1, src2));
    INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S,
            Mw(src1 + imm, 2, src2 & 0xFFFF));
    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R,
            R(rd) = src1 << src2);
    INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli, I,
            R(rd) = src1 << imm);
    INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R,
            R(rd) = (sword_t)src1 < (sword_t)src2);
    INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I,
            R(rd) = ((sword_t)src1 < (sword_t)imm));
    INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R,
            R(rd) = ((word_t)src1 < (word_t)src2));
    INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I,
            R(rd) = (src1 < imm)); // leave a qquestion
    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R,
            R(rd) = (sword_t)src1 >> (sword_t)src2);
    INSTPAT("010000 ?????? ????? 101 ????? 00100 11", srai, I,
            R(rd) = (sword_t)src1 >> (imm & 0b111111));
    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl, R,
            R(rd) = src1 >> src2);
    INSTPAT("000000 ?????? ????? 101 ????? 00100 11", srli, I,
            R(rd) = src1 >> (imm & 0b111111));
    INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R,
            R(rd) = src1 - src2);
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S,
            Mw(src1 + imm, 4, src2));
    INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R,
            R(rd) = src1 ^ src2);
    INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I,
            R(rd) = src1 ^ imm);

    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N,
            NEMUTRAP(s->pc, R(10))); // R(10) is $a0
    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
    INSTPAT_END();

    R(0) = 0; // reset $zero to 0

    return 0;
}

int isa_exec_once(Decode *s)
{
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    void add_inst_trace(word_t pc, word_t inst);
    IFDEF(CONFIG_ITRACE, add_inst_trace(s->pc, s->isa.inst.val));
    return decode_exec(s);
}
