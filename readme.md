# Checklist

- [x] Implement tool logging feature to log global variable changed values (feature 0)
- [x] Implement feature to check if number of global variable change exceeds certain threshold (feature 1)
- [ ] Implement feature to check if global variable is only changed inside certain function (feature 2)
- [x] Implement demo app as intended
- [ ] Write a demo app exploit using ROP to showcase tool feature 1
- [ ] Write a demo app exploit using format string exploit to showcase tool feature 2

# Compile Tools

set env variable $DynamoRIO_HOME to your dynamorio home. i.e: /home/student/dynamorio

run 
```
bash build_tools.sh
```

A folder `build` should be created alongside with the compiled tools.

# Running Tools

`apps` folder is the test apps we used tools on. First compile the app

(TODO MAKEFILE)

```
gcc global_write.cpp -o bin/global_write
```

then run the tool 

```
$DynamoPATH -c ../build/libglobal_var_write_detect.so -- bin/global_write
```

# DynamoRIO API Reference:

DynamoRIO gives instruction in binary (instr_t), and its source, destination, and opcode is encoded using opnd_t.

1. Main methodology on using dynamorio: https://www.google.com/search?channel=fs&client=ubuntu&q=code+manipulation+api

2. Functions to get data on a single instruction (e.g: instr_get_src, instr_get_dst, instr_get_opnd): https://dynamorio.org/dr__ir__instr_8h.html

3. API to get data on a single operand (e.g: opnd_get_addr, opnd_get_int): https://dynamorio.org/dr__ir__opnd_8h.html#a0ab6ce32e2e95004a263028e9879fc7e

4. Encoding and decoding new instructions: https://dynamorio.org/API_BT.html#sec_decode

5. Utils, main functionality: https://dynamorio.org/group__drutil.html

6. Fetching global variable/function addresses from their names: https://dynamorio.org/page_drsyms.html

