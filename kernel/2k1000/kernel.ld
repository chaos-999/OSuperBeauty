OUTPUT_ARCH( "loongarch" )
ENTRY( _entry )

SECTIONS
{
  . = 0x9000000080000000;

  kernel_start = .;

  /* 把引导头部段放在最前，确保 go 地址即落在 _entry */
  . = ALIGN(4K);
  .head.text : {
    KEEP(*(.head.text))
  }

  . = ALIGN(4K);

  .text : {
    *(.text .text.*)
    . = ALIGN(0x1000);
    _keentry = .;
    *(k_eentry)
    . = ALIGN(0x1000);
    _ueentry = .;
    *(u_eentry)
    . = ALIGN(0x1000);
    _tlbrentry = .;
    *(tlbrentry)
    . = ALIGN(0x1000);
    _merrentry = .;
    *(merrentry)
    . = ALIGN(0x1000);
    _sig_trampoline = .;
    *(sigtrampsec)
    . = ALIGN(0x1000);
    ASSERT(. - _sig_trampoline == 0x1000, "error: signal trampoline larger than one page");
    PROVIDE(etext = .);
  }

    . = ALIGN(4K);

    .rodata : {
        srodata = .;
        *(.rodata .rodata.*)
        erodata = .;
    }

    . = ALIGN(4K);

    .data : {
        sdata = .;
        *(.data .data.*)
        edata = .;
    }

    . = ALIGN(4K);
    bss_start = .;
    .bss : {
        __bss_start = .;
        *(.bss.stack)
        sbss_clear = .;
        *(.sbss .bss .bss.*)
        *(COMMON)
        ebss_clear = .;
        __bss_stop = .;
    }
    bss_end = .;

    . = ALIGN(8);
    PROVIDE(end = .);
}


