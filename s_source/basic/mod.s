// mod.s

if not(x2) != 0 goto [E]
z1 <- z1+1
[a1]
if less(x1,x2) != 0 goto [a2]
x1 <- minus(x1,x2)
if z1 != 0 goto [a1]
[a2]
y <- identity(x1)