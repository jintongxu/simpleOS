# SimilarLinux0.11

SimilarLinux0.11项目是一个简单版的32位操作系统，支持8个终端页面，实现了一下简单的系统调用命令（ls，echo等等）。项目通过qemu来模拟32位操作系统运行所需要的硬件环境。本项目用作学习使用。

## 一、最终成果展示

shell功能

- 内部命令：help、clear、echo、ls、less、cp、rm。
- 支持加载磁盘上的文件执行。

进程管理相关的接口实现了9个，文件系统相关的接口实现了13个。

![](https://img.ricemoon.cn/images/9e7796e75d0ff58348d6825ed7376517.zhongduan.webp)



![](https://img.ricemoon.cn/images/49db39c594fb3aa1f2b403a63b3a50fb.ls.webp)

## 二、项目介绍

### 设计结构

**boot**只占据第0个扇区，大小只有512字节。让系统支持多进程，同时并发去运行。开启了分页机制，将进程与进程之间进行隔离。 

![](https://img.ricemoon.cn/images/6d3f155d1b977391d593fb0a5a4a00ca.%C3%A6%C2%93%C2%8D%C3%A4%C2%BD%C2%9C%C3%A7%C2%B3%C2%BB%C3%A7%C2%BB%C2%9F%C3%A8%C2%AE%C2%BE%C3%A8%C2%AE%C2%A1%C3%A7%C2%BB%C2%93%C3%A6%C2%9E%C2%84.webp)

### 中断管理

利用了CPU的IDT表进行管理，外部硬件用两块8259芯片，对键盘的事件进行相关的处理。

![](https://img.ricemoon.cn/images/aab8752c12cd841c11d0979eb3b1a1b1.%C3%A4%C2%B8%C2%AD%C3%A6%C2%96%C2%AD%C3%A7%C2%AE%C2%A1%C3%A7%C2%90%C2%86.webp)

### 多进程

使用TSS进行进程切换。

给每个进程定义了一个``task_t``的结构，在里面增加了一个TSS的描述符，用TSS进行进程切换。

![](https://img.ricemoon.cn/images/9082f9dba9c229058a93b90c67a00be7.%C3%A4%C2%BD%C2%BF%C3%A7%C2%94%C2%A8TSS%C3%A8%C2%BF%C2%9B%C3%A7%C2%A8%C2%8B%C3%A5%C2%88%C2%87%C3%A6%C2%8D%C2%A2.webp)

展示了如何手动切换进程。

保存相关寄存器到当前任务的栈，然后通过栈切换实现手动切换进程。

![](https://img.ricemoon.cn/images/b1c6ced71f84b2f34f978d38451ca5a7.%C3%A5%C2%A4%C2%9A%C3%A8%C2%BF%C2%9B%C3%A7%C2%A8%C2%8B1.webp)

允许系统中同时存在许多进程，不同进程间采用时间片调试算法进行管理。

定义一个链表，将进行按先进先出的方式插入到链表中，每100ms切换进程运行。

![](https://img.ricemoon.cn/images/e1950b185c3cd3b84baaea643f46866a.%C3%A5%C2%A4%C2%9A%C3%A8%C2%BF%C2%9B%C3%A7%C2%A8%C2%8B2.webp)

### 进程同步

使用**计数信号量**解决行为同步的问题。对于多个进程访问共享资源，我们用**互斥锁**保证同一时间只有一个进程访问临界区。

![](https://img.ricemoon.cn/images/b43301565be3e313fbd86d6c2b983d5f.%C3%A8%C2%BF%C2%9B%C3%A7%C2%A8%C2%8B%C3%A5%C2%90%C2%8C%C3%A6%C2%AD%C2%A5.webp)

### 内存管理

操作系统开启了分页机制，应用访问的线性内存需要通过转换表转换成对应的物理地址。

![](https://img.ricemoon.cn/images/daaaaf4c4950b20397a34c01e7542274.%C3%A5%C2%86%C2%85%C3%A5%C2%AD%C2%98%C3%A7%C2%AE%C2%A1%C3%A7%C2%90%C2%862.webp)

利用分页机制，让操作系统占用0x80000000以下的区域，进程则占用以上的区域。通过让每个进程都拥有自己的页表实现进程隔离。

![](https://img.ricemoon.cn/images/68f15e9688c2d4f6881ebb0c9dca0429.%C3%A5%C2%86%C2%85%C3%A5%C2%AD%C2%98%C3%A7%C2%AE%C2%A1%C3%A7%C2%90%C2%861.webp)

### 权限管理

操作系统运行于特权级0，应用进程运行于特权级3。还通过分页机制对某些页做了特权处理，应用程序不能运行于特权级0的地址空间。

![](https://img.ricemoon.cn/images/1f6dff30d73770625ea4d08e5a59bda9.%C3%A6%C2%9D%C2%83%C3%A9%C2%99%C2%90%C3%A7%C2%AE%C2%A1%C3%A7%C2%90%C2%86.webp)

### 系统调用

使用**调用门**提供相应的系统调用接口来给应用程序使用。

![](https://img.ricemoon.cn/images/0f42cc2a3230f33367ac2bdb21c6c1e0.%C3%A7%C2%B3%C2%BB%C3%A7%C2%BB%C2%9F%C3%A8%C2%B0%C2%83%C3%A7%C2%94%C2%A8.webp)

### ELF文件加载

解析ELF头，再解析表，提取出对应的数据拷贝到相对应的进程地址空间。

![](https://img.ricemoon.cn/images/2c85647c2ccd22aa28d453c947770b09.ELF%C3%A6%C2%96%C2%87%C3%A4%C2%BB%C2%B6%C3%A5%C2%8A%C2%A0%C3%A8%C2%BD%C2%BD.webp)

### 设备管理

设备管理层定义统一的设备操作接口，方便上层使用。

将所有不同的设备统一抽象成统一的接口，使操作系统内核只需要统一的操作接口就能实现设备的访问。

![](https://img.ricemoon.cn/images/08683786206822fc6e0d2c757b5d530b.%C3%A8%C2%AE%C2%BE%C3%A5%C2%A4%C2%87%C3%A7%C2%AE%C2%A1%C3%A7%C2%90%C2%86.drawio.webp)

### 文件系统

![](https://img.ricemoon.cn/images/c80975b895df70762e5c02e1cfdc03b8.%C3%A6%C2%96%C2%87%C3%A4%C2%BB%C2%B6%C3%A7%C2%B3%C2%BB%C3%A7%C2%BB%C2%9F1.webp)

![](https://img.ricemoon.cn/images/9ccc6492d59a6c1e1913cd95f62c5644.%C3%A6%C2%96%C2%87%C3%A4%C2%BB%C2%B6%C3%A7%C2%B3%C2%BB%C3%A7%C2%BB%C2%9F2.webp)

### shell

Shell俗称壳，即命令行解释器，它允许用户交互式的输入命令并解释执行，并且可以调用出相应的应用程序运行，从而让用户能够使用内核的功能去操作计算机。

Shell支持应用从磁盘中加载程序进行运行。

![](https://img.ricemoon.cn/images/064b2ee40da454253d6630ac0f0db9b6.shell.webp)

## 三、不足和展望

### 不足

- 本项目只是demo级别的32位操作系统，许多接口和功能的实现非常简单，并且有安全漏洞。

- 和真正的Linux0.11还有很大的距离。

### 展望

- 多加几个系统调用接口。
- 提高某些功能的实现方式，例如将进程调度的**时间片轮转调度**切换成**多级反馈队列调度**，或者由多个调度算法进行进程的调度。

- 增加**TCP/IP网络协议栈**
- 可以支持用户**多线程**。