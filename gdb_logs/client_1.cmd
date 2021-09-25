set logging file gdb_logs/client_1.log 
set logging on
set history filename gdb_logs/client_1.cmd 
set history size unlimited
set history size 1024
show history
set history save on
show history
b psu_thread.c:294
info b
r localhost 0
info r
info f
disas foo
p/x thread_info.uctx_user_func
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EBP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_ESP]
p/x thread_info.uctx_user_func.uc_mcontext.gregs[REG_EIP]
p/x thread_info.user_func_stack 
n
p/x thread_info.user_func_stack 
p user_func_offset 
p/x user_func + user_func_offset 
q
c
q
