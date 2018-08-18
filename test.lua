//local a, b = 159, 236 
//a = 96
//print(a, 78, 89 ,b)


//local b = 10 + 2 * 3
//local a = 1 + b
//print(a)


f = function(a)
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

/*function f(a, b)
    a = a + 1
    b = b * 11
    print(a, b)
    return b / 9
end
local a, b = 159, 7
print(f(a, b, 666))
print(a, b)*/

/*local f = function(a)
    print(a)
end

f(123)*/



//local c = 15
//print(a, b, c)
//local n2 = ff()
//print(n2)



//local a = f()
//print(a)
//local b = f()
//print(b)


//local a, b = f()
//local a, b = f() + 5
//a, b = f()
//a = f()
//a, b = f() + 2
//a = 2 + f()
          

