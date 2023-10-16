#include "dr_api.h"
#include "drreg.h"
#include "drutil.h"
#include "drx.h"
#include "dr_defines.h"
#include "drmgr.h"


#ifdef WINDOWS
	#define DISPLAY_STRING(msg) dr_messagebox(msg)
#else
	#define DISPLAY_STRING(msg) dr_printf("%s\n", msg)
#endif

void *mutex;

static void instrument_mem_write(void *drcontext, instrlist_t *ilist, instr_t *where)
{


	int opcode = instr_get_opcode(where);

	opnd_t src = instr_get_src(where, 0);
	opnd_t dst = instr_get_dst(where, 0);

	ptr_int_t new_value = opnd_get_immed_int(src);
	void* dst_address = opnd_get_addr(dst);
	dr_printf("memory write (opcode %d) value %ld to %0x\n", opcode, new_value, dst_address);

}

static dr_emit_flags_t per_insn_instrument(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, 
		                             bool for_trace, bool translating, void *user_data)
{
	if (!instr_is_app(instr)) return DR_EMIT_DEFAULT;

	int i;

	if(instr_num_srcs(instr) == 1 && 
		instr_num_dsts(instr) == 1 && 
		opnd_is_memory_reference(instr_get_dst(instr, 0))) {

		instrument_mem_write(drcontext, bb, instr);
	} 

	//	dr_insert_clean_call(drcontext, bb, instr, (void *)save_insn, false, 0);
	return DR_EMIT_DEFAULT;
}

static dr_emit_flags_t bb_instrumentation_analyze(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating, void **user_data) {
	
	return DR_EMIT_DEFAULT;
}


static void event_exit(void) {

	/* display results */
	// char msg[512];
	// int len;
	// // write something

	// DR_ASSERT(len > 0);
	// msg[sizeof(msg)/sizeof(msg[0])-1] = '\0'; // msg[511]
	// DISPLAY_STRING(msg);

	drmgr_exit();
	dr_mutex_destroy(mutex);
}

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[]) {

	drmgr_init();

	// register events
	dr_register_exit_event(event_exit);
	drmgr_register_bb_instrumentation_event(bb_instrumentation_analyze, per_insn_instrument, NULL);

	mutex = dr_mutex_create();
}
