### [Jim Keller](https://www.youtube.com/watch?v=Nb2tebYAaOA)

- computer is built based on abstraction layers
  - atom -> silicon -> transitor -> logic gates -> funtional units (adder, subtractor) -> cpu
  - instruction set (how encoding the basic operations like load, store, branding ...) -> assembly language -> c/c++ -> javascript
- in the old days, you fetched instructions then executed them in order. Modern computer, you fetch a large number of instructions, then you compute a dependency graph, then you execute it in the way that gets a correct answer like reading a book, you could find sentences that can read in anti order without changing the meaning <- found parallelism in narrative
- 20 years ago, branch prediction did via simple recording which way the branch went last time and we got ~85%. Today, we use something that looks like a neural network based on all execution flows. There is probably a little supercomputer inside a computer to do that job

### [Brian Kernighan](https://www.youtube.com/watch?v=O9upVbGSBFo)

- [CTSS](https://www.youtube.com/watch?v=Q07PhW5sCEk) -> [Multics](https://en.wikipedia.org/wiki/Multics) -> Unix
  - Timesharing System: in the early days, a computer was a huge/expensive machine and which receives punch cards as input and processes based on them. It took time for each phase, to save money and resources, many teletypes (remote or local) were connected to a computer, each of them gets a slice of time
  - Multics was failed due to too much complicated/heavy (even for hardware) at the time and
- Unix's philosophy is providing an environment that easy to write programs and programmers could be highly productive. Another point creating a community for programmers

### [Bjarne Stroustrup](https://www.youtube.com/watch?v=uTxRF5ag27A)

- Fortran is the first independent-machine language, Simula is the first language that introduces OOP concepts (object, class, ...)
  ![programming language tree](https://i.imgur.com/zlhXiZy.png)
- C++ philosophy is performance + reliability + zero overhead
  > If you want to have reliability, performance or security, you have to look at the whole system
- Once you have the right abstraction, you usually get a better performance/efficiency
  For example: write matrix multiplication in C++ can outperform one written in C with usual facilities except using special embedded assembly code which the compiler doesn't know
