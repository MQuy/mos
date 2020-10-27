#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>

static struct lconv lc;

struct lconv* localeconv()
{
	lc.decimal_point = (char*)".";
	lc.thousands_sep = (char*)"";
	lc.grouping = (char*)"";
	lc.int_curr_symbol = (char*)"";
	lc.currency_symbol = (char*)"";
	lc.mon_decimal_point = (char*)"";
	lc.mon_thousands_sep = (char*)"";
	lc.mon_grouping = (char*)"";
	lc.positive_sign = (char*)"";
	lc.negative_sign = (char*)"";
	lc.int_frac_digits = 127;
	lc.frac_digits = 127;
	lc.p_cs_precedes = 127;
	lc.n_cs_precedes = 127;
	lc.p_sep_by_space = 127;
	lc.n_sep_by_space = 127;
	lc.p_sign_posn = 127;
	lc.n_sign_posn = 127;
	lc.int_p_cs_precedes = 127;
	lc.int_n_cs_precedes = 127;
	lc.int_p_sep_by_space = 127;
	lc.int_n_sep_by_space = 127;
	lc.int_p_sign_posn = 127;
	lc.int_n_sign_posn = 127;
	return &lc;
}

static char* current_locales[LC_NUM_CATEGORIES] = {NULL};

char* setlocale(int category, const char* locale)
{
	if (category < 0 || LC_ALL < category)
		return errno = EINVAL, (char*)NULL;
	char* new_strings[LC_NUM_CATEGORIES];
	int from = category != LC_ALL ? category : 0;
	int to = category != LC_ALL ? category : LC_NUM_CATEGORIES - 1;
	if (!locale)
		return current_locales[to] ? current_locales[to] : (char*)"C";
	for (int i = from; i <= to; i++)
	{
		if (!(new_strings[i] = strdup(locale)))
		{
			for (int n = from; n < i; n++)
				free(new_strings[n]);
			return (char*)NULL;
		}
	}
	for (int i = from; i <= to; i++)
	{
		free(current_locales[i]);
		current_locales[i] = new_strings[i];
	}
	return (char*)locale;
}
