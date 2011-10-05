// isprime.s

if or( equal(x1,0), equal(x1,1) ) != 0 goto [E]
if or( equal(x1,2), equal(x1,3) ) != 0 goto [a2]

z1 <- sqrt(x1)
z1 <- z1-1
z2 <- z2+1

[a1]
z1 <- z1-1
z2 <- z2+1
if not(mod(x1,z2)) != 0 goto [E]
if z1 != 0 goto [a1]

[a2]
y <- y+1