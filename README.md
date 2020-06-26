# CppScript

把C++代码当脚本一样写。
在项目的过程中往往遇到一些这样的情况：有一些代码的业务逻辑比较复杂，而且业务比较容易发生变化。我们有一种方法就是把这部分的业务实现提出去，让脚本的形式运行，例如：lua，js等，但是还要求性能，例如一个数据转发器，不可能在数据转发的过程使用如lua或js吧，怎么办呢？于是就有这个项目了。


# 特性

 - 脚本化
 - 性能和原来的C++一样（因为就是DLL）

# 原理

把vc中的命令行编译工具提取出来。我们写好业务脚本后，保存一个文件，再使用这个命令行编译工具编译、连接后loadlibrary，运行。

# 使用

主程序：[main.cpp](https://github.com/gergul/CppScript/blob/master/CppScript/main.cpp)

脚本：[testScript.cpp](https://github.com/gergul/CppScript/blob/master/testScript.cpp)
	
