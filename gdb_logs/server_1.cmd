set logging file gdb_logs/server_1.log
set logging on
set history filename gdb_logs/server_1.cmd
set history size unlimited
show history
set history size 1024
show history
set history save on
show history
b psu_thread.c:172
info b
r localhost 1
info r
info f
disas foo
p/x thread_info.uctx_user_func 
p/x thread_info.user_func_stack 
n
info r
info f
p/x thread_info.uctx_user_func 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.user_func_stack 
p/x &thread_info.user_func_stack[2047]
n
info r
info f
p/x thread_info.uctx_user_func 
p/x thread_info.user_func_stack 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
n
info f
info r
p/x thread_info.uctx_user_func 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.user_func_stack 
n
info r
info f
p/x thread_info.uctx_user_func 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.user_func_stack 
p user_func_offset 
p/x user_func
p/x user_func + user_func_offset 
n
p/x thread_info.uctx_user_func 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
b app1.c:31
info b
n
info r
info f
p/x thread_info.uctx_user_func 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.user_func_stack 
s
info r
info f
p/x thread_info.uctx_user_func 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.user_func_stack 
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x &thread_info.user_func_stack[2043]
p/x &thread_info.user_func_stack[2047]
p 0x804d504 - 0x804b508
p/x &thread_info.user_func_stack[0]
p/x &thread_info.user_func_stack[2048]
b app1.c:23
n
p/x thread_info.user_func_stack 
c
q
