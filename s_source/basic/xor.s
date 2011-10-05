// xor.s
{inline}

// y <- and( or( x1, x2 ), not( and( x1, x2 ) ) )

if x1 != 0 goto [a1]
if x2 != 0 goto [a2]
x1 <- x1 + 1
if x1 != 0 goto [E]
[a1] if x2 != 0 goto [E]
[a2] y <- y+1
