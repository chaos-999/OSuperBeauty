OUTPUT_ARCH( "riscv" )
ENTRY( _entry )

SECTIONS
{
  /*
   * VF2板子通过u-boot加载，内核加载地址设为0x80400000
   * 这与xv6-vf2保持一致，为u-boot预留空间
   */
  . = 0x80400000;

  kernel_start = .;

  .text : {
    *(.text .text.*)
    . = ALIGN(0x1000);
    _trampoline = .;
    *(trampsec)
    . = ALIGN(0x1000);
    ASSERT(. - _trampoline == 0x1000, "error: trampoline larger than one page");
    _sig_trampoline = .;
    *(sigtrampsec)
    . = ALIGN(0x1000);
    ASSERT(. - _sig_trampoline == 0x1000, "error: signal trampoline larger than one page");
    PROVIDE(etext = .);
  }

  .rodata : {
    . = ALIGN(16);
    *(.srodata .srodata.*)
    . = ALIGN(16);
    *(.rodata .rodata.*)
    /* Place embedded ramdisk (binary) into rodata with proper alignment */
    . = ALIGN(4096);
    _binary_ramdisk_img_start = .;
    *(.ramdisk)
    _binary_ramdisk_img_end = .;
  }

  .data : {
    . = ALIGN(16);
    *(.sdata .sdata.*)
    . = ALIGN(16);
    *(.data .data.*)
  }

  .bss : {
    . = ALIGN(16);
    __bss_start = .;
    *(.sbss .sbss.*)
    *(.bss .bss.*)
    *(.bss.stack .bss.stack.*)
    *(COMMON)
    . = ALIGN(16);
    __bss_end = .;
  }

  PROVIDE(end = .);
}