# littlesat

This is a toy SAT solver I wrote as an exercise in learning C++, which uses a
lightly parallelised version of the basic DPLL algorithm.

### Description

The DPLL algorithm works on the basis of *unit propagation*. First any unit
clauses are propagated. Then, one of the remaining literals is selected, and
the algorithm recursively attempts to find a solution under the assumption of
that literal being true, or if that fails, under the assumption of that literal
being false. Either an empty clause is derived, in which case the formula is
unsatisfiable, or all the clauses are eliminated by the propagation process,
in which case the formula has a satisfying assignment.

This solver parallelises on the assumption step, as it spawns two threads
(using `std::async`) to attempt to solve both branches of the assumption in
parallel (i.e. one thread attempts to solve under the assumption that the
literal is true, the other attempts to solve under the assumption is false);
the spawning then thread waits for both workers to finish. Every time one of
these branches occurs (where threads are spawned), a global branch count is
incremented. Once this branch counter reaches a certain threshold, no more
threads are spawned (to prevent forkbombing the system), and the two sides of
the assumption will be attempted sequentially (as decribed above). If a thread
finds a solution then it sets a global flag to indicate to other threads that
they should stops searching and return.

The solver's literal selection heuristic (for deciding which literal whose
value to assume) is also *very* simple, as it always selects the first literal
in the first remaining clause.

### Compiling

The solver is implemented in C++ 17 (mostly so I could use `std::optional`
like one would use `Maybe` in Haskell or Idris), and can be compiled by running
`make`. The default compiler is set to `clang++`, but this can be changed by
setting the `CXX` make variable.

The branching threshold described above is set at compile time, in the
`concurrency.hh` header file. The number of active threads at any one time is
approximately equal to the branching rate (each branch increases the number of
active threads by one -- two threads are spawned, but the parent thread then
blocks on both child threads finishing), so a sensible value for this setting
is the number of cores on the host machine.

### Running

The solver accepts SAT problems given in DIMACS CNF format. If no arguments are
provided, then the problem specification is read on standard input, or the file
containing the problem can be given as an argument on the command line.

### Performance

This is a fairly naive exponential time algorithm, so the solver is pretty
limited in the size of problems it can (quickly) solve. I've tested it on a
24-core Xeon E5-2640 system running Fedora 31 with the branching threshold
set to 20 with a handful of 3-SAT instances, and instances with more than 100
variables start to take a noticable amount of time for the solver to find a
solution.

### License

Licensed under the ISC license; Copyright (c) 2020 Molly Miller. See the
[LICENSE](LICENSE.txt) file for full details.

