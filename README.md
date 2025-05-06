
# clox

This program implements a lox interpreter in C. It can (as of now) tokenize, parse, and execute print statements. This interpreter is mostly inspired by the one in Crafting Interpreters.


## Features

- Tokenizes, parses, and runs lox scripts
- Can interpret global variables, arithmetic, logic, and print statements
- Variable assignment supported
- See test_files for working examples


## Acknowledgements

 - [Crafting Interpreters](https://craftinginterpreters.com/)


## Building

Build clox with make and gcc

```bash
make
```

By default, a debug version is built. For an optimized version, run

```bash
make release
```

If any weird building errors occur, run


```bash
make clear && make
```

## Running

To run the clox interpreter, run

```bash
./clox {tokenize | parse | run} your_file.lox
```


## Usage/Examples

```bash
./clox run test_files/run_simple.lox
```
should yield

```bash8
one
true
false
-54
concat
```

