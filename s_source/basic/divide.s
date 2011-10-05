// divide.s
if not(x2) != 0 goto [E]
z1 <- z1+1
[a1]
if less(x1,x2) != 0 goto [E]
x1 <- minus(x1,x2)
y <- y+1
if z1 != 0 goto [a1]