#ifdef __cplusplus
extern "C" {
#endif

#ifdef WRAP_DEFINE

#include <pthread.h>
#include <semaphore.h>

typedef struct thread_info_s {
    void *(*func)(void *);
    void *arg;
    print_filter_t *filter_ctx;
    sem_t sem;
} thread_info_t;

__attribute__((__no_instrument_function__))
static void *default_start_routine(void *arg)
{
    thread_info_t *info = (thread_info_t *)arg;
    void *(*start_func)(void *) = info->func;
    void *user_arg = info->arg;
    if (info->filter_ctx) {
        print_filter_ctx = malloc(sizeof(*print_filter_ctx));
        memcpy(print_filter_ctx, info->filter_ctx, sizeof(print_filter_t));
    }
    sem_post(&info->sem);
    return start_func(user_arg);
}

wrap_define(int, pthread_create, pthread_t *thread, const pthread_attr_t *attr,
                  void *(*start_routine) (void *), void *arg)
{
    char *this_sym; 
    void *this; 
    int ret;

    thread_info_t info = {
        .func = start_routine,
        .arg = arg,
        .filter_ctx = print_filter_ctx,
    };

    sem_init(&info.sem, 0, 0);
    ret = __real_pthread_create(thread, attr, default_start_routine, &info);
    sem_wait(&info.sem);
    confirm_addr_info((void *)start_routine, &this, &this_sym); 
    log_wrap_lib_info("pthread_create([%s]%p tid[%lu]", this_sym, this, *thread);

    return ret;
}

// int pthread_mutex_lock(pthread_mutex_t *mutex);
wrap_define(int, pthread_mutex_lock, pthread_mutex_t *mutex)
{
    int ret;
    ret = __real_pthread_mutex_lock(mutex);
    log_wrap_lib_info("pthread_mutex_lock(%p) = %d", mutex, ret);
    return ret;
}

// int pthread_mutex_trylock(pthread_mutex_t *mutex);
wrap_define(int, pthread_mutex_trylock, pthread_mutex_t *mutex)
{
    int ret;
    ret = __real_pthread_mutex_trylock(mutex);
    log_wrap_lib_info("pthread_mutex_trylock(%p) = %d", mutex, ret);
    return ret;
}

// int pthread_mutex_unlock(pthread_mutex_t *mutex);
wrap_define(int, pthread_mutex_unlock, pthread_mutex_t *mutex)
{
    int ret;
    ret = __real_pthread_mutex_unlock(mutex);
    log_wrap_lib_info("pthread_mutex_unlock(%p) = %d", mutex, ret);
    return ret;
}

#else

// 添加下面函数的 define
#define pthread_mutex_lock(mutex)           __real_pthread_mutex_lock(mutex)
#define pthread_mutex_trylock(mutex)        __real_pthread_mutex_trylock(mutex)
#define pthread_mutex_unlock(mutex)         __real_pthread_mutex_unlock(mutex)
#define pthread_create(thread, attr, start_routine, arg)   __real_pthread_create(thread, attr, start_routine, arg)

#endif

#ifdef __cplusplus
}
#endif
