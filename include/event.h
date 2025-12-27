#ifndef EVENT_H
#define EVENT_H

#include <sys/epoll.h>

int client_event(int efd, struct epoll_event* ev);

#endif