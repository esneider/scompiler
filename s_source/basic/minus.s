// minus.s

z1 <- z1 + 1
[a1]
if not(x2) != 0 goto[a2]
if not(x1) != 0 goto[a2]
x1 <- x1 - 1
x2 <- x2 - 1
if z1 != 0 goto[a1]
[a2]
y <- identity(x1)