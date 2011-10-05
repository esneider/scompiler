// power.s

if not(x1) != 0 goto [E]
z1 <- z1 + 1
y <- y + 1
[a1]
if not(x2) != 0 goto [E]
x2 <- x2 - 1
y <- times( y, x1 )
if z1 != 0 goto [a1]