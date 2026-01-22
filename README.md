启动流程说明
先跑bootloader（bootasm.S和bootmain.c），把内核从硬盘读到内存，然后跳转到内核入口entry.S，最后执行main()函数。
BIOS → Boot Sector → bootasm.S → bootmain.c → entry.S → main()

运行结果
[BOOT] enter bootmain
[BOOT] elf header loaded
[BOOT] kernel loaded  
[KERNEL] main() started

个人总结
通过实证方法验证了操作系统启动过程的多阶段特性，证实了启动过程对CPU工作模式、内存管理单元等硬件状态的严格依赖关系，证明了ELF作为标准可执行格式在系统启动中的关键作用。
为操作系统内核开发提供了可靠的启动框架参考，建立的调试方法可用于后续内核模块的故障诊断，启动时间优化方案可为嵌入式系统开发提供借鉴。