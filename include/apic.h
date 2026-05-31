#pragma once

#ifdef LOONGARCH

#include "types.h"

void apic_init(void);

void apic_complete(uint64 irq);

#endif