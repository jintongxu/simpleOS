# 项目部署

这里我们的直接用虚拟机了，想来想去感觉用虚拟机最方便了，直接把我的虚拟机文件复制过去就能运行项目了。因为这个需要qemu的窗口输入命令，所以用docker部署感觉有点不太方便。

## 前期准备

这里用到的虚拟机软件是**VMware**，如果没有安装的可以在百度网盘上下载：https://pan.baidu.com/s/1mDlmSEfLLC2BU9_0DllEhQ?pwd=sq9m 。提取码：sq9m 。**VMware激活密钥：** NZ4RR-FTK5H-H81C1-Q30QH-1V2LA

**我的系统镜像**可以从百度网盘上下载。百度网盘[链接](https://pan.baidu.com/s/1YDQgjFz3LheZ_9zl7BrTkg?pwd=xwh5)：https://pan.baidu.com/s/1YDQgjFz3LheZ_9zl7BrTkg?pwd=xwh5 ，提取码：xwh5 。

## 部署

1，打开vmware页面，然后点击打开虚拟机。

![](https://img.xujintong.com/2023/10/29/2e20f2e1edbaf.webp)

2，然后找到下载的系统文件中的OS_Li.vmx，然后选中打开这个虚拟系统就ok了。

![](https://img.xujintong.com/2023/10/29/9dc2ddc91d972d9.webp)

3，输入密码：**hellolinux**

![](https://img.xujintong.com/2023/10/29/9f57fc8555741.webp)

4，然后在终端输入``code``，打开**VSCode**。

5，项目的文件放在**home**目录下了，在VSCode中打开项目文件夹。

![](https://img.xujintong.com/2023/10/29/407a4a396bb.webp)

6，按图进行操作。此时会生成一个``build``文件夹，如果编译失败，可以将build文件夹删除，然后重新生成。

![](https://img.xujintong.com/2023/10/29/a79f2b23622a27.webp)

构建成功

![](https://img.xujintong.com/2023/10/29/7689c46a612dd0b.webp)

7， 按一下**F5**进入调试页面。

![](https://img.xujintong.com/2023/10/29/4a7e5bdab0edc55.webp)

8，再按一次**F5**，中间的黄条没了就，点击左边的那个软件图标，就打开shell面板了。就可以输入命令进行愉快的玩耍了。

![](https://img.xujintong.com/2023/10/29/d10810420.webp)