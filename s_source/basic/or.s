// or.s
{inline}

if x1 != 0 goto[a1]
if x2 != 0 goto[a1]
x1 <- x1 + 1
if x1 != 0 goto[E]
[a1]
y <- y + 1