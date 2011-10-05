// times.s

z1 <- z1 + 1
if not(x1) != 0 goto [E]
[a0]
if not(x2) != 0 goto [E]
x2 <- x2 - 1
// not using plus due to loose of eficiency
z2 <- identity(x1)
[a1]
if not(z2) != 0 goto [a0]
z2 <- z2 -1
y <- y + 1
if z1 != 0 goto [a1]