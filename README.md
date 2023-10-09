MPStuBS - Multiprozessor Studenten Betriebssystem
=================================================

This directory contains the source code for the exercises of the
[operating systems lecture](https://sys.cs.fau.de/lehre/ws/bs).

Each assignment should be solved in a separate git branch and
submit as *Merge Request* into the `master` branch,
where it is reviewed by the tutors.

Coding Guidelines
-----------------

Similar to [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) but with following exceptions:
 - No license boilerplate
 - *Tabs* instead of *Spaces*
 - Line length of 120 characters
 - `#pragma once` instead of `#include` guards

The code should be *self-documenting*, don't state the obvious!
However, this does not make comments superfluous:
Since good naming is sometimes not enough, more advanced parts need to be documented,
so any operating system developer should be able to easily understand your code.

### Naming Convention

 - **Variables**: lowercase with underscore

       char* variable_name;

 - **Constants** (and **enum** values): uppercase with underscore

       const int CONST_VALUE = 42;

 - **Type Names** (`class`/`struct`/`namespace`/`enum`): Capital letter, camel case

       class SomeClassName;

 - **Methods/Functions** (C++): start with lowercase letter, then camel case

       void someFunctionName();

 - **extern "C" Functions**: lowercase with underscore (like variables).

       void interrupt_handler(int vector);

 - **File Names**: lowercase, main type name, underscores only if is a sub type

       folder/classname.cc
