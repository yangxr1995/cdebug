#ifdef __cplusplus
extern "C" {

#define this _this

#endif

#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <execinfo.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>


static inline void __attribute__((__no_instrument_function__))
print_filter_run(char is_find, char *newdata);

__attribute__((__no_instrument_function__))
inline static void 
print_filter_ctx_init();

#include "trace.h"

typedef struct map_s {
	char *name;
	unsigned long begin;
	unsigned long end;
} map_t;

typedef struct trace_ctx_s trace_ctx_t;
struct trace_ctx_s {
    pid_t pid;
    char name[128];
    char trace_on[256];

    map_t *text_maps;
    unsigned int text_map_max_num;
    unsigned int text_map_num;
};

static trace_ctx_t ctx;

__attribute__((__no_instrument_function__))
static void log_append(const char *str);

__attribute__((__no_instrument_function__))
inline static int
confirm_addr_info(void *orig_addr, void **paddr, char **psym);

// init -> kcached -> find -> flush -> kdirect -> cnt == 0 -> kcached
// kcached -> find 1 -> flush -> kdirect
// kcached -> find 0 -> kcached
// kdirect -> find 1 -> cnt reset -> kdirect
// kdirect -> find 0 -> cnt-- -> cnt == 0 -> kcached
typedef enum print_filter_work_mode {
    kcached,
    kdirect,
} print_filter_work_mode_t;

#define QUEUE_SIZE 10
#define QUEUE_ITEM_SIZE 256

typedef struct print_filter {
    print_filter_work_mode_t mode;
    char queue_buffer[QUEUE_SIZE][QUEUE_ITEM_SIZE];
    int begin;
    int end;
    int trace_count;
} print_filter_t;

print_filter_t __thread *print_filter_ctx;

char gfilter_filename[256];

#define PRINT_FILTER_TRACE_COUNT 10
#define PRINT_FILTER_KEYWORDS_MAX_NB 1024

void *print_filter_keyaddr[PRINT_FILTER_KEYWORDS_MAX_NB];
int print_filter_keyaddr_nb;

__attribute__((__no_instrument_function__))
inline static void 
print_filter_keyaddr_print() {
    printf("%s %d\n", __func__, __LINE__);
    for (int i = 0; i < print_filter_keyaddr_nb; ++i) {
        printf("%p\n", print_filter_keyaddr[i]);
    }
}

#ifdef WRAP_TURN_ON

#include "wrap/utils.h"

#define WRAP_DEFINE
#include "wrap/lib.h"

#ifdef WRAP_DEFINE
#undef WRAP_DEFINE
#endif

#define WRAP_REPLACE
#include "wrap/lib.h"

#ifdef WRAP_REPLACE
#undef WRAP_REPLACE
#endif

#endif

int gtrace_on;

// wrap_define(pid_t, fork)
pid_t __real_fork();
__attribute__((__no_instrument_function__))
pid_t __wrap_fork()
{
    int ret;
    // if (access(ctx.trace_on, F_OK) == 0) {
    if (gtrace_on) {
        char buf[256] = {0}, *call_sym;
        void *call;
        confirm_addr_info(__builtin_return_address(0) - sizeof(void *), &call, &call_sym);
        snprintf(buf, sizeof(buf) - 1, "%s::::%p::::" "fork()" "\n", call_sym, call);
        if (print_filter_ctx) {
            print_filter_run(0, buf);
        }
        else {
            log_append(buf);
        }
    }
    if ((ret = __real_fork()) == 0)
        ctx.pid = getpid();
    return ret;
}

__attribute__((__no_instrument_function__))
static void
log_append(const char *str)
{
    int fd;
    char filename[256];
    pthread_t tid = pthread_self();
    snprintf(filename, sizeof(filename), "/var/run/%s-%d-%ld.log", ctx.name, ctx.pid, tid);
    if ((fd = open(filename, O_APPEND | O_CREAT | O_WRONLY, 0644)) == 0) {
        perror("log_append : open");
        exit(1);
    }
    write(fd, str, strlen(str));
    close(fd);
}

inline static
__attribute__((__no_instrument_function__))
void *convert_to_elf(void *addr) 
{
    return addr;
    Dl_info info;
    int result = dladdr(addr, &info);
    if (result == 0) {
        return NULL; // 解析失败
    }
    uintptr_t offset = (uintptr_t)addr - (uintptr_t)info.dli_fbase;
    return (void *)offset;
}

__attribute__((__no_instrument_function__))
inline static int
confirm_addr_info(void *orig_addr, void **paddr, char **psym)
{
    unsigned int i;
    void *addr;
    char *sym;
    char is_find = 0;

	for (i = 0; i < ctx.text_map_num; i++) {

		if (ctx.text_maps[i].begin < (unsigned long)orig_addr &&	
				ctx.text_maps[i].end > (unsigned long)orig_addr) {

            is_find = 1;
			sym = ctx.text_maps[i].name;
            addr = convert_to_elf(orig_addr);
            break;
		}

    }

    if (!is_find) {
        printf("WARNING:can't find %p\n", orig_addr);
        return 1;
    }

    *paddr = addr;
    *psym = sym;
    return 0;
}

static inline char __attribute__((__no_instrument_function__))
print_filter_keyaddr_check(void *target)
{
    for (int i = 0; i < print_filter_keyaddr_nb; ++i) {
        if (print_filter_keyaddr[i] == target)
            return 1;
    }
    return 0;
}

#define QUEUE_INDEX_WALK(__index) \
do { \
    ++(__index); \
    (__index) = (__index) % QUEUE_SIZE; \
} while(0)

static inline void __attribute__((__no_instrument_function__))
print_filter_flush()
{
    while (print_filter_ctx->begin != print_filter_ctx->end) {
        log_append(print_filter_ctx->queue_buffer[print_filter_ctx->begin]);
        print_filter_ctx->queue_buffer[print_filter_ctx->begin][0] = 0;
        QUEUE_INDEX_WALK(print_filter_ctx->begin);
    }
}

static inline void __attribute__((__no_instrument_function__))
print_filter_cache(char *data)
{
    strncpy(print_filter_ctx->queue_buffer[print_filter_ctx->end], data, QUEUE_ITEM_SIZE - 1);
    QUEUE_INDEX_WALK(print_filter_ctx->end);
    if (print_filter_ctx->begin == print_filter_ctx->end) {
        QUEUE_INDEX_WALK(print_filter_ctx->begin);
    }
}

static inline void __attribute__((__no_instrument_function__))
print_filter_run(char is_find, char *newdata)
{
    // printf("%s %d : find[%d] mode[%d] trace_count[%d] \n", 
    //         __func__, __LINE__, is_find, print_filter_ctx->mode, print_filter_ctx->trace_count);
    if (print_filter_ctx->mode == kcached) {
        if (is_find) {
            print_filter_flush();
            print_filter_ctx->mode = kdirect;
            print_filter_ctx->trace_count = PRINT_FILTER_TRACE_COUNT;
            log_append(newdata);
        }
        else {
            print_filter_cache(newdata);
        }

    }
    else if (print_filter_ctx->mode == kdirect) {
        log_append(newdata);
        if (is_find) {
            print_filter_ctx->trace_count = PRINT_FILTER_TRACE_COUNT;
        }
        else {
            print_filter_ctx->trace_count--;
            if (print_filter_ctx->trace_count <= 0)
                print_filter_ctx->mode = kcached;
        }
    }
}

static inline void __attribute__((__no_instrument_function__))
__trace_running(const char *msg, void *this, void *call)
{
	char *this_sym = NULL, *call_sym = NULL;

    confirm_addr_info(this, &this, &this_sym);
    confirm_addr_info(call, &call, &call_sym);

    char data[256];
    snprintf(data, sizeof(data), "%s\n%s:%p\n%s:%p\n", msg, call_sym, call, this_sym, this);

    if (print_filter_ctx == NULL) {
        if (print_filter_keyaddr_nb > 0) {
            // 通过c++11 thread启动的线程,
            // 因为无法通过hook pthread_create 初始化 print_filter_ctx,
            // 只能在此处完成初始化
            print_filter_ctx_init();
        }
    }

    // check filter
    if (print_filter_ctx) {
        int is_find = print_filter_keyaddr_check(this) || print_filter_keyaddr_check(call);
        print_filter_run(is_find, data);
    }
    else {
        log_append(data);
    }
}

static inline void __attribute__((__no_instrument_function__))
trace_running(const char *msg, void *this, void *call)
{
    if (access(ctx.trace_on, F_OK) == 0) {
        gtrace_on = 1;
		__trace_running(msg, this, call);
    }
}

#include <dlfcn.h>
#include <stdbool.h>

#ifdef __cplusplus
__attribute__((__no_instrument_function__))
inline static bool is_skip(void *this)
{
    Dl_info info;
    dladdr(this, &info);
    if (info.dli_sname == NULL)
        return true;
    if (strstr(info.dli_sname, "St"))
        return true;
    if (strstr(info.dli_sname, "_ZnwmPv"))
        return true;
    if (strstr(info.dli_sname, "__cxx"))
        return true;
    if (strstr(info.dli_sname, "__gnu_"))
        return true;
    if (strstr(info.dli_sname, "_trait"))
        return true;
    /*printf("==%s\n", info.dli_sname);*/
    return false;
}
#endif

void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this, void *call)
{
#ifdef __cplusplus
    if (!is_skip(this))
#endif
        trace_running("Enter", this, call);
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this, void *call)
{
#ifdef __cplusplus
    if (!is_skip(this))
#endif
        trace_running("Exit", this, call);
}

