/*
 * cmsis_os_itron_wrap.c
 *
 *  Created on: 2017/09/16
 *      Author: masatakanabeshima
 */
#include "cmsis_os.h"
#include <itron.h>

struct os_thread_cb {
	int dummy;
};

struct os_mutex_cb {
	int dummy;
};

struct os_semaphore_cb {
	int dummy;
};

struct os_messageQ_cb {
	int dummy;
};

struct os_mailQ_cb {
	int dummy;
};

static int32_t s_isKernelRunning = 0;

osStatus osKernelInitialize (void)
{
	kos_init_kernel();

	return osOK;
}

osStatus osKernelStart (void)
{
	s_isKernelRunning = 1;
	kos_start_kernel();

	return osOK;
}

int32_t osKernelRunning(void)
{
	return s_isKernelRunning;
}

uint32_t osKernelSysTick (void)
{
	SYSTIM tim;

	get_tim(&tim);

	return (uint32_t)tim;
}

osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
	T_CTSK ctsk;
	PRI pri;
	ID tskid;

	pri = (PRI)((int)osPriorityRealtime - (int)thread_def->tpriority) + 1;

	ctsk.tskatr = TA_ACT;
	ctsk.itskpri = pri;
	ctsk.stk = NULL;
	ctsk.stksz = (SIZE)(thread_def->stacksize == 0 ? 512 : thread_def->stacksize);
	ctsk.exinf = (VP_INT)argument;
	ctsk.task = (FP)thread_def->pthread;

	tskid = acre_tsk(&ctsk);

	if(tskid == KOS_E_NOID) {
		return NULL;
	}

	return (osThreadId)tskid;
}

osStatus osDelay (uint32_t millisec)
{
	dly_tsk(millisec);
	return osOK;
}

osSemaphoreId osSemaphoreCreate (const osSemaphoreDef_t *semaphore_def, int32_t count)
{
	T_CSEM csem;
	ID semid;

	csem.isemcnt = count;
	csem.maxsem = 0xFFFFFFFF;
	csem.sematr = 0;

	semid = acre_sem(&csem);

	if(semid == E_NOID) {
		return NULL;
	}

	return (osSemaphoreId)semid;
}

int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec)
{
	ID semid = (ID)semaphore_id;
	ER er;

	if(millisec == 0) {
		er = pol_sem(semid);
	} else if(millisec == osWaitForever) {
		er = wai_sem(semid);
	} else {
		er = twai_sem(semid, millisec);
	}

	switch(er) {
	case E_OK:
		{
			T_RSEM rsem;
			er = ref_sem(semid, &rsem);
			return er == E_OK ? rsem.semcnt : -1;
		}
	default:
		return -1;
	}
}

osStatus osSemaphoreRelease (osSemaphoreId semaphore_id)
{
	ID semid = (ID)semaphore_id;
	ER er;

	if(sns_ctx()) {
		er = isig_sem(semid);
	} else {
		er = sig_sem(semid);
	}

	switch(er) {
	case E_OK:
		return osOK;
	default:
		return osErrorOS;
	}
}

osStatus osSemaphoreDelete (osSemaphoreId semaphore_id)
{
	ID semid = (ID)semaphore_id;
	ER er;

	er = del_sem(semid);

	switch(er) {
	case E_OK:
		return osOK;
	default:
		return osErrorOS;
	}
}

osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id)
{
	T_CDTQ cdtq;
	ID dtqid;

	cdtq.dtq = NULL;
	cdtq.dtqatr = TA_TFIFO;
	cdtq.dtqcnt = queue_def->queue_sz;

	dtqid = acre_dtq(&cdtq);

	if(dtqid == E_NOID) {
		return NULL;
	}

	return (osThreadId)dtqid;
}

