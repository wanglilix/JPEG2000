
#ifndef __MQC_H
#define __MQC_H
#include"openjpeg.h"
/**
@file mqc.h
@brief Implementation of an MQ-Coder (MQC)

The functions in MQC.C have for goal to realize the MQ-coder operations. The functions
in MQC.C are used by some function in T1.C.
*/

/** @defgroup MQC MQC - Implementation of an MQ-Coder */
/*@{*/

/**
This struct defines the state of a context.
（MQ编码器状态）
*/
typedef struct mi_mqc_state {
	/** the probability of the Least Probable Symbol (0.75->0x8000, 1.5->0xffff) */
	mi_UINT32 qeval;
	/** the Most Probable Symbol (0 or 1) */
	mi_UINT32 mps;
	/** next state if the next encoded symbol is the MPS */
	struct mi_mqc_state *nmps;
	/** next state if the next encoded symbol is the LPS */
	struct mi_mqc_state *nlps;
} mi_mqc_state_t;

#define MQC_NUMCTXS 19

/**
MQ coder
*/
typedef struct mi_mqc {
	mi_UINT32 c;//C
	mi_UINT32 a;//A
	mi_UINT32 ct;//区间转换次数
	mi_BYTE *bp;//当前编码位置的前一个位置
	mi_BYTE *start;//编码开始位置
	mi_BYTE *end;//编码终止位置
	mi_mqc_state_t *ctxs[MQC_NUMCTXS];//上下文索引（CX）的MQ编码器状态
	mi_mqc_state_t **curctx;//一个二维的编码器状态的数组
#ifdef MQC_PERF_OPT
	unsigned char *buffer;
#endif
} mi_mqc_t;

/** @name Exported functions */
/*@{*/
/* ----------------------------------------------------------------------- */
/**
Create a new MQC handle 
@return Returns a new MQC handle if successful, returns NULL otherwise
*/
mi_mqc_t* mi_mqc_create(void);
/**
Destroy a previously created MQC handle
@param mqc MQC handle to destroy
*/
void mi_mqc_destroy(mi_mqc_t *mqc);
/**
Return the number of bytes written/read since initialisation
@param mqc MQC handle
@return Returns the number of bytes already encoded
*/
mi_UINT32 mi_mqc_numbytes(mi_mqc_t *mqc);
/**
Reset the states of all the context of the coder/decoder 
(each context is set to a state where 0 and 1 are more or less equiprobable)
@param mqc MQC handle
*/
void mi_mqc_resetstates(mi_mqc_t *mqc);
/**
Set the state of a particular context
@param mqc MQC handle
@param ctxno Number that identifies the context
@param msb The MSB of the new state of the context
@param prob Number that identifies the probability of the symbols for the new state of the context
*/
void mi_mqc_setstate(mi_mqc_t *mqc, mi_UINT32 ctxno, mi_UINT32 msb, mi_INT32 prob);
/**
Initialize the encoder
@param mqc MQC handle
@param bp Pointer to the start of the buffer where the bytes will be written
*/
void mi_mqc_init_enc(mi_mqc_t *mqc, mi_BYTE *bp);
/**
Set the current context used for coding/decoding
@param mqc MQC handle
@param ctxno Number that identifies the context
*/
#define mi_mqc_setcurctx(mqc, ctxno)	(mqc)->curctx = &(mqc)->ctxs[(mi_UINT32)(ctxno)]
/**
Encode a symbol using the MQ-coder
@param mqc MQC handle
@param d The symbol to be encoded (0 or 1)
*/
void mi_mqc_encode(mi_mqc_t *mqc, mi_UINT32 d);
/**
Flush the encoder, so that all remaining data is written
@param mqc MQC handle
*/
void mi_mqc_flush(mi_mqc_t *mqc);
/**
BYPASS mode switch, initialization operation. 
JPEG 2000 p 505. 
<h2>Not fully implemented and tested !!</h2>
@param mqc MQC handle
*/
void mi_mqc_bypass_init_enc(mi_mqc_t *mqc);
/**
BYPASS mode switch, coding operation. 
JPEG 2000 p 505. 
<h2>Not fully implemented and tested !!</h2>
@param mqc MQC handle
@param d The symbol to be encoded (0 or 1)
*/
void mi_mqc_bypass_enc(mi_mqc_t *mqc, mi_UINT32 d);
/**
BYPASS mode switch, flush operation
<h2>Not fully implemented and tested !!</h2>
@param mqc MQC handle
@return Returns 1 (always)
*/
mi_UINT32 mi_mqc_bypass_flush_enc(mi_mqc_t *mqc);
/**
RESET mode switch
@param mqc MQC handle
*/
void mi_mqc_reset_enc(mi_mqc_t *mqc);
/**
RESTART mode switch (TERMALL)
@param mqc MQC handle
@return Returns 1 (always)
*/
mi_UINT32 mi_mqc_restart_enc(mi_mqc_t *mqc);
/**
RESTART mode switch (TERMALL) reinitialisation
@param mqc MQC handle
*/
void mi_mqc_restart_init_enc(mi_mqc_t *mqc);
/**
ERTERM mode switch (PTERM)
@param mqc MQC handle
*/
void mi_mqc_erterm_enc(mi_mqc_t *mqc);
/**
SEGMARK mode switch (SEGSYM)
@param mqc MQC handle
*/
void mi_mqc_segmark_enc(mi_mqc_t *mqc);
/**
Initialize the decoder
@param mqc MQC handle
@param bp Pointer to the start of the buffer from which the bytes will be read
@param len Length of the input buffer
*/
mi_BOOL mi_mqc_init_dec(mi_mqc_t *mqc, mi_BYTE *bp, mi_UINT32 len);
/**
Decode a symbol
@param mqc MQC handle
@return Returns the decoded symbol (0 or 1)
*/
mi_INT32 mi_mqc_decode(mi_mqc_t * const mqc);
/* ----------------------------------------------------------------------- */
/*@}*/

/*@}*/

#endif /* __MQC_H */
