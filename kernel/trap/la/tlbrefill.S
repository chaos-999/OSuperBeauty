#include "asm.h"

.section tlbrentry
.globl handle_tlbr
.align 0x4
handle_tlbr:
	csrwr	$t0, LOONGARCH_CSR_TLBRSAVE
	csrwr	$t1, LOONGARCH_CSR_SAVE1
	csrrd	$t0, LOONGARCH_CSR_PGD
	lddir	$t0, $t0, 3
	andi	$t1, $t0, 1
	beqz	$t1, tlbr_invalid
	addi.d  $t0, $t0, -1
	lddir	$t0, $t0, 2
	andi	$t1, $t0, 1
	beqz	$t1, tlbr_invalid
	addi.d  $t0, $t0, -1
	lddir   $t0, $t0, 1
	andi	$t1, $t0, 1
	beqz	$t1, tlbr_invalid
	addi.d  $t0, $t0, -1
	ldpte	$t0, 0
	ldpte	$t0, 1
	b	tlbr_fill

tlbr_invalid:
	/*
	 * An intermediate page-table level is missing (e.g. a wild user
	 * pointer such as NULL).  Installing a bogus entry here used to make
	 * the retried access refill forever.  Fill an *invalid* entry instead:
	 * the retry then raises the normal Page Invalid Load/Store exception,
	 * which usertrap() handles (lazy allocation or SIGSEGV).
	 */
	csrwr	$zero, LOONGARCH_CSR_TLBRELO0
	csrwr	$zero, LOONGARCH_CSR_TLBRELO1

tlbr_fill:
	tlbfill
	csrrd	$t0, LOONGARCH_CSR_TLBRSAVE
	csrrd	$t1, LOONGARCH_CSR_SAVE1
	ertn
