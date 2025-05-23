#define DO_MOV_AM();	DO_AM_I();
#define DO_ADD_AM();	DO_AM_I();
#define DO_SUB_AM();	DO_AM_I();
#define DO_CMP_AM();	DO_AM_I();
#define DO_SHL_AM();	DO_AM_I();
#define DO_SHR_AM();	DO_AM_I();
#define DO_JMP_AM();	DO_AM_I();
#define DO_SAR_AM();	DO_AM_I();
#define DO_MUL_AM();	DO_AM_I();
#define DO_DIV_AM();	DO_AM_I();
#define DO_MULU_AM();	DO_AM_I();
#define DO_DIVU_AM();	DO_AM_I();
#define DO_OR_AM();	DO_AM_I();
#define DO_AND_AM();	DO_AM_I();
#define DO_XOR_AM();	DO_AM_I();
#define DO_NOT_AM();	DO_AM_I();
#define DO_MOV_I_AM();	DO_AM_II();
#define DO_ADD_I_AM();	DO_AM_II();
#define DO_SETF_AM();	DO_AM_II();
#define DO_CMP_I_AM();	DO_AM_II();
#define DO_SHL_I_AM();	DO_AM_II();
#define DO_SHR_I_AM();	DO_AM_II();
#define DO_EI_AM();	DO_AM_II();
#define DO_SAR_I_AM();	DO_AM_II();
#define DO_TRAP_AM();	DO_AM_II();
#define DO_RETI_AM();	DO_AM_IX();
#define DO_HALT_AM();	DO_AM_IX();
#define DO_LDSR_AM();	DO_AM_II();
#define DO_STSR_AM();	DO_AM_II();
#define DO_DI_AM();	DO_AM_II();
#define DO_BSTR_AM();	DO_AM_BSTR();
#define DO_MOVEA_AM();	DO_AM_V();
#define DO_ADDI_AM();	DO_AM_V();
#define DO_JR_AM();	DO_AM_IV();
#define DO_JAL_AM();	DO_AM_IV();
#define DO_ORI_AM();	DO_AM_V();
#define DO_ANDI_AM();	DO_AM_V();
#define DO_XORI_AM();	DO_AM_V();
#define DO_MOVHI_AM();	DO_AM_V();
#define DO_LD_B_AM();	DO_AM_VIa();
#define DO_LD_H_AM();	DO_AM_VIa();
#define DO_LD_W_AM();	DO_AM_VIa();
#define DO_ST_B_AM();	DO_AM_VIb();
#define DO_ST_H_AM();	DO_AM_VIb();
#define DO_ST_W_AM();	DO_AM_VIb();
#define DO_IN_B_AM();	DO_AM_VIa();
#define DO_IN_H_AM();	DO_AM_VIa();
#define DO_CAXI_AM();	DO_AM_VIa();
#define DO_IN_W_AM();	DO_AM_VIa();
#define DO_OUT_B_AM();	DO_AM_VIb();
#define DO_OUT_H_AM();	DO_AM_VIb();
#define DO_FPP_AM();	DO_AM_FPP();
#define DO_OUT_W_AM();	DO_AM_VIb();
#define DO_BV_AM();	DO_AM_III();
#define DO_BL_AM();	DO_AM_III();
#define DO_BE_AM();	DO_AM_III();
#define DO_BNH_AM();	DO_AM_III();
#define DO_BN_AM();	DO_AM_III();
#define DO_BR_AM();	DO_AM_III();
#define DO_BLT_AM();	DO_AM_III();
#define DO_BLE_AM();	DO_AM_III();
#define DO_BNV_AM();	DO_AM_III();
#define DO_BNL_AM();	DO_AM_III();
#define DO_BNE_AM();	DO_AM_III();
#define DO_BH_AM();	DO_AM_III();
#define DO_BP_AM();	DO_AM_III();
#define DO_NOP_AM();	DO_AM_III();
#define DO_BGE_AM();	DO_AM_III();
#define DO_BGT_AM();	DO_AM_III();


#define DO_INVALID_AM(); DO_AM_UDEF();


#define DO_WAIT_BE_AM();     DO_AM_III();
#define DO_WAIT_BNE_AM();    DO_AM_III();

#define DO_WAIT_BP_AM();    DO_AM_III();

#define DO_WAIT_BL_AM();    DO_AM_III();

#define DO_WAIT_BLT_AM();    DO_AM_III();

#define DO_WAIT_BR_AM();    DO_AM_III();

