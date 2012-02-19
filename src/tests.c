#include <stdio.h>
#include "helper.h"
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

  char *hashtag = "#Hashtaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaag";
  total_words = 0;
  words = strsplit(hashtag, &total_words);
  mu_assert("strsplit: single word, total", total_words == 1);
  mu_assert_eq("strsplit: single word, result", strcmp(words[0], hashtag));
  mu_assert("strsplit: single word, length", strlen(words[0]) == 140);

  return NULL;
}

/* formatting.c */
#include "formatting.h"

static char * test_justify()
{
	char *nothing[] = {"abc", "def"};
	mu_assert_eq("nothing", strcmp(
		justify(nothing, 2),
		"abc def"
	));
	
	char *long_word[] = {
		"some",
		"realllllllllllllllllllllllllllllllylooooooooooooooooooooooooooooooooooooongwooooooooooooooooooooooooooooooooooooordwordword"
	};
	mu_assert_eq("long word", strcmp(
		justify(long_word, 2),
		"some reallllllllllllllllllllllllllllllly\n"
		"looooooooooooooooooooooooooooooooooooong\n"
		"wooooooooooooooooooooooooooooooooooooord\n"
		"wordword"
	));
	
	char *move_one_down[] = {"moooooooooooooooooooooooooooooove", "one", "down"};
	mu_assert_eq("move one down", strcmp(
		justify(move_one_down, 3),
		"moooooooooooooooooooooooooooooove one \n"
		"down"
	));
	
	char *move_some_down[] = {
		"moooooooooooooooooooooooooooooove", "some", 
		"down", "but", "nooooooooooooot", "aaaaaaaaaaaaaaallll", "becauuuuuuuuuuuuuuuuuuse", "some", 
		"aaaaaree", "tooooooooooooooooooooooo", "long"
	};
	mu_assert_eq("move some down", strcmp(
		justify(move_some_down, 11),
		"moooooooooooooooooooooooooooooove some \n"
		"down but nooooooooooooot aaaaaaaaaaaaaaa\n"
		"llll becauuuuuuuuuuuuuuuuuuse some \n"
		"aaaaaree tooooooooooooooooooooooo long"
	));
	
	return NULL;
}

static char * test_add_date()
{
	char first_line[164] = "abc def 0123456789abc";
	add_date(first_line, "- 12/34 56:78");
	mu_assert_eq("first_line", strcmp(
		"abc def \n"
		"\n"
		"\n"
		"                           - 12/34 56:78",
		first_line)
	);
	
	char last_line[164] =
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
		"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n"
		"cccccccccccccccccccccccccccccccccccccccc\n"
		"dddddddddd 0123456789abc";
	add_date(last_line, "- 12/34 56:78");
	mu_assert_eq(
		"last_line",
		strcmp(
			"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n"
			"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n"
			"cccccccccccccccccccccccccccccccccccccccc\n"
			"dddddddddd                 - 12/34 56:78",
			last_line
		)
	);
	
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
  mu_run_test(test_justify);
  mu_run_test(test_add_date);
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
