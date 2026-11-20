#pragma once

/* Task stack sizes (bytes) and priorities */
#define TASK_DISP_STACK     8192
#define TASK_DISP_PRIO      4

#define TASK_SCALE_STACK    4096
#define TASK_SCALE_PRIO     5

#define TASK_NET_STACK      6144
#define TASK_NET_PRIO       3

void task_disp(void *arg);
void task_scale(void *arg);
void task_net(void *arg);
