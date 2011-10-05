// max

[a]
if not(or( x1, x2 )) != 0 goto[E]
y<-y+1
x1<-x1-1
x2<-x2-1
if y != 0 goto[a]