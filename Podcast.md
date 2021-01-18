### [Jim Keller](https://www.youtube.com/watch?v=Nb2tebYAaOA)

- computer is built based on abstraction layers
  - atom -> silicon -> transitor -> logic gates -> funtional units (adder, subtractor) -> cpu
  - instruction set (how encoding the basic operations like load, store, branding ...) -> assembly language -> c/c++ -> javascript
- in the old days, you fetched instructions then executed them in order. Modern computer, you fetch large number of instructions, then you compute a dependency graph, then you execute it in the way that gets a correct answer like reading a book, you could find sentences that can read in anti order without changing the meaning <- found parallelism in narrative
- 20 years ago, branch prediction did via simple recording which way the branch went last time and we got ~85%. Today, we use something looks like neural network based on all execution flows. There is probably a little supercomputer inside a computer to do that job
