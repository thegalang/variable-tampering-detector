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

void *global_var_address;

/*
goal: Before memory write, check if written address is same as global
if same, store written value in buffer
*/
static void instrument_mem_write(void *drcontext, instrlist_t *ilist, instr_t *where)
{

	reg_id_t reg_ptr, reg_tmp, reg_savedval;

	if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_tmp) !=
        DRREG_SUCCESS) {
        DR_ASSERT(false);
        return DR_REG_NULL;
    }

    if (drreg_reserve_register(drcontext, ilist, where, NULL, &reg_ptr) !=
        DRREG_SUCCESS) {
        DR_ASSERT(false);
        return DR_REG_NULL;
    }
	
	opnd_t src = instr_get_src(where, 0);
	opnd_t dst = instr_get_dst(where, 0);

	uint32_t size = drutil_opnd_mem_size_in_bytes(ref, where);
	drutil_insert_get_mem_addr(drcontext, ilist, where, dst, reg_tmp, reg_ptr);

	// compare if reg_tmp address is same as global address
	// TODO

	// if its a simple constant load
	// reg tmp value is no longer needed. can use to scratch
	if(opnd_is_immed(src)) {

		
		drx_buf_insert_load_buf_ptr(drcontext, memfile_buf, ilist, where, reg_ptr);

		// todo: store immed value in buffer

	} else if(opnd_is_reg(src)) {

		drx_buf_insert_load_buf_ptr(drcontext, memfile_buf, ilist, where, reg_ptr);

		// todo: read reg value of src and store in reg_savedval
		// src is a reg, get it and store value in buffer
		reg_id_t = opnd_get_reg(src);

		// todo: insert immed value in buffer
		drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, DR_REG_NULL, opnd_create_reg(reg_tmp), OPSZ_PTR, offsetof(memfile_t, addr)); 
		drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT64(0), OPSZ_8, offsetof(memfile_t, value));
		drx_buf_insert_buf_store(drcontext, memfile_buf, ilist, where, reg_ptr, reg_tmp, OPND_CREATE_INT32(size), OPSZ_4, offsetof(memfile_t, size));
	}
	


	// ptr_int_t new_value = opnd_get_immed_int(src);
	// void* dst_address = opnd_get_addr(dst);
	// dr_printf("memory write (opcode %d) value %ld to %0x\n", opcode, new_value, dst_address);

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
