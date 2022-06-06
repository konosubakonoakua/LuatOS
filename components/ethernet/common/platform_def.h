

#ifndef ETHERNET_COMMON_PLATFORM_DEF_H_
#define ETHERNET_COMMON_PLATFORM_DEF_H_

#define platform_random	luat_crypto_trng
#define platform_send_event luat_send_event_to_task
#define platform_wait_event luat_wait_event_from_task
#define platform_create_task	luat_thread_start
#define platform_create_timer	luat_create_rtos_timer
#define platform_start_timer	luat_start_rtos_timer
#define platform_stop_timer	luat_stop_rtos_timer
#define platform_release_timer	luat_release_rtos_timer
#define malloc 	luat_heap_malloc
#define free 	luat_heap_free
#define msleep	luat_timer_mdelay

#define OS_LOCK	luat_task_suspend_all()
#define OS_UNLOCK luat_task_resume_all()
#endif /* ETHERNET_COMMON_PLATFORM_DEF_H_ */
