#ifndef TRACE
# ifdef NDEBUG
#  define TRACE do { } while(0)
#  define TRACE_FMT(...) do { } while(0)
# else
#  define TRACE fprintf(stderr, "TRACE:%s():%d\n", __func__, __LINE__)
#  define TRACE_FMT(...) fprintf(stderr, "TRACE:" __VA_ARGS__)
# endif
#endif