static inline char  __attribute__((__no_instrument_function__))
is_text(const char *str)
{
	return strstr(str, "r-xp") != NULL ? 1 : 0;
}

static void __attribute__((__no_instrument_function__))
get_task_maps()
{
	map_t *ptext_map;
	char *ptr;
	char maps[256];	
	snprintf(maps, sizeof(maps), "/proc/%d/maps", ctx.pid);

	FILE *fp;

	fp = fopen(maps, "r");
	assert(fp != NULL && "can't open proc maps");
	char *line = NULL;
	size_t n = 0;

	while (getline(&line, &n, fp) > 0) {

		if (!is_text(line))
			goto __next__;
		
		if (ctx.text_map_max_num <= ctx.text_map_num) {
			ctx.text_map_max_num += 32;
			ctx.text_maps = (map_t *)realloc(ctx.text_maps, sizeof(*ctx.text_maps) * ctx.text_map_max_num);
		}

		for (ptr = line + strlen(line); *ptr != '/' && *ptr != ' '; ptr--)
			NULL;
		if (*ptr == ' ')
			goto __next__;

		ptext_map = ctx.text_maps + ctx.text_map_num;
		ctx.text_map_num++;
		ptext_map->name = strdup(ptr + 1);	
		ptext_map->name[strlen(ptext_map->name) - 1] = '\0';

		ptext_map->begin = strtoul(line, &ptr, 16);
		ptext_map->end = strtoul(ptr + 1, NULL, 16);

__next__:
		if (line) {
			free(line);
			line = NULL;
		}
	}

	fclose(fp);
}

