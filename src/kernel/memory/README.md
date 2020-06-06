### mOS memory model

WIP

✍️The reason to choose between int32 vs int or int32 vs int8 (if value is in their range) is your limited memory. For performance, you have to measure because there are many involved factors

- how many supported operations on x-bit (8, 16, 32 ...) in your processors
- int8 saves bus/cache bandwidth but a processor has to mask off the relevant bits, perform and write back. In case of int32 on 32-bit, data are aligned -> fetch, add, bitwise ... are faster

[More details](https://www.quora.com/Which-is-faster-Int64-or-Int8)
