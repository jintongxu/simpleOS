**这里的命令实现，不像Linux那么多的参数，只实现了一些参数。**

## help

这个没啥好说的，就是shell中输入``help``会输出一些提示信息。

## clear

这个是清屏命令，和Linux的一样。

## echo

打印字符串命令，加个``-n``后面跟个数字，表示打印多少次。（注意：这里的字符串不能加双引号，这是和平时不一样的。）

![](https://img.xujintong.com/-PMqsJqa5Lo/d0a6eb7cee893.webp)

## ls

终端输入``ls``，列出根目录。（这里只实现了，列出根目录中的文件）

## less

less命令查看文件内容，加个参数``- l``，后面接文件名字。这里每次只输出1行文件内容，按``n``继续输出下一行内容。

![](https://img.xujintong.com/-bVKPeTJBbt/8cb55eb0.webp)

## cp

复制文件命令，``cp  源文件 目的文件``。

![](https://img.xujintong.com/-7NipPdpCUh/f0846e11c2524.webp)

## rm

删除文件命令，后面接个文件名就OK了。