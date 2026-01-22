// 引导程序的C语言部分，负责加载并运行内核
#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

// 定义扇区大小
#define SECTSIZE  512

// 从磁盘偏移offset处读取count个字节到虚拟地址va
void readseg(uchar*, uint, uint);

// bootmain函数是bootloader的C入口点，由bootasm.S调用
void
bootmain(void)
{
  // 任务3要求：在bootmain入口处打印信息
  cprintf("[BOOT] enter bootmain\n");

  // 定义ELF头指针，并读取第一个扇区（包含ELF头）到内存地址0x10000
  struct elfhdr *elf;
  elf = (struct elfhdr*)0x10000;

  readseg((uchar*)elf, SECTSIZE*8, 0);

  // 检查ELF魔数，确保这是一个有效的ELF可执行文件
  if(elf->magic != ELF_MAGIC){
    // 如果魔数不匹配，说明磁盘上的内核镜像可能已损坏，引导失败
    goto bad;
  }

  // 任务3要求：在成功读取ELF头后打印信息
  cprintf("[BOOT] elf header loaded\n");

  // 获取程序头表(Program Header Table)的起始地址
  struct proghdr *ph, *eph;
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum; // 程序头表的结束地址

  // 遍历程序头表，将每个需要加载的段(segment)从磁盘读入内存
  for(; ph < eph; ph++){
    // 只加载类型为PT_LOAD（可加载）的段
    if(ph->type != ELF_PROG_LOAD){
      continue;
    }
    // 从磁盘偏移ph->off处读取ph->filesz字节到内存地址ph->va
    readseg((uchar*)ph->va, ph->filesz, ph->off);
    // 如果段在内存中的大小ph->memsz大于文件大小ph->filesz，则将剩余部分（.bss段）清零
    if(ph->memsz > ph->filesz){
      stosb((uchar*)ph->va + ph->filesz, 0, ph->memsz - ph->filesz);
    }
  }

  // 任务3要求：在所有内核段加载完毕后打印信息
  cprintf("[BOOT] kernel loaded\n");

  // 从ELF头中获取内核入口点地址，并跳转到该地址执行，将控制权完全交给内核
  // entry()函数通常在entry.S中定义
  ( (void (*)(void)) (elf->entry) )();

  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
void
readsect(void *dst, uint offset)
{
  // Issue command.
  waitdisk();
  outb(0x1F2, 1);   // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}