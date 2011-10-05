// sqrt.s

x1 <- x1+1
z1 <- z1+1
[a1]
z2 <- identity(z1)
[a2]
z2 <- z2-1
x1 <- x1-1
if z2 != 0 goto [a2]
if not(x1) != 0 goto [E]
z1 <- z1+1
z1 <- z1+1
y <- y+1
if z1 != 0 goto [a1]