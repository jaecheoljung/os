#include <core/eos.h>
#include <core/eos_internal.h>
#include "emulator_asm.h"

typedef struct _os_context {
	/* low address */
	int32u_t edi;
	int32u_t esi;
	int32u_t ebp;
	int32u_t esp;
	int32u_t ebx;
	int32u_t edx;
	int32u_t ecx;
	int32u_t eax;
	int32u_t eflags;
	int32u_t eip;
	/* high address */
} _os_context_t;


addr_t _os_create_context(addr_t stack_base, size_t stack_size, void (*entry)(void *), void *arg) {

	addr_t cur = stack_base + stack_size - 4;

	*(int32u_t*)cur = arg,		cur -= 4;
	*(int32u_t*)cur = 0,		cur -= 4;
	*(int32u_t*)cur = entry,	cur -= 4;	//eip
	*(int32u_t*)cur = 1,		cur -= 4;	//eflags
	*(int32u_t*)cur = 0,		cur -= 4;	//eax
	*(int32u_t*)cur = 0,		cur -= 4;	//ecx
	*(int32u_t*)cur = 0,		cur -= 4;	//edx
	*(int32u_t*)cur = 0,		cur -= 4;	//ebx
	*(int32u_t*)cur = 0,		cur -= 4;	//esp
	*(int32u_t*)cur = 0,		cur -= 4;	//ebp
	*(int32u_t*)cur = 0,		cur -= 4;	//esi
	*(int32u_t*)cur = 0;					//edi

	return cur;
}

void _os_restore_context(addr_t sp) {
	__asm__ __volatile__(
			"mov 	%0, %%esp\n"
			"popa\n"
			"pop	_eflags\n"
			"ret\n"
			::"r"(sp)
		);
}

addr_t _os_save_context() {
	__asm__ __volatile__(
			"push	$resume_point\n"
			"push 	_eflags\n"
			"pusha\n"
			"movl	$0, -12(%%ebp)\n"
			"mov 	%%esp, %%eax\n"
			"push 	4(%%ebp)\n"
			"push 	0(%%ebp)\n"
			"mov 	%%esp, %%ebp\n"
			"resume_point:\n"
			"leave\n"
			"ret\n"
			::
		);
}


void print_context(addr_t context) {
	if(context == NULL) return;
	_os_context_t *ctx = (_os_context_t *)context;
	//PRINT("reg1  =0x%x\n", ctx->reg1);
	//PRINT("reg2  =0x%x\n", ctx->reg2);
	//PRINT("reg3  =0x%x\n", ctx->reg3);
	//...
}