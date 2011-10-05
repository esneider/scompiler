// fibrec.s

if not(x1) != 0 goto [E]
x1 <- x1-1
y <- y+1
if not(x1) != 0 goto [E]

y <- fibrec(x1)
x1 <- x1-1
y <- plus( y, fibrec(x1) )
