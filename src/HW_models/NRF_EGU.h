/*
 * Copyright (c) 2022, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _NRF_HW_MODEL_EGU_H
#define _NRF_HW_MODEL_EGU_H

#include "nrfx.h"
#include "hal/nrf_egu.h"
#include "NRF_PPI.h"

#ifdef __cplusplus
extern "C"{
#endif

extern NRF_EGU_Type NRF_EGU_regs[EGU_COUNT];
extern const ppi_event_types_t PPI_EGU_EVENTS[EGU_COUNT][EGU0_CH_NUM];

/**
 * @brief return instance number for EGU peripheral
 *
 * @param p_reg NRF_EGU_Type pointer
 * @return Number of EGU instance
 *
 */
#define REG_TO_ID(p_reg)    (((size_t)p_reg - (size_t)(&NRF_EGU_regs[0])) / sizeof(NRF_EGU_Type))

/**
 * @brief return ID of task in EGU
 *
 * @param task nrf_egu_task_t 
 * @return channel of the task
 *
 */
#define TASK_TO_ID(task)    ((size_t)task - NRF_EGU_TASK_TRIGGER0) / sizeof(NRF_EGU_regs[0].TASKS_TRIGGER[0])

/**
 * @brief return ID of event in EGU
 *
 * @param event nrf_egu_event_t 
 * @return channel of the event
 *
 */
#define EVENT_TO_ID(event)    ((size_t)event - NRF_EGU_EVENT_TRIGGERED0) / sizeof(NRF_EGU_regs[0].EVENTS_TRIGGERED[0])

#define EVAL_ID(ID, _) ID

// 0,1,2, ... ,EGU0_CH_NUM
#define CHANNEL_LIST LISTIFY(EGU0_CH_NUM, EVAL_ID, (,))

#define NRF_EGU_TASK_TRIGGER_DECLARE(id, channel) \
void nrf_egu_ ## id ## _TASK_TRIGGER_ ## channel(void);

#define NRF_EGU_0_TASK_TRIGGER_DECLARE(channel) NRF_EGU_TASK_TRIGGER_DECLARE(0, channel)
#define NRF_EGU_1_TASK_TRIGGER_DECLARE(channel) NRF_EGU_TASK_TRIGGER_DECLARE(1, channel)
#define NRF_EGU_2_TASK_TRIGGER_DECLARE(channel) NRF_EGU_TASK_TRIGGER_DECLARE(2, channel)
#define NRF_EGU_3_TASK_TRIGGER_DECLARE(channel) NRF_EGU_TASK_TRIGGER_DECLARE(3, channel)
#define NRF_EGU_4_TASK_TRIGGER_DECLARE(channel) NRF_EGU_TASK_TRIGGER_DECLARE(4, channel)
#define NRF_EGU_5_TASK_TRIGGER_DECLARE(channel) NRF_EGU_TASK_TRIGGER_DECLARE(5, channel)


FOR_EACH(NRF_EGU_0_TASK_TRIGGER_DECLARE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_1_TASK_TRIGGER_DECLARE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_2_TASK_TRIGGER_DECLARE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_3_TASK_TRIGGER_DECLARE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_4_TASK_TRIGGER_DECLARE, ( ), CHANNEL_LIST)
FOR_EACH(NRF_EGU_5_TASK_TRIGGER_DECLARE, ( ), CHANNEL_LIST)

void nrf_egu_regw_sideeffects_TASKS_TRIGGER(NRF_EGU_Type * p_reg);
void nrf_egu_regw_sideeffects_EVENTS_TRIGGERED_INTEN(NRF_EGU_Type * p_reg);
void nrf_egu_regw_sideeffects_INTENSET(NRF_EGU_Type * p_reg);
void nrf_egu_regw_sideeffects_INTENCLR(NRF_EGU_Type * p_reg);

#ifdef __cplusplus
}
#endif

#endif

