DynamoRIO API Reference:

DynamoRIO gives instruction in binary (instr_t), and its source, destination, and opcode is encoded using opnd_t.

1. Functions to get data on a single instruction (e.g: instr_get_src, instr_get_dst, instr_get_opnd): https://dynamorio.org/dr__ir__instr_8h.html

2. API to get data on a single operand (e.g: opnd_get_addr, opnd_get_int): https://dynamorio.org/dr__ir__opnd_8h.html#a0ab6ce32e2e95004a263028e9879fc7e