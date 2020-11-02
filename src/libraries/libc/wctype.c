#include <string.h>
#include <wctype.h>

int iswalnum(wint_t c)
{
	return iswalpha(c) || iswdigit(c);
}

int iswalpha(wint_t c)
{
	return iswupper(c) || iswlower(c);
}

int iswblank(wint_t c)
{
	return c == L' ' || c == L'\t';
}

int iswcntrl(wint_t c)
{
	return /*0 <= c [c is unsigned] && */ c < 32;
}

int iswctype(wint_t c, wctype_t wct)
{
	return wct(c);
}

int iswdigit(wint_t c)
{
	return L'0' <= c && c <= L'9';
}

int iswgraph(wint_t c)
{
	return L'!' <= c && c != 127;
}

int iswlower(wint_t c)
{
	return L'a' <= c && c <= L'z';
}

int iswprint(wint_t c)
{
	return iswgraph(c) || c == L' ';
}

int iswpunct(wint_t c)
{
	return iswprint(c) && c != L' ' && !iswalnum(c);
}

int iswspace(wint_t c)
{
	return c == L'\t' || c == L'\n' || c == L'\v' ||
		   c == L'\f' || c == L'\r' || c == L' ';
}

int iswupper(wint_t c)
{
	return L'A' <= c && c <= L'Z';
}

int iswxdigit(wint_t c)
{
	if (iswdigit(c))
		return 1;
	if (L'a' <= c && c <= L'f')
		return 1;
	if (L'A' <= c && c <= L'F')
		return 1;
	return 0;
}

wint_t towlower(wint_t c)
{
	if (L'A' <= c && c <= L'Z')
		return L'a' + c - L'A';
	return c;
}

wint_t towupper(wint_t c)
{
	if (L'a' <= c && c <= L'z')
		return L'A' + c - L'a';
	return c;
}

wctype_t wctype(const char *name)
{
	if (!strcmp(name, "alnum"))
		return (wctype_t)iswalnum;
	if (!strcmp(name, "alpha"))
		return (wctype_t)iswalpha;
	if (!strcmp(name, "blank"))
		return (wctype_t)iswblank;
	if (!strcmp(name, "cntrl"))
		return (wctype_t)iswcntrl;
	if (!strcmp(name, "digit"))
		return (wctype_t)iswdigit;
	if (!strcmp(name, "graph"))
		return (wctype_t)iswgraph;
	if (!strcmp(name, "lower"))
		return (wctype_t)iswlower;
	if (!strcmp(name, "print"))
		return (wctype_t)iswprint;
	if (!strcmp(name, "punct"))
		return (wctype_t)iswpunct;
	if (!strcmp(name, "space"))
		return (wctype_t)iswspace;
	if (!strcmp(name, "upper"))
		return (wctype_t)iswupper;
	if (!strcmp(name, "xdigit"))
		return (wctype_t)iswxdigit;
	return (wctype_t)NULL;
}
