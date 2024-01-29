# The Hibiku Programming Language

Hibiku is a programming language designed to help educate people on how to write
interpreters and compilers. This is the reference implementation used in the
book [Zero to Compiler](https://nashiora.com/compiler-book/contents.html).

I realize that writing in C is not necessarily as approachable as could be, but
I want to show how to embed and interpreter as well as run it as a standalone
application, and C makes embedding very easy. Just about anything can reference
a C library, or have bindings written to allow for it.

## How To Build

The build should be as simple as running `make` in this directory. The `hibiku`
executable will be generated in the same directory.

```bash
$ make
```

## Usage

I don't actually know how this is going to be used yet, but you can always check
if there's up to date help information by passing the `--help` argument. As that
fills out, so should this README.

```bash
$ ./hibiku --help
```
