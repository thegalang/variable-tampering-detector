# Checklist

- [x] Implement tool logging feature to log global variable changed values (feature 0)
- [x] Implement feature to check if number of global variable change exceeds certain threshold (feature 1)
- [ ] Implement feature to check if global variable is only changed inside certain function (feature 2)
- [x] Implement demo app as intended
- [x] Write a demo app exploit using ROP to showcase tool feature 1
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


```
cd apps
make
```

then run the tool 

```
$DynamoRIO_HOME/bin64/drrun -c ../build/libglobal_var_write_detect.so global_write_config -- global_write.out
```

# Running Tools on Global Write Program

We provide a simple global write program to test logging and constraint capabilities of the tool. This program reads from standard input until `EOF` and randomly assigns the three global variables the value read in a round robin style. We provide `input.txt` and `inputs_large.txt` for benchmarking purposes. These inputs can be utilized by

```
./global_write.out < input.txt
```

and similary, when running the tool

```
$DynamoRIO_HOME/bin64/drrun -c ../build/libglobal_var_write_detect.so global_write_config -- global_write.out < input.txt
```

Note: there is a convenience script `run_global_write_wtool.sh` to run the above command.


# Running Tools on vulnerable program

In this section, we will do experiment to see how our tool prevents malicious programs to corrupt variables of a binary from the program `company_app.c`. All apps are in the `apps` folder.

For easier and consistent result, disable ASLR.

## Credentials

Two available credentials to login into the app:

CEO  
username: ceo  
password: ceo123  

Employee  
username: employee  
password: employee123  

CEO can do both sell_stocks and print_ebitda, while employee can only do print_ebitda.


## Normal Execution

This is the "expected" behaviour of the program without any exploits

```
bash run_normal_program.sh
```

You can then try to login using CEO and verify you can sell_stocks and print_ebitda. Then login to employee account and can only print_ebitda

## ROP Exploit

In this exploit, we will use buffer overflow vulnerability in `print_ebitda` and ROP technique to corrupt the `current_user_id` variable of the binary. 

### Set up

We modify the return address so after `print_ebitda` , `set_current_user_id(1)` is called (CEO user_id). Then, `sell_stocks` is called.

Since this is an x64 architecture, arguments are stored in the `rdi` register, so we also need the `pop $rdi` gadget to set its value. 

There is a function `stat_fmts/rop_exploit_app/payload_builder.c` available to build the payload. Get the address of `set_current_user_id`, `sell_stocks`, and `pop rdi` gadget and insert it into the appropriate variable.

### Run Without Tool
To run the exploit without the tool, run:
```
bash run_rop_exploit.sh
``` 

Then login as `employee` and do `print_ebitda`. You will that sell_stocks will be successfull.

### Run With Tool

To run the exploit with our tool, first compile the tool with `bash build_tool.sh` in root directory. Then in `apps` directory, run:

```
bash run_rop_wtool.sh
```

After logging in as `employee` and do `print_ebitda`, the tool will detct variable tampering and terminate the program. 

Note that the addresses of functions and variables will be different when running in app and in dynamorio, so you need to also get these addresses when running from dynamorio. Both can be obtained by running inside gdb.

### Proof of Concept

![](https://files.catbox.moe/p7trov.png)

# Known bugs

- [ ] Functions that only translates to one line in assembly wont be executed inside the dynamorio tool. For example: void hello(int x) { y = x; }
- [ ] Smashing the stack in x64 will cause all printf functions to get SIGSEV

# DynamoRIO API Reference:

DynamoRIO gives instruction in binary (instr_t), and its source, destination, and opcode is encoded using opnd_t.

1. Main methodology on using dynamorio: https://www.google.com/search?channel=fs&client=ubuntu&q=code+manipulation+api

2. Functions to get data on a single instruction (e.g: instr_get_src, instr_get_dst, instr_get_opnd): https://dynamorio.org/dr__ir__instr_8h.html

3. API to get data on a single operand (e.g: opnd_get_addr, opnd_get_int): https://dynamorio.org/dr__ir__opnd_8h.html#a0ab6ce32e2e95004a263028e9879fc7e

4. Encoding and decoding new instructions: https://dynamorio.org/API_BT.html#sec_decode

5. Utils, main functionality: https://dynamorio.org/group__drutil.html

6. Fetching global variable/function addresses from their names: https://dynamorio.org/page_drsyms.html
