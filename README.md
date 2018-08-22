# mlua
An interpreter of lua-like language written in C++ 

目前已实现了lua的大部分语句，包括函数，闭包，table等。 虚拟机是基于栈的

一个例子：

```
local f = function(a)
    return function() a = a + 1 return a end 
end


local up = f(12)
local n1 = up()
local n2 = up()
print(n1, n2)

up = f(34)
local n1 = up()
local n2 = up()
print(n1, n2)
```
能正确的打印出
```
13      14
35      36
```
下一步：
+ 实现for, while等复合语句
+ 注释还是C++格式的，单行//，多行 /* */，要改成lua的--
+ 为了快速出结果，堆变量没有释放，有内存泄露，这个要处理好
+ 还没有垃圾回收GC，这个以后要加上去
+ 有时间把基于栈的虚拟机改为基于寄存器的
