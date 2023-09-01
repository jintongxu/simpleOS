/**
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * 该部分程序存储于磁盘的第1个扇区，在计算机启动时将会由BIOS加载到0x7c00处
 * 之后，将由BIOS跳转至0x7c00处开始运行
 * 
 */


  	# 16位代码，务必加上
  	.code16
 	.text
	.extern loader_entry
	.global _start
_start:
	# 栈和段等沿用之前的设置,也可以重新设置
	# 这里简单起见，就不做任何设置了
	# 你可能会想，直接跳到loader_entry，但这样需要先知识loader_entry在哪儿
	# boot没有这个能力做到，所以直接用汇编，以便_start处于整个bin文件开头，这样boot直接跳到开头就可以
    jmp loader_entry

	# 32位保护模式下的代码
	.code32
	.text
	.global protect_mode_entry
	.extern load_kernel
protect_mode_entry:
	# 重新加载所有的数据段描述符
	mov $16, %ax		# 16为数据段选择子
	mov %ax, %ds
	mov %ax, %ss
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	# 长跳转进入到32位内核加载模式中
	jmp $8, $load_kernel
