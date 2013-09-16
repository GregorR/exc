exc
===

exc is Extensible C, a system for syntactic C language extensions. exc code
compiles to C code. Moreover, exc code /is/ C code, but with a syntactic
"escape", the @ symbol, which provides a very broad syntax for modular syntax
transformations to use as a basis. The syntactic escapes are called
"decorations" or "decorators".

The concept is simple: Need a way of annotating particular references for an
automatic reference counter[1]? Add a `@rc` type syntax extension and generate
the reference counting code when it's encountered. Need a simple syntax for
accessing members of dynamic objects in your C-based domain-specific language?
How about an `@.` operator? Anything you can write as a *syntactic* transform
over regular C code can be written as an extension in exc.

Because @ is not used in normal C, it is perfectly unambiguous with
conventional C code. In fact, conventional C code can be compiled with exc
flawlessly[2]. The purpose of exc is not to replace C, only to extend it.

[1] Not that this is a good idea.

[2] exc is preprocessed, like C, but because preprocessing includes all
    #included headers, the result is usually not portable. To add the *option*
    of generating portable C code, the built-in decoration `@include` is
    supported. Using `@include` is obviously a change from conventional C code,
    but it's strictly optional: exc code which uses traditional #include for
    system headers works fine and is itself portable, it simply generates
    unportable C code in the compilation process.
