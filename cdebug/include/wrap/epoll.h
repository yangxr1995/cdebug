#ifdef __cplusplus
extern "C" {
#endif

#ifdef WRAP_DEFINE

#include <sys/epoll.h>

wrap_define(int, epoll_create, int size)
{
    int ret = __real_epoll_create(size);
    log_wrap_lib_info("%d = epoll_create(%d)", ret, size);
    return ret;
}

wrap_define(int, epoll_create1, int flags)
{
    int ret = __real_epoll_create1(flags);
    log_wrap_lib_info("%d = epoll_create1(%d)", ret, flags);
    return ret;
}

wrap_define(int, epoll_ctl, int epfd, int op, int fd, struct epoll_event * event)
{
    int ret = __real_epoll_ctl(epfd, op, fd, event);
    const char *op_str = "unknown";
    if (op == EPOLL_CTL_ADD)
        op_str = "EPOLL_CTL_ADD";
    else if (op == EPOLL_CTL_DEL)
        op_str = "EPOLL_CTL_DEL";
    else if (op == EPOLL_CTL_MOD)
        op_str = "EPOLL_CTL_MOD";

       // struct epoll_event {
       //     uint32_t      events;  /* Epoll events */
       //     epoll_data_t  data;    /* User data variable */
       // };
       //
       // union epoll_data {
       //     void     *ptr;
       //     int       fd;
       //     uint32_t  u32;
       //     uint64_t  u64;
       // };
    char event_str[1024] = {0};
    char event_data[32] = {0};
    sprintf(event_data, "data(%p)", event->data.ptr);
    strcat(event_str, "event(");
    if (event->events & EPOLLIN)
        strcat(event_str, "EPOLLIN|");
    if (event->events & EPOLLOUT)
        strcat(event_str, "EPOLLOUT|");
    if (event->events & EPOLLRDHUP)
        strcat(event_str, "EPOLLRDHUP|");
    if (event->events & EPOLLPRI)
        strcat(event_str, "EPOLLPRI|");
    if (event->events & EPOLLERR)
        strcat(event_str, "EPOLLERR|");
    if (event->events & EPOLLHUP)
        strcat(event_str, "EPOLLHUP|");
    if (event->events & EPOLLET)
        strcat(event_str, "EPOLLET|");
    if (event->events & EPOLLONESHOT)
        strcat(event_str, "EPOLLONESHOT|");
    if (event->events & EPOLLWAKEUP)
        strcat(event_str, "EPOLLWAKEUP|");
    if (event->events & EPOLLEXCLUSIVE)
        strcat(event_str, "EPOLLEXCLUSIVE");
    strcat(event_str, "), ");
    strcat(event_str, event_data);
    log_wrap_lib_info("%d = epoll_ctl(%d, %s(%d), %d, %s)", ret, epfd, op_str, op, fd, event_str);
    return ret;
}

#include <alloca.h>

__attribute__((__no_instrument_function__)) \
inline static const char *parse_event(struct epoll_event *event)
{
    if (event->events & EPOLLIN)
        return "EPOLLIN";
    if (event->events & EPOLLOUT)
        return "EPOLLOUT";
    if (event->events & EPOLLRDHUP)
        return "EPOLLRDHUP";
    if (event->events & EPOLLPRI)
        return "EPOLLPRI";
    if (event->events & EPOLLERR)
        return "EPOLLERR";
    if (event->events & EPOLLHUP)
        return "EPOLLHUP";
    if (event->events & EPOLLET)
        return "EPOLLET";
    if (event->events & EPOLLONESHOT)
        return "EPOLLONESHOT";
    if (event->events & EPOLLWAKEUP)
        return "EPOLLWAKEUP";
    if (event->events & EPOLLEXCLUSIVE)
        return "EPOLLEXCLUSIVE";
}

__attribute__((__no_instrument_function__)) \
static void parse_events(struct epoll_event *evs, int nb, char *out)
{
    char item[32];
    out[0] = 0;
    for (int i = 0; i < nb; ++i) {
        sprintf(item, "%d[%s] ", i, parse_event(evs + i));
        strcat(out, item);
    }
}

wrap_define(int, epoll_wait, int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    log_wrap_lib_info("call epoll_wait(epfd(%d), events(%p), maxevents(%d), timeout(%d))",
            epfd, events, maxevents, timeout);

    char *events_str = "no event";
    int ret = __real_epoll_wait(epfd, events, maxevents, timeout);
    if (ret > 0) {
        events_str = (char *)alloca(15 * ret + 15);
        parse_events(events, ret, events_str);
    }
    log_wrap_lib_info("%d =  epoll_wait(epfd(%d) : events: %s",
            ret, epfd, events_str);

    return ret;
}

wrap_define(int, epoll_pwait, int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t * sigmask)
{
    log_wrap_lib_info("call epoll_pwait(epfd(%d), events(%p), maxevents(%d), timeout(%d), sigmask(%p)",
            epfd, events, maxevents, timeout, sigmask);
    int ret = __real_epoll_pwait(epfd, events, maxevents, timeout, sigmask);
    log_wrap_lib_info("%d = epoll_pwait(epfd(%d), events(%p), maxevents(%d), timeout(%d), sigmask(%p)",
            ret, epfd, events, maxevents, timeout, sigmask);
    return ret;
}

// wrap_define(int, epoll_pwait2, int epfd, struct epoll_event *events, int maxevents, const struct timespec * timeout, const sigset_t * sigmask)
// {
//     log_wrap_lib_info("call epoll_pwait2(epfd(%d), events(%p), maxevents(%d), timeout(%p), sigmask(%p)",
//             epfd, events, maxevents, timeout, sigmask);
//     int ret = __real_epoll_pwait2(epfd, events, maxevents, timeout, sigmask);
//     log_wrap_lib_info("%d = epoll_pwait2(epfd(%d), events(%p), maxevents(%d), timeout(%p), sigmask(%p)",
//             ret, epfd, events, maxevents, timeout, sigmask);
//     return ret;
// }

#else

#define epoll_create(size)                  __epoll_create(size)
#define epoll_create1(flags)                __epoll_create1(flags)
#define epoll_ctl(epfd, op, fd, event)      __epoll_ctl(epfd, op, fd, event)
#define epoll_wait(epfd, events, maxevents, timeout)                __epoll_wait(epfd, events, maxevents, timeout)
#define epoll_pwait(epfd, events, maxevents, timeout, sigmask)      __epoll_pwait(epfd, events, maxevents, timeout, sigmask)
// #define epoll_pwait2(epfd, events, maxevents, timeout, sigmask)     __epoll_pwait2(epfd, events, maxevents, timeout, sigmask)

#endif

#ifdef __cplusplus
}
#endif
