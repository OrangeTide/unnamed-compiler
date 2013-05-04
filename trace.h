#ifndef TRACE
# ifdef NDEBUG
#  define TRACE do { } while(0)
#  define TRACE_FMT(...) do { } while(0)
# else
#  define TRACE fprintf(stderr, "TRACE:%s()\n", __func__);
#  define TRACE_FMT(...) fprintf(stderr, __VA_ARGS__);
# endif
#endif