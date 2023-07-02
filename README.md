# CTL
*A simple CTL Model Checker in C++.*

## How To Use
1) How to build:
```sh
mkdir build && cd build && cmake .. && cmake --build .
```
2) How to run:
```sh
./ctl path/to/file/containing/transition/system path/to/file/containing/ctl/formula
```
Now, the program will show you in which states your formula holds, and then tell you whether or not the model satisfies the formula (at least one initial state is satisfying).

## Transition System Definitions
(see [example/graph.gts](./example/graph.gts) for an example).

In the regexes below, `<>` mean placeholders (shouldn't contain spaces) and `[]` mark sub-groups of the regex. All characters other than `*` and `?` are mandatory in the code.

1) Defining nodes: ``NODE [INITIAL|ACCEPTING]* <name> ([<proposition>[, <proposition>]*]?)``, or in human language:
  - Keyword `NODE`, 
  - then one or more of `INITIAL` (mark a state as initial) and/or `ACCEPTING` (mark a state as accepting, currently unused), 
  - then the name of the state, 
  - then zero or more atomic propositions (labels); separated by spaces and surrounded by parentheses.

ONE NODE PER LINE

2) Defining transitions: `TRANS <node 1> -> <node 2>`, or in human language:
  - Keyword `TRANS`,
  - then the name of the first (start) node,
  - then the name of the second (destination) node.

ONE TRANSITION PER LINE, CAN'T USE NODES BEFORE THEIR DECLARATION

Additionally, you can start a line with `//` to mark a comment.

## CTL Formulae
(see [example/formula.ctl](./example/formula.ctl) for an example).

Formulae should be given in ECTL (existential normal form). This means we support the following operators/tokens:
 - the literal `true`, `TRUE`, `True` (all equivalent);
 - atomic propositions (alphanumerical and `_`);
 - negation using `!`;
 - conjunction using `/\`;
 - exists-next (there exists a successor where `X` holds): using `\E \X` (these two should be next to each other);
 - exists-always (there exists a path such that `X` holds everywhere): using `\E \G` (these two should be next to each other);
 - exists-until (there exists a path such that `X` holds until `Y` holds, and `Y` holds eventually): using `\E <...> \U <...>` (`X` should be between `\E` and `\U`).

 It is highly recommended to use parentheses when using multiple consecutive unary operators (so don't write `!\E\X \E\G p` but `! (\E\X (\E\G p))`). Otherwise, the parser will crash ;).
