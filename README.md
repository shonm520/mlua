# mlua
An interpreter of lua-like language written in C++ 

技术细节详细讲解博客：

https://blog.csdn.net/column/details/25937.html

### 安装：
git clone https://github.com/shonm520/mlua.git

windows:  用vs2013以上版本直接打开mlua.vcxproj文件即可。因为使用到了c++11语法，所以需要vs2013及以上版本。

Linux :  g++  \*.cpp libs/\*.cpp -o mlua -std=c++11

目前已实现了lua的大部分语句，包括if语句，函数，闭包，table，for循环等。 虚拟机目前是基于栈的。

### 函数与闭包的例子：

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

### if语句，函数递归调用的例子：
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

### 泛型for循环：
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
```
能正确打印出：
```
1   12  10
2   78  10
```

### table的例子
```
t = {name = 'shonm', 28, sex = 'male', game = {name = 'Glory of the king'}}

t.func = function(str) 
	    print(str, t.name, 'age is ', t[1], 'sex is ', t.sex, 'he plays', t.game.name) 
	end
t.func('hello')
```

### 部分内置函数和类库的例子
```
print(string.len('hello world'))
print(string.upper('world'))
print(math.pow(2, 6))
print(type({}))
```

### 元表的例子：
```
fa = {house = 3}
son = {car = 2}
fa.__index = fa
setmetatable(son, fa)
print(son.house)
```
能打印出预期结果3


### 面向对象多态特性
```
cat = {}
function cat.call()
	print('cat call maomao~~~')
end

dog = {}
dog.call = function ()
	print('dog call wangwang~~~')
end

function test_duck(duck)
	duck.call()
end

test_duck(cat)
test_duck(dog)
```
结果为：
```
cat call maomao~~~
dog call wangwang~~~
```



### 下一步：

+ 注释还是C++格式的，单行//，多行 /* */，要改成lua的--
+ 为了快速出结果，堆变量没有释放，有内存泄露，这个要处理好
+ 实现协程
+ 还没有垃圾回收GC，这个以后要加上去
+ 有时间把基于栈的虚拟机改为基于寄存器的

### 交流QQ群 858791125 

