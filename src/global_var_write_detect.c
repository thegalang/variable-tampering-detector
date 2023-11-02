#include "dr_api.h"
#include "drreg.h"
#include "drutil.h"
#include "drx.h"
#include "dr_defines.h"
#include "drmgr.h"
#include "drsyms.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WINDOWS
	#define DISPLAY_STRING(msg) dr_messagebox(msg)
#else
	#define DISPLAY_STRING(msg) dr_printf("%s\n", msg)
#endif

#define MINSERT instrlist_meta_preinsert

void *mutex;

void *global_var_address;
int saved_global_var_value;
static int num_global_var_changed;
int global_var_change_limit;
int found_error; // error codes: 0: no error, 1: update limit exceeded


// multiple of 6, OPSZ_8
#define HISTORY_BUF_SIZE 510

drx_buf_t *global_history_buf;
FILE *global_history_fp;

static void flush_global_history(void *drcontext, void *buf_base, size_t size) {

	//dr_pr\n", buf_base);
	//dr_printf("size is %d\n", size);
	int n = size/OPSZ_8;
	// all element are 8-byte integers
	dr_mutex_lock(mutex);

	DR_ASSERT(size % OPSZ_8 == 0);
	
	// manually print one by one since auto methods does not seem to work
	for(int i = 0; i < n; i++) {
		char nextInt[8]; memset(nextInt, 0, sizeof(nextInt));

		strncpy(nextInt, buf_base, OPSZ_8);
		fprintf(global_history_fp, "%d\n", *(int*)nextInt);
		buf_base += OPSZ_8;
	}

	dr_mutex_unlock(mutex);
	return;
}

static inline void insert_terminate_instr(void *drcontext, instrlist_t *ilist, instr_t *where) {

	printf("exit addr: %p\n", exit);
	instr_t* crashProgram = INSTR_CREATE_jmp(drcontext, OPND_CREATE_INTPTR(*exit));
    instr_set_translation(crashProgram, instr_get_app_pc(where));
	instrlist_preinsert(ilist, where, crashProgram);
} 

static inline void insert_when_global_var_is_modified(void *drcontext, instrlist_t *ilist, instr_t *where, reg_id_t reg_ptr, reg_id_t reg_tmp) {

	// update the saved global value to be the current app's global value
	instr_t* loadCurrentGlobalVarValue = INSTR_CREATE_mov_ld(drcontext, opnd_create_reg(reg_tmp), opnd_create_abs_addr(global_var_address, OPSZ_PTR));
    instr_set_translation(loadCurrentGlobalVarValue, instr_get_app_pc(where));
	instrlist_preinsert(ilist, where, loadCurrentGlobalVarValue);

	instr_t* updateSavedGlobalValue = INSTR_CREATE_mov_st(drcontext, opnd_create_abs_addr(&saved_global_var_value, OPSZ_PTR), opnd_create_reg(reg_tmp));
    instr_set_translation(updateSavedGlobalValue, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, updateSavedGlobalValue);

    instr_t* reloadSavedGlobalVarAddress = INSTR_CREATE_mov_ld(drcontext, opnd_create_reg(reg_tmp), opnd_create_abs_addr(&saved_global_var_value, OPSZ_PTR));
    instr_set_translation(reloadSavedGlobalVarAddress, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, reloadSavedGlobalVarAddress);

    // write the global value in the output log (global value history)
	drx_buf_insert_load_buf_ptr(drcontext, global_history_buf, ilist, where, reg_ptr);
	drx_buf_insert_buf_store(drcontext, global_history_buf, ilist, where, reg_ptr, DR_REG_NULL, opnd_create_reg(reg_tmp), OPSZ_8, 0);
	drx_buf_insert_update_buf_ptr(drcontext, global_history_buf, ilist, where, reg_ptr, reg_tmp, OPSZ_PTR);
	

	// increase the counter of the number of times the value changes
    instr_t* increaseChangedGlobalVarCounter = INSTR_CREATE_inc(drcontext, opnd_create_abs_addr(&num_global_var_changed, OPSZ_PTR));
    instr_set_translation(increaseChangedGlobalVarCounter, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, increaseChangedGlobalVarCounter);

    // is change limit exceeded?
    instr_t* isGlobalVarLimitExceeded = INSTR_CREATE_cmp(drcontext, opnd_create_abs_addr(&num_global_var_changed, OPSZ_PTR), OPND_CREATE_INT32(global_var_change_limit));
    instr_set_translation(isGlobalVarLimitExceeded, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, isGlobalVarLimitExceeded);

    instr_t* NotExceedLabel = INSTR_CREATE_label(drcontext);
    instr_set_translation(NotExceedLabel, instr_get_app_pc(where));

    instr_t* jumpIfNotExceed = INSTR_CREATE_jcc(drcontext, OP_jbe, opnd_create_instr(NotExceedLabel));
    instr_set_translation(jumpIfNotExceed, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, jumpIfNotExceed);

    // VARIABLE TAMPERING DETECTED: Modifications limit exceeded
    instr_t* setErrorFlagToModifyLimitExceeded = INSTR_CREATE_mov_st(drcontext, opnd_create_abs_addr(&found_error, OPSZ_PTR), OPND_CREATE_INT32(1));
    instr_set_translation(setErrorFlagToModifyLimitExceeded, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, setErrorFlagToModifyLimitExceeded);

    // terminte program
    insert_terminate_instr(drcontext, ilist, where);


    instrlist_preinsert(ilist, where, NotExceedLabel);
}

