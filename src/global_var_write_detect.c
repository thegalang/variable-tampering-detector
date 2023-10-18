#include "dr_api.h"
#include "drreg.h"
#include "drutil.h"
#include "drx.h"
#include "dr_defines.h"
#include "drmgr.h"
#include <stdio.h>
#include <string.h>

#ifdef WINDOWS
	#define DISPLAY_STRING(msg) dr_messagebox(msg)
#else
	#define DISPLAY_STRING(msg) dr_printf("%s\n", msg)
#endif


void *mutex;

void *global_var_address;

// multiple of 6, OPSZ_8
#define HISTORY_BUF_SIZE 510

drx_buf_t *global_history_buf;
FILE *global_history_fp;

static void flush_global_history(void *drcontext, void *buf_base, size_t size) {

	int n = size/OPSZ_8;
	// all element are 8-byte integers
	dr_mutex_lock(mutex);

	DR_ASSERT(size % OPSZ_8 == 0);
	
	// manually print one by one since auto methods does not seem to work
	for(int i = 0; i < n; i++) {
		char nextInt[8]; memset(nextInt, 0, sizeof(nextInt));

		strncpy(nextInt, buf_base, OPSZ_8);
		fprintf(global_history_fp, "%lld\n", *(long long*)nextInt);
		buf_base += OPSZ_8;
	}

	dr_mutex_unlock(mutex);
	return;
}

/*
goal: Before memory write, check if written address is same as global
if same, store written value in buffer
*/
static dr_emit_flags_t instrument_mem_write(void *drcontext, instrlist_t *ilist, instr_t *where)
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

	drutil_insert_get_mem_addr(drcontext, ilist, where, dst, reg_tmp, reg_ptr);

	drx_buf_insert_load_buf_ptr(drcontext, global_history_buf, ilist, where, reg_ptr);

	// TODO: change to actual inputted global variable
	if(opnd_get_addr(dst) == 0x00007ffff3dd6018) {
	
		// if its a simple constant load
		// reg tmp value is no longer needed. can use to scratch
		if(opnd_is_immed(src)) {

			

			u_int64_t immed_value = opnd_get_immed_int(src);
			drx_buf_insert_buf_store(drcontext, global_history_buf, ilist, where, reg_ptr, reg_tmp, opnd_create_immed_int(immed_value, OPSZ_8), OPSZ_8, 0);
			drx_buf_insert_update_buf_ptr(drcontext, global_history_buf, ilist, where, reg_ptr, reg_tmp, OPSZ_8);

		} else if(opnd_is_reg(src)) {

			// src is a reg, get it and store value in buffer
			reg_id_t src_reg = opnd_get_reg(src);
			drx_buf_insert_buf_store(drcontext, global_history_buf, ilist, where, reg_ptr, DR_REG_NULL, opnd_create_reg(src_reg), OPSZ_8, 0);
			drx_buf_insert_update_buf_ptr(drcontext, global_history_buf, ilist, where, reg_ptr, reg_tmp, OPSZ_8);

		}
	}

	// TODO: case when address input is a register must be handled
	
	if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
	    drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS)
		DR_ASSERT(false);

	return DR_EMIT_DEFAULT;
	// ptr_int_t new_value = opnd_get_immed_int(src);
	// void* dst_address = opnd_get_addr(dst);
	// dr_printf("memory write (opcode %d) value %ld to %0x\n", opcode, new_value, dst_address);

}

static dr_emit_flags_t per_insn_instrument(void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, 
		                             bool for_trace, bool translating, void *user_data)
{
	if (!instr_is_app(instr)) return DR_EMIT_DEFAULT;

	int i;

	// proceed further if its a memory write operation
	if(instr_num_srcs(instr) == 1 && 
		instr_num_dsts(instr) == 1 && 
		opnd_is_memory_reference(instr_get_dst(instr, 0))) {

		return instrument_mem_write(drcontext, bb, instr);
	} 

	//	dr_insert_clean_call(drcontext, bb, instr, (void *)save_insn, false, 0);
	return DR_EMIT_DEFAULT;
}

static dr_emit_flags_t bb_instrumentation_analyze(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating, void **user_data) {
	
	return DR_EMIT_DEFAULT;
}

int glob1 = 0;

static void event_exit(void) {

	// free buffers
	drx_buf_free(global_history_buf);

	// closing files
	fclose(global_history_fp);

	drmgr_exit();
	drx_exit();
	drutil_exit();
	drreg_exit();
	dr_mutex_destroy(mutex);


}

DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[]) {

	drmgr_init();
	drx_init();
	drutil_init();
	drreg_options_t ops = {sizeof(ops), 4, false};
	drreg_init(&ops);

	// register buffers
	global_history_buf = drx_buf_create_trace_buffer(HISTORY_BUF_SIZE, flush_global_history);

	// open file pointers
	global_history_fp = fopen("global_history_log.out", "w");
	fprintf(global_history_fp, "History of global variable:\n");
	// register events
	dr_register_exit_event(event_exit);
	drmgr_register_bb_instrumentation_event(bb_instrumentation_analyze, per_insn_instrument, NULL);

	mutex = dr_mutex_create();
}
