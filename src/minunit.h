/* file: minunit.h */
/* source: http://www.jera.com/techinfo/jtns/jtn002.html */

#define mu_assert_eq(message, test) mu_assert(message, ! test)
#define mu_assert(message, test) do { \
    mu_assertion_count++;             \
    printf("[TEST] (%s:%d) %s… ", __FILE__, __LINE__, message); \
    if ( ! (test))                    \
    {                                 \
      printf("FAIL\n");               \
      return message;                 \
    }                                 \
    else                              \
    {                                 \
      printf("OK\n");                 \
      fflush(stdout);                 \
    }                                 \
  } while (0)

#define mu_run_test(test) do {   \
    char *message = test();      \
    mu_test_count++;             \
    if (message) return message; \
  } while (0)

int mu_assertion_count;
int mu_test_count;
