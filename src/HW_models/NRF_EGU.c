/*
 * Copyright (c) 2022, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <irq_ctrl.h>
#include "NRF_EGU.h"
#include "NRF_PPI.h"

#include "bs_tracing.h"

#define PPI_SIDEEFFECT_TASK(EGU_ID, CHANNEL) \
do {\
	NRF_EGU_regs[EGU_ID].TASKS_TRIGGER[CHANNEL] = 1;\
	nrf_egu_regw_sideeffects_TASKS_TRIGGER(&NRF_EGU_regs[EGU_ID]);\
} while(0)

#define NRF_EGU_TASK_TRIGGER_DEFINE(id, channel) \
void nrf_egu_ ## id ## _TASK_TRIGGER_ ## channel(void) {PPI_SIDEEFFECT_TASK(id,channel);}

#define NRF_EGU_0_TASK_TRIGGER_DEFINE(channel) NRF_EGU_TASK_TRIGGER_DEFINE(0, channel)
#define NRF_EGU_1_TASK_TRIGGER_DEFINE(channel) NRF_EGU_TASK_TRIGGER_DEFINE(1, channel)
#define NRF_EGU_2_TASK_TRIGGER_DEFINE(channel) NRF_EGU_TASK_TRIGGER_DEFINE(2, channel)
#define NRF_EGU_3_TASK_TRIGGER_DEFINE(channel) NRF_EGU_TASK_TRIGGER_DEFINE(3, channel)
#define NRF_EGU_4_TASK_TRIGGER_DEFINE(channel) NRF_EGU_TASK_TRIGGER_DEFINE(4, channel)
#define NRF_EGU_5_TASK_TRIGGER_DEFINE(channel) NRF_EGU_TASK_TRIGGER_DEFINE(5, channel)

// implementation of task triggers for all EGU: 
// void nrf_egu_0_TASK_TRIGGER_0(void) { 
// 		NRF_EGU_regs[0].TASKS_TRIGGER[0] = 1;
//		nrf_egu_regw_sideeffects_TASKS_TRIGGER(&NRF_EGU_regs[0]);
// }
FOR_EACH(NRF_EGU_0_TASK_TRIGGER_DEFINE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_1_TASK_TRIGGER_DEFINE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_2_TASK_TRIGGER_DEFINE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_3_TASK_TRIGGER_DEFINE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_4_TASK_TRIGGER_DEFINE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_5_TASK_TRIGGER_DEFINE, ( ), CHANNEL_LIST)

#define EGU0_EVENTS_TRIGGERED(N, _) EGU0_EVENTS_TRIGGERED_ ## N
#define EGU1_EVENTS_TRIGGERED(N, _) EGU1_EVENTS_TRIGGERED_ ## N
#define EGU2_EVENTS_TRIGGERED(N, _) EGU2_EVENTS_TRIGGERED_ ## N
#define EGU3_EVENTS_TRIGGERED(N, _) EGU3_EVENTS_TRIGGERED_ ## N
#define EGU4_EVENTS_TRIGGERED(N, _) EGU4_EVENTS_TRIGGERED_ ## N
#define EGU5_EVENTS_TRIGGERED(N, _) EGU5_EVENTS_TRIGGERED_ ## N

const ppi_event_types_t PPI_EGU_EVENTS[EGU_COUNT][EGU0_CH_NUM] = {
	{
		//EGU0_EVENTS_TRIGGERED_0, ...
		//EGU0_EVENTS_TRIGGERED_15
		LISTIFY(EGU0_CH_NUM, EGU0_EVENTS_TRIGGERED, (,))
	},
	{
		LISTIFY(EGU0_CH_NUM, EGU1_EVENTS_TRIGGERED, (,))
	},
	{
		LISTIFY(EGU0_CH_NUM, EGU2_EVENTS_TRIGGERED, (,))
	},
	{
		LISTIFY(EGU0_CH_NUM, EGU3_EVENTS_TRIGGERED, (,))
	},
	{
		LISTIFY(EGU0_CH_NUM, EGU4_EVENTS_TRIGGERED, (,))
	},
	{
		LISTIFY(EGU0_CH_NUM, EGU5_EVENTS_TRIGGERED, (,))
	},
};

/**
 * @brief Get Interrupt Id of corresponding EGU peripheral
 *
 * @param p_reg NRF_EGU_Type pointer
 * @return IRQn for p_reg
 */
#define REG_TO_IRQ(p_reg)   (REG_TO_ID(p_reg) + SWI0_EGU0_IRQn)

/**
 * @brief Check if channel has enabled interrupts
 *
 * @param p_reg NRF_EGU_Type pointer
 * @param channel uint8_t channel number to check
 * @return True if interrupt is enabled for channel in p_reg, otherwise return False
 */
#define CHANNEL_INTERRUPT_FLAG(p_reg, channel)    (p_reg->INTEN & nrf_egu_channel_int_get(channel))


NRF_EGU_Type NRF_EGU_regs[EGU_COUNT];
// prevent enabling already enabled IRQ
bool EGU_interrupt_enabled[EGU_COUNT];

void nrf_egu_regw_sideeffects_TASKS_TRIGGER(NRF_EGU_Type *p_reg)
{
	// move tasks to triggers
	memcpy(p_reg->EVENTS_TRIGGERED, p_reg->TASKS_TRIGGER, sizeof(p_reg->EVENTS_TRIGGERED));
	// task has been moved to trigger, zero out the register for future use
	memset(p_reg->TASKS_TRIGGER, 0, sizeof(p_reg->TASKS_TRIGGER));
	nrf_egu_regw_sideeffects_EVENTS_TRIGGERED_INTEN(p_reg);
}

void nrf_egu_regw_sideeffects_EVENTS_TRIGGERED_INTEN(NRF_EGU_Type *p_reg)
{
	for (uint8_t i = 0; i < nrf_egu_channel_count(p_reg); i++) {
		if (CHANNEL_INTERRUPT_FLAG(p_reg, i) && p_reg->EVENTS_TRIGGERED[i]) {
			// if there is some task, and interrupt is enabled - set irq
			if (!EGU_interrupt_enabled[REG_TO_ID(p_reg)]) {
				EGU_interrupt_enabled[REG_TO_ID(p_reg)] = true;
				hw_irq_ctrl_set_irq(REG_TO_IRQ(p_reg));
			}
			return;
		}
	}
	// if there are not active tasks with enabled interrupt - clear irq
	if (EGU_interrupt_enabled[REG_TO_ID(p_reg)]) {
		EGU_interrupt_enabled[REG_TO_ID(p_reg)] = false;
		hw_irq_ctrl_clear_irq(REG_TO_IRQ(p_reg));
	}
}

void nrf_egu_regw_sideeffects_INTENSET(NRF_EGU_Type *p_reg)
{
	if (p_reg->INTENSET) {
		p_reg->INTEN |= p_reg->INTENSET;
		p_reg->INTENSET = p_reg->INTEN;
	}
	nrf_egu_regw_sideeffects_EVENTS_TRIGGERED_INTEN(p_reg);
}

void nrf_egu_regw_sideeffects_INTENCLR(NRF_EGU_Type *p_reg)
{
	if (p_reg->INTENCLR) {
		p_reg->INTEN &= ~p_reg->INTENSET;
		p_reg->INTENSET = p_reg->INTEN;
		p_reg->INTENCLR = 0;
	}
	nrf_egu_regw_sideeffects_EVENTS_TRIGGERED_INTEN(p_reg);
}
