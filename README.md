# mlua
An interpreter of lua-like language written in C++ 

安装：

windows:  用vs2013以上版本直接打开mlua.vcxproj文件即可。因为使用到了c++11语法，所以需要vs2013及以上版本。

Linux：   待续

目前已实现了lua的大部分语句，包括if语句，函数，闭包，table，for循环等。 虚拟机目前是基于栈的。

闭包的例子：

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

if语句，递归的例子：
```
function fib(n)
    if n == 0 then return 1
    elseif n == 1 then return 1
    else 
        return fib(n - 1) + fib(n - 2)
    end
end

local ret = fib(15)
print(ret)
```
能正确打印出987

泛型for循环：
```
local ipairsIter = function(t, i)
  i = i + 1
  local v = t[i]
  if v then
    return i, v, 10
  end
end
local tt = {12, 78}
for k, v, m in ipairsIter, tt, 0 do
    print(k, v, m)
end
print(20)
```
能正确打印出：
```
1   12  20
2   78  20
```

下一步：
+ 实现for, while等复合语句
+ 注释还是C++格式的，单行//，多行 /* */，要改成lua的--
+ 为了快速出结果，堆变量没有释放，有内存泄露，这个要处理好
+ 还没有垃圾回收GC，这个以后要加上去
+ 有时间把基于栈的虚拟机改为基于寄存器的
