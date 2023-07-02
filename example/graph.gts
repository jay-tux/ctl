// nodes
NODE INITIAL init (p, q)
NODE INITIAL ACCEPTING alt (p)
NODE second (q)
NODE third (p, q, r, s)
NODE fourth (p, q, r)
NODE ACCEPTING final (r, s)

// transitions
TRANS init -> second
TRANS init -> final
TRANS init -> init
TRANS alt -> init
TRANS second -> third
TRANS second -> fourth
TRANS third -> init
TRANS third -> fourth
TRANS fourth -> second
TRANS fourth -> final
TRANS final -> second