/*
goal: after memory write, check address value of global variable,
compare current value with saved global value. if it has changed, log it
*/
static dr_emit_flags_t after_memory_write(void *drcontext, instrlist_t *ilist, instr_t *where)
{


	if(found_error > 0) {
		dr_exit_process(127);
	}

	//dr_printf("%d %d %d\n", saved_global_var_value, *(int*)global_var_address, num_global_var_changed);
	reg_id_t reg_ptr, reg_tmp, reg_flags;


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

    
    // load both current app global var and the saved value in this tool
    instr_t* loadCurrentGlobalVarValue = INSTR_CREATE_mov_ld(drcontext, opnd_create_reg(reg_tmp), opnd_create_abs_addr(global_var_address, OPSZ_PTR));
    instr_set_translation(loadCurrentGlobalVarValue, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, loadCurrentGlobalVarValue);

    instr_t* loadSavedGlobalValue = INSTR_CREATE_mov_ld(drcontext, opnd_create_reg(reg_ptr), opnd_create_abs_addr(&saved_global_var_value, OPSZ_PTR));
    instr_set_translation(loadSavedGlobalValue, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, loadSavedGlobalValue);

    instr_t* saveEflags = INSTR_CREATE_pushf(drcontext);
    instr_set_translation(saveEflags, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, saveEflags);

    // compare their values. If they are same then ignore the next instructions
    instr_t* isGlobalVarChange = INSTR_CREATE_cmp(drcontext, opnd_create_reg(reg_ptr), opnd_create_reg(reg_tmp));
    instr_set_translation(isGlobalVarChange, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, isGlobalVarChange);

    instr_t* labelContinue = INSTR_CREATE_label(drcontext);
    instr_set_translation(labelContinue, instr_get_app_pc(where));

    instr_t* continueIfEqual = INSTR_CREATE_jcc(drcontext, OP_je, opnd_create_instr(labelContinue));
    instr_set_translation(continueIfEqual, instr_get_app_pc(where));
    instrlist_preinsert(ilist, where, continueIfEqual);

    
    // if global value has changed, these instructions will check if its a valid change
    insert_when_global_var_is_modified(drcontext, ilist, where, reg_ptr, reg_tmp);
     

	instrlist_preinsert(ilist, where, labelContinue);
	
	
	instr_t* restoreEflags = INSTR_CREATE_popf(drcontext);
    instr_set_translation(restoreEflags, instr_get_app_pc(where));
	instrlist_preinsert(ilist, where, restoreEflags);

	
	if (drreg_unreserve_register(drcontext, ilist, where, reg_ptr) != DRREG_SUCCESS ||
	    drreg_unreserve_register(drcontext, ilist, where, reg_tmp) != DRREG_SUCCESS
	    )
		DR_ASSERT(false);

	return DR_EMIT_DEFAULT;


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

		
		instr_t* nextInstr = instr_get_next(instr);
		return after_memory_write(drcontext, bb, nextInstr);
	} 

	//	dr_insert_clean_call(drcontext, bb, instr, (void *)save_insn, false, 0);
	return DR_EMIT_DEFAULT;
}

static dr_emit_flags_t bb_instrumentation_analyze(void *drcontext, void *tag, instrlist_t *bb, bool for_trace, bool translating, void **user_data) {
	
	return DR_EMIT_DEFAULT;
}


static void event_exit(void) {


	// free buffers
	drx_buf_free(global_history_buf);

	// closing files
	fclose(global_history_fp);

	drmgr_exit();
	drx_exit();
	drutil_exit();
	drreg_exit();
	drsym_exit();
	dr_mutex_destroy(mutex);

	if(found_error == 1) {
		dr_printf("VARIABLE TAMPERING DETECTED: Modifications limit exceeded\n");
	}

}


DR_EXPORT void
dr_client_main(client_id_t id, int argc, const char *argv[]) {

	drmgr_init();
	drx_init();
	drutil_init();
	drreg_options_t ops = {sizeof(ops), 4, false};
	drreg_init(&ops);
	drsym_init(0);

	if(argc < 2) {
		dr_printf("config file args missing");
		DR_ASSERT(false);
	}

	FILE *configFP = fopen(argv[1], "r");

	// read global var name and get its address
	char globalVarName[100]; memset(globalVarName, 0, sizeof(globalVarName));
	fscanf(configFP, "%s", globalVarName);

	// register event to search for global var symbol
	//drmgr_register_module_load_event(module_load_event);

	module_data_t* main_module = dr_get_main_module();
	global_var_address = dr_get_proc_address(main_module->handle, globalVarName);
    saved_global_var_value = *(int*)global_var_address;
	num_global_var_changed = 0;

	fscanf(configFP, "%d", &global_var_change_limit);

	// register buffers
	global_history_buf = drx_buf_create_trace_buffer(HISTORY_BUF_SIZE, flush_global_history);

	// open file pointers
	global_history_fp = fopen("global_history_log.out", "w");
	fprintf(global_history_fp, "Tracked global variable history:\n");
	// register events
	dr_register_exit_event(event_exit);
	drmgr_register_bb_instrumentation_event(bb_instrumentation_analyze, per_insn_instrument, NULL);

	mutex = dr_mutex_create();
}
