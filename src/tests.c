#include <stdio.h>
#include "util.h"
#include "minunit.h"

/* define your test cases below */

/* util.c */

static char * test_xfree()
{
  void *ptr = NULL;
  printf("testing xfree\n"); // must be shown in case of segfault
  xfree(ptr);
  return NULL;
}

static char * test_strclone()
{
  char *original = "original string";
  char *clone    = strclone(original);
  mu_assert_eq("strclone", strcmp(original, clone));
  mu_assert("strclone: NULL", strclone(NULL) == NULL);
  return NULL;
}

/* cleaning.c */
#include "cleaning.h"

static char * test_utf8_strip()
{
  mu_assert_eq("utf8_strip: ASCII", strcmp(utf8_strip("aB5!><-"), "aB5!><-"));
  mu_assert_eq("utf8_strip: åäö",   strcmp(utf8_strip("[dåäöb]"), "[daaob]"));
  mu_assert_eq("utf8_strip: other", strcmp(utf8_strip("dªﬁæπb"), "d~~~~~b"));
  return NULL;
}

/* splitting.c */
#include "splitting.h"

static char * test_strsplit()
{
  int total_words = 0;
  char **words = strsplit("  kim  is splitting     words!  ", &total_words);

  mu_assert("strsplit: detects correct number of words", total_words == 4);
  mu_assert_eq("strsplit: words[0]", strcmp(words[0], "kim"));
  mu_assert_eq("strsplit: words[1]", strcmp(words[1], "is"));
  mu_assert_eq("strsplit: words[2]", strcmp(words[2], "splitting"));
  mu_assert_eq("strsplit: words[3]", strcmp(words[3], "words!"));
  return NULL;
}

/* formatting */

/* no smudge below this line! */

static char * run_all_tests()
{
  mu_run_test(test_xfree);
  mu_run_test(test_strclone);
  mu_run_test(test_utf8_strip);
  mu_run_test(test_strsplit);
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
    show("[PASS] %d assertions, %d tests.", mu_assertion_count, mu_test_count);
  }
}
