#include "luat_base.h"
#include "luat_rtos.h"
#include "luat_rtos_legacy.h"

LUAT_WEAK void luat_rtos_task_suspend_all(void)
{
	luat_task_suspend_all();
}
LUAT_WEAK void luat_rtos_task_resume_all(void)
{
	luat_task_resume_all();
}
LUAT_WEAK int luat_rtos_event_send(luat_rtos_task_handle task_handle, uint32_t id, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t timeout)
{
	return luat_send_event_to_task(task_handle, id, param1, param2, param3);
}
LUAT_WEAK int luat_rtos_event_recv(luat_rtos_task_handle task_handle, uint32_t wait_event_id, luat_event_t *out_event, luat_rtos_event_wait_callback_t *callback_fun, uint32_t timeout)
{
	return luat_wait_event_from_task(task_handle, wait_event_id, out_event, callback_fun, timeout);
}
LUAT_WEAK int luat_rtos_message_send(luat_rtos_task_handle task_handle, uint32_t message_id, void *p_message)
{
	return luat_send_event_to_task(task_handle, message_id, p_message, 0, 0);
}
LUAT_WEAK int luat_rtos_message_recv(luat_rtos_task_handle task_handle, uint32_t *message_id, void **p_p_message, uint32_t timeout)
{
	luat_event_t event;
	int result = luat_wait_event_from_task(task_handle, 0, &event, 0, timeout);
	if (!result)
	{
		*message_id = event.id;
		*p_p_message = (void *)event.param1;
	}
	return result;
}

LUAT_WEAK int luat_rtos_mutex_create(luat_rtos_mutex_t *mutex_handle)
{
	*mutex_handle = luat_mutex_create();
	return (*mutex_handle)?0:-1;
}

LUAT_WEAK int luat_rtos_mutex_unlock(luat_rtos_mutex_t mutex_handle)
{
	return luat_mutex_unlock(mutex_handle);
}

LUAT_WEAK int luat_rtos_timer_create(luat_rtos_timer_t *timer_handle)
{
	*timer_handle = luat_create_rtos_timer(NULL, NULL, NULL);
	return (*timer_handle)?0:-1;
}

LUAT_WEAK int luat_rtos_timer_stop(luat_rtos_timer_t timer_handle)
{
	luat_stop_rtos_timer(timer_handle);
	return 0;
}

LUAT_WEAK int luat_rtos_timer_delete(luat_rtos_timer_t timer_handle)
{
	luat_release_rtos_timer(timer_handle);
	return 0;
}