static void  __attribute__((__no_instrument_function__))
get_prg_name(char *buf, size_t buf_sz, pid_t pid)
{
	char file[128];
	int fd, cnt;

	sprintf(file, "/proc/%d/comm", pid);
	fd = open(file, O_RDONLY);
	cnt = read(fd, buf, buf_sz - 1);
	close(fd);
	buf[cnt-1] = '\0';
}

__attribute__((__no_instrument_function__))
inline static void 
prg_info_init()
{
    ctx.pid = getpid();
    get_prg_name(ctx.name, sizeof(ctx.name), ctx.pid);
    snprintf(ctx.trace_on, sizeof(ctx.trace_on) - 1, "/var/run/trace_%s", ctx.name);
    get_task_maps();
}

__attribute__((__no_instrument_function__))
inline static void 
print_filter_keyaddr_push(void *addr)
{
    print_filter_keyaddr[print_filter_keyaddr_nb] = addr;
    ++print_filter_keyaddr_nb;
}

__attribute__((__no_instrument_function__))
inline static void 
print_filter_ctx_init()
{
    print_filter_ctx = NULL;
    if (print_filter_keyaddr_nb > 0) {
        print_filter_ctx = malloc(sizeof(*print_filter_ctx));
        memset(print_filter_ctx, 0x0, sizeof(*print_filter_ctx));
        print_filter_ctx->mode = kcached;
        print_filter_ctx->trace_count = 0;
    }
}

__attribute__((__no_instrument_function__))
inline static void 
print_filter_init()
{
    char filter_filename[256] = {0};
    char line[1024];

    snprintf(filter_filename, sizeof(filter_filename), "/var/run/filter_%s", ctx.name);
    printf("filter_filename : %s\n", filter_filename);
    strcpy(gfilter_filename, filter_filename);

    FILE *fp = fopen(filter_filename, "r");
    if (!fp) {
        return ;
    }

    unsigned long long addr_ll;
    uintptr_t addr;
    while (fgets(line, sizeof(line), fp)) {
        addr_ll = strtoll(line, NULL, 16);
        addr = (uintptr_t) addr_ll;
        print_filter_keyaddr_push((void *)addr);
    }

    fclose(fp);

    print_filter_keyaddr_print();

    print_filter_ctx_init();
}

__attribute__((constructor)) 
__attribute__((__no_instrument_function__))
inline static void 
init()
{
    prg_info_init();
    print_filter_init();
}

#ifdef __cplusplus
}
#endif
