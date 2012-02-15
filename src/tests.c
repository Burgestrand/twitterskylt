#include <stdio.h>
#include "util.h"
#include "minunit.h"
int tests_run = 0;

/* define your test cases below */

/* cleaning.c */
#include "cleaning.h"

static char * test_utf8_strip()
{
  mu_assert("utf8_strip: ASCII", 0 == strcmp(utf8_strip("aB5!><-"), "aB5!><-"));
  mu_assert("utf8_strip: åäö",   0 == strcmp(utf8_strip("[dåäöb]"), "[daaob]"));
  mu_assert("utf8_strip: other", 0 == strcmp(utf8_strip("dªﬁæπb"), "d~~~~~b"));
  return NULL;
}

/* formatting */

/* no smudge below this line! */

static char * run_all_tests()
{
  mu_run_test(test_utf8_strip);
  return NULL;
}

int main(void)
{
  char *result = run_all_tests();

  if (result != NULL)
  {
    show("[FAIL] %s", result);
  }
  else
  {
    show("[PASS] %d tests.", tests_run);
  }
}
