#ifdef __cplusplus
extern "C" {
#endif

#ifndef __WRAP_UTILS_H__
#define __WRAP_UTILS_H__

extern int gtrace_on;

#define log_wrap_lib_info(format, ...) do { \
   if ((gtrace_on) == 0) { \
        char buf[1024] = {0}, *call_sym; \
        void *call; \
        confirm_addr_info(__builtin_return_address(0) - sizeof(void *), &call, &call_sym); \
        snprintf(buf, sizeof(buf) - 1, "%s::::%p::::" format "\n", call_sym, call, ## __VA_ARGS__); \
        if (print_filter_ctx == NULL) { \
            if (print_filter_keyaddr_nb > 0) { \
                print_filter_ctx_init(); \
            } \
        } \
        if (print_filter_ctx) { \
            print_filter_run(0, buf); \
        } \
        else { \
            log_append(buf); \
        } \
    } \
} while (0)

#define wrap_define(_ret, _name, ...) \
_ret __real_##_name(__VA_ARGS__); \
__attribute__((__no_instrument_function__)) \
_ret __wrap_##_name(__VA_ARGS__)


#endif

#ifdef __cplusplus
}
#endif
