#include "dr_api.h"
#include "drmgr.h"
#include "drreg.h"
#include "drutil.h"
#include "drx.h"
#include "dr_defines.h"

static drx_buf_t *memfile_buf;

#ifdef WINDOWS
	#define DISPLAY_STRING(msg) dr_messagebox(msg)
#else
	#define DISPLAY_STRING(msg) dr_printf("%s\n", msg)
#endif

void *mutex;

static void instrument_mem(void *drcontext, instrlist_t *ilist, instr_t *where, opnd_t ref, bool write)
{
	/* reserve to registers for our memory address*/
	reg_id_t reg_ptr, reg_tmp;
	if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_ptr) != DRREG_SUCCESS  || 
		drreg_reserve_register(drcontext, ilist, where, NULL, &reg_tmp) != DRREG_SUCCESS) {
		DR_ASSERT(false); /* cannot recover */
		return;
	}


	int opcode = instr_get_opcode(where);
	opnd_t src = instr_get_src(where, 0);
	opnd_t dst = instr_get_dst(where, 0);
	if(write == 1) {

		int src_new_value = opnd_get_int(src);
		void* dst_address = opnd_get_addr(dst);
		dr_printf("memory write (opcode %d) from %d to %d\n", opcode, src_new_value, dst_address);
	}

	// uint32_t size = drutil_opnd_mem_size_in_bytes(ref, where);
	// drutil_insert_get_mem_addr(drcontext, ilist, where, ref, reg_tmp, reg_ptr);

	// drx_buf_insert_load_buf_ptr(drcontext, memfile_buf, ilist, where, reg_ptr);

	// // writes the memory address (stored in reg_tmp)
	// drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, DR_REG_NULL, opnd_create_reg(reg_tmp), OPSZ_PTR, offsetof(memfile_t, addr)); 
	
	// // writes the memory value
	// drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT64(0), OPSZ_8, offsetof(memfile_t, value));

	// // writes the size
	// drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(size), OPSZ_4, offsetof(memfile_t, size));
	
	// // is it read or write? determined from per_instr
	// drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(write?1:0), OPSZ_4, offsetof(memfile_t, status));

	if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
	    drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS)
		DR_ASSERT(false);
}

static dr_emit_flags_t per_insn_instrument(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, 
		                             bool for_trace, bool translating, void *user_data)
{
	drmgr_disable_auto_predication(drcontext, bb);
	if (!instr_is_app(instr)) return DR_EMIT_DEFAULT;

	//uint32_t mem_count = 0;
	int i;
	for (i = 0; i < instr_num_srcs(instr); i++) {
		if (opnd_is_memory_reference(instr_get_src(instr, i)))
		{
			instrument_mem(drcontext, bb, instr, instr_get_src(instr, i), false);
		}
	}

	for (i = 0; i < instr_num_dsts(instr); i++) {
		if (opnd_is_memory_reference(instr_get_dst(instr, i)))
		{
			instrument_mem(drcontext, bb, instr, instr_get_dst(instr, i), true);
		}
	}

	//if (drmgr_is_first_instr(drcontext, instr) IF_AARCHXX(&& !instr_is_exclusive_store(instr)))
	//	dr_insert_clean_call(drcontext, bb, instr, (void *)save_insn, false, 0);
	return DR_EMIT_DEFAULT;
}

static void event_exit(void) {

	/* display results */
	char msg[512];
	int len;
	// write something

	DR_ASSERT(len > 0);
	msg[sizeof(msg)/sizeof(msg[0])-1] = '\0'; // msg[511]
	DISPLAY_STRING(msg);

	dr_mutex_destroy(mutex);
}

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[]) {

	// register events
	dr_register_exit_event(event_exit);
	drmgr_register_bb_instrumentation_event(NULL, per_insn_instrument, NULL);

	mutex = dr_mutex_create();
}
