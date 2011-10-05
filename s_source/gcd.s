// gcd.s

// hacky, fast, gcd

// if a>b
//    a-=b
// if a<b
//    b-=a
// if a=b return a

z10 <- z10+1 // aux

if less(x1,x2) != 0 goto [a1]
		z1 <- minus(x1,x2)
		if z10 != 0 goto [a2]
	[a1]
		z2 <- minus(x2,x1)
[a2]

if not( or(z1,z2) ) != 0 goto [f]

[a3]
	if z1 != 0 goto [b1]
			z3 <- identity(x1)
			[c1]
				x2 <- x2-1
				z3 <- z3-1
				z2 <- z2-1
				if and(not(z2),z3) != 0 goto [c2]
			if z3 != 0 goto [c1]
			if z10 != 0 goto [a4]
			[c2]
				x2 <- x2-1
				z3 <- z3-1
				z1 <- z1+1
			if z3 != 0 goto [c2]
			if z10 != 0 goto [a4]
		[b1]
			z3 <- identity(x2)
			[c3]
				x1 <- x1-1
				z3 <- z3-1
				z1 <- z1-1
				if and(not(z1),z3) != 0 goto [c4]
			if z3 != 0 goto [c3]
			if z10 != 0 goto [a4]
			[c4]
				x1 <- x1-1
				z3 <- z3-1
				z2 <- z2+1
			if z3 != 0 goto [c4]
[a4]
if or(z1,z2) != 0 goto [a3]

[f]
y <- identity(x1)