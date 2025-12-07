#include "syntax.h"

#include "term.h"
#include "core.h"
#include "color.h"
#include "utils.h"
#include "textbuffer.h"
#include "render.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum HL_Flags{
    HL_HIGHLIGHT_STRINGS = (1 << 0),
    HL_HIGHLIGHT_NUMBERS = (1 << 1)
};

#define GROUP_TYPE_NORMAL 0
#define GROUP_TYPE_FUNCTION 1
#define GROUP_TYPE_FORCE 2

typedef struct SyntaxGroup
{
    char **keywords;
    Color color;
    uint8_t type;
} SyntaxGroup;

char *c_extensions[] = {".c", ".h", NULL};
char *c_keywords1[] = {
    "auto", "break", "case", "continue", "default", "do", "else", "enum",
    "extern", "for", "goto", "if", "register", "return", "sizeof", "static",
    "struct", "switch", "typedef", "union", "volatile", "while", NULL
};
char *c_keywords2[] = {
    "int", "long", "double", "float", "char", "unsigned", "signed",
    "void", "short", "auto", "const", "bool", "size_t", "ssize_t", 
    "off_t", "wchar_t", "ptrdiff_t", "sig_atomic_t", "fpos_t", 
    "clock_t", "time_t", "va_list", "FILE", "uint8_t", "uint16_t",
    "uint64_t", "int8_t", "int16_t", "int64_t", NULL
};
char *c_keywords3[] = {
    "#if", "#elif", "#else", "#endif", "#ifdef", "#ifndef", "#elifdef",
    "#elifndef", "#define", "#undef", "#include", "#embed", "#line",
    "#error", "#warning", "#pragma", NULL
};
char *c_keywords4[] = {
    "true", "false", "NULL", "EXIT_SUCCESS", "EXIT_FAILURE", NULL
};
char *c_stdfunctions[] = {
    "abort", "abs", "acos", "acosh", "alignas", "aligned_alloc", 
    "alignof", "arc4random", "asctime", "asctime_r", "asin", "asinh", 
    "assert", "at_quick_exit", "atan", "atan2", "atanh", "atexit", 
    "atof", "atoi", "atol", "atoll", "atomic_compare_exchange_strong", 
    "atomic_compare_exchange_weak", "atomic_exchange", "atomic_fetch_add", 
    "atomic_fetch_and", "atomic_fetch_or", "atomic_fetch_sub", 
    "atomic_fetch_xor", "atomic_flag_clear", "atomic_flag_test_and_set", 
    "atomic_init", "atomic_is_lock_free", "atomic_load", 
    "atomic_signal_fence", "atomic_store", "atomic_thread_fence", 
    "auto", "bool", "break", "bsearch", "c16rtomb", "c32rtomb", 
    "c8rtomb", "cabs", "cacos", "cacosh", "call_once", "calloc", 
    "carg", "case", "casin", "casinh", "catan", "catanh", "cbrt", 
    "ccos", "ccosh", "ceil", "cexp", "char", "cimag", "ckd_add", 
    "ckd_mul", "ckd_sub", "clearerr", "clock", "clog", "cnd_broadcast", 
    "cnd_destroy", "cnd_init", "cnd_signal", "cnd_timedwait", "cnd_wait", 
    "conj", "const", "constexpr", "continue", "copysign", "cos", "cosh", 
    "cpow", "cproj", "creal", "csin", "csinh", "csqrt", "ctan", "ctanh", 
    "ctime", "ctime_r", "default", "difftime", "div", "do", "double", 
    "else", "enum", "exit", "exp", "exp2", "expm1", "extern", "fabs", 
    "false", "fclose", "fdim", "feclearexcept", "fegetenv", "fegetexceptflag", 
    "fegetround", "feholdexcept", "feof", "feraiseexcept", "ferror", "fesetenv", 
    "fesetexceptflag", "fesetround", "fetestexcept", "feupdateenv", "fflush", 
    "fgetc", "fgetpos", "fgets", "fgetwc", "fgetws", "float", "floor", "fma",
    "fmax", "fmin", "fmod", "fopen", "fpclassify", "fprintf", "fputc", "fputs",
    "fputwc", "fputws", "fread", "free", "freopen", "frexp", "fscanf", "fseek",
    "fsetpos", "ftell", "fwprintf", "fwrite", "fwscanf", "getc", "getchar", 
    "gets", "gets_s", "getwc", "getwchar", "gmtime", "gmtime_r", "goto", "hypot",
    "if", "ilogb", "imaxabs", "imaxdiv", "inline", "int", "isalnum", "isalpha",
    "isblank", "iscntrl", "isdigit", "isfinite", "isgraph", "isinf", "islower",
    "isnan", "isnormal", "isprint", "ispunct", "isspace", "isupper", "iswalnum",
    "iswalpha", "iswblank", "iswcntrl", "iswctype", "iswdigit", "iswgraph", 
    "iswlower", "iswprint", "iswpunct", "iswspace", "iswupper", "iswxdigit", 
    "isxdigit", "kill_dependency", "labs", "ldexp", "ldiv", "llabs", "lldiv", 
    "llround", "localeconv", "localtime", "localtime_r", "log", "log10", "log1p",
    "log2", "logb", "long", "longjmp", "lround", "malloc", "mblen", "mbrtoc16",
    "mbrtoc32", "mbrtoc8", "mbstowcs", "mbtowc", "memchr", "memcmp", "memcpy",
    "memmove", "memset", "memset_explicit", "mktime", "modf", "mtx_destroy",
    "mtx_init", "mtx_lock", "mtx_timedlock", "mtx_trylock", "mtx_unlock", "nan",
    "nearbyint", "nextafter", "nexttoward", "nullptr", "perror", "pow", "printf",
    "putc", "putchar", "puts", "putwc", "putwchar", "qsort", "quick_exit", "raise",
    "rand", "realloc", "register", "remainder", "remove", "remquo", "rename",
    "restrict", "return", "rewind", "round", "scalbln", "scalbn", "scanf", "setbuf",
    "setjmp", "setlocale", "setvbuf", "short", "signbit", "signal", "signed", "sin",
    "sinh", "sizeof", "snprintf", "sprintf", "sqrt", "srand", "sscanf", "static",
    "static_assert", "stdc_bit_ceil", "stdc_bit_floor", "stdc_bit_width",
    "stdc_count_ones", "stdc_count_zeros", "stdc_first_leading_one",
    "stdc_first_leading_zero", "stdc_first_trailing_one", "stdc_first_trailing_zero",
    "stdc_has_single_bit", "stdc_leading_ones", "stdc_leading_zeros", "stdc_trailing_ones",
    "stdc_trailing_zeros", "strcat", "strchr", "strcmp", "strcoll", "strcpy", "strcspn",
    "strdup", "strerror", "strftime", "strlen", "strncat", "strncmp", "strncpy", 
    "strndup", "strpbrk", "strrchr", "strspn", "strstr", "strtod", "strtof", "strtoimax",
    "strtok", "strtol", "strtold", "strtoll", "strtoul", "strtoull", "strtoumax", 
    "struct", "strxfrm", "switch", "swprintf", "swscanf", "system", "tan", "tanh",
    "thrd_create", "thrd_current", "thrd_detach", "thrd_equal", "thrd_exit", "thrd_join",
    "thrd_sleep", "thrd_yield", "thread_local", "time", "timegm", "timespec_get",
    "timespec_getres", "tmpfile", "tmpnam", "tolower", "toupper", "towlower", "towupper",
    "true", "trunc", "tss_create", "tss_delete", "tss_get", "tss_set", "typedef", "typeof",
    "typeof_unqual", "ungetc", "ungetwc", "union", "unsigned", "va_arg", "va_copy",
    "va_end", "va_start", "vfprintf", "vfscanf", "vfwprintf", "vfwscanf", "void",
    "volatile", "vprintf", "vscanf", "vsnprintf", "vsprintf", "vsscanf", "vswprintf",
    "vswscanf", "vwprintf", "vwscanf", "wcscat", "wcschr", "wcscmp", "wcscoll", "wcscpy",
    "wcscspn", "wcsftime", "wcslen", "wcsncat", "wcsncmp", "wcsncpy", "wcspbrk", "wcsrchr",
    "wcsspn", "wcsstr", "wcstod", "wcstof", "wcstoimax", "wcstok", "wcstol", "wcstold",
    "wcstoll", "wcstombs", "wcstoul", "wcstoull", "wcstoumax", "wctomb", "wctype", "while",
    "wmemchr", "wmemcmp", "wmemcpy", "wmemmove", "wprintf", "wscanf", "_Alignas", "_Alignof",
    "_Atomic", "_Bool", "_Complex", "_Exit", "_Generic", "_Imaginary", "_Noreturn",
    "_Static_assert", "_Thread_local", NULL
};
SyntaxGroup c_group[] = {
{
    c_keywords1,
    HL_KEYWORD1,
    GROUP_TYPE_NORMAL
},
{
    c_keywords2,
    HL_KEYWORD2,
    GROUP_TYPE_NORMAL
},
{
    c_keywords3,
    HL_KEYWORD3,
    GROUP_TYPE_NORMAL
},
{
    c_keywords4,
    HL_KEYWORD4,
    GROUP_TYPE_NORMAL
},
{
    c_stdfunctions,
    HL_KEYWORD5,
    GROUP_TYPE_FUNCTION
}
};

char *rust_extensions[] = {".rs", ".rlib", NULL};
char *rust_keywords1[] = {
    "as", "break", "const", "continue", "crate", "else", "enum",
    "extern", "false", "for", "if", "impl", "in", "let",
    "loop", "match", "mod", "move", "mut", "pub", "unsafe",
    "where", "while", "async", "await", "dyn", "try", "gen",
    "abstract", "become", "box", "do", "final", "macro", "override",
    "piv", "typeof", "unsized", "virtual", "yield", NULL
};
char *rust_keywords2[] = {
    "bool", "char", "str", "i8", "i16", "i32", "i64", "i128", "isize",
    "u8", "u16", "u32", "u64", "u128", "usize", "f32", "f64", "Self", 
    "String", "Vec", "Option", "Result", "Box", "Rc", "Arc", "HashMap", NULL
};
char *rust_keywords3[] = {
    "println!", "use", "fn", NULL
};
char *rust_keywords4[] = {
    "true", "false", "None", "Some", NULL
};
SyntaxGroup rust_group[] = {
{
    rust_keywords1,
    HL_KEYWORD1,
    GROUP_TYPE_NORMAL
},
{
    rust_keywords2,
    HL_KEYWORD2,
    GROUP_TYPE_NORMAL
},
{
    rust_keywords3,
    HL_KEYWORD3,
    GROUP_TYPE_NORMAL
},
{
    rust_keywords4,
    HL_KEYWORD4,
    GROUP_TYPE_NORMAL
}
};

Syntax HLDB[] = {
{
    c_extensions,
    c_group,
    sizeof(c_group)/sizeof(SyntaxGroup),
    "//", 
    "/*", "*/",
    HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
},
{
    rust_extensions,
    rust_group,
    sizeof(rust_group)/sizeof(SyntaxGroup),
    "//", 
    "/*", "*/",
    HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_NUMBERS
}
};

typedef struct HighlightState
{
    Row *row;
    Syntax *syntax;
    const char *r_current;
    unsigned char *hl_current;
    int in_string;
    bool in_comment;
    bool prev_sep;
} HighlightState;

/* Return true if the specified row last char is part of a multi line comment
 * that starts at this row or at one before, and does not end at the end
 * of the row but spawns to the next row. */
static bool editorRowHasOpenComment(Row *row)
{
    if (row->render.hl && row->render.size && 
        row->render.hl[row->render.size - 1] == HL_MLCOMMENT &&
        (row->render.size < 2 || 
        row->render.c[row->render.size - 2] != '*' ||
        row->render.c[row->render.size - 1] != '/'))
    {
        return true;
    }

    return false;
}

static bool Highlight_Skip(HighlightState *s)
{
    if (*(s->hl_current) != HL_NORMAL)
    {
        s->r_current++;
        s->hl_current++;
        s->prev_sep = 1;
        return true;
    }
    return false;
}

static bool Highlight_SingleLineComment(HighlightState *s)
{
    char *start = s->syntax->singleline_comment_start;

    if (s->prev_sep && *s->r_current == start[0] && *(s->r_current + 1) == start[1])
    {
        memset(s->hl_current, HL_COMMENT, s->row->render.size - (s->hl_current-s->row->render.hl));
        return true;
    }
    return false;
}

static bool Highlight_MultiLineComment(HighlightState *s)
{
    char *start = s->syntax->multiline_comment_start;
    char *end = s->syntax->multiline_comment_end;

    if (s->in_comment)
    {
        *s->hl_current = HL_MLCOMMENT;
        if (*s->r_current == end[0] && *(s->r_current + 1) == end[1])
        {
            *(s->hl_current + 1) = HL_MLCOMMENT;
            s->r_current += 2;
            s->hl_current += 2;
            s->in_comment = 0;
            s->prev_sep = 1;
        }
        else
        {
            s->r_current++;
            s->hl_current++;
            s->prev_sep = 0;
        }
        return true;
    }
    else if (*s->r_current == start[0] && *(s->r_current + 1) == start[1])
    {
        *(s->hl_current) = HL_MLCOMMENT;
        *(s->hl_current + 1) = HL_MLCOMMENT;
        s->r_current += 2;
        s->hl_current += 2;
        s->in_comment = 1;
        s->prev_sep = 0;
        return true;
    }
    return false;
}

static bool Highlight_String(HighlightState *s)
{
    if (s->in_string)
    {
        *(s->hl_current) = HL_STRING;
        if (*s->r_current == '\\' && *(s->r_current + 1))
        {
            *(s->hl_current + 1) = HL_STRING;
            s->r_current += 2;
            s->hl_current += 2;
        }
        else
        {
            if (*s->r_current == s->in_string)
                s->in_string = 0;
            s->r_current++;
            s->hl_current++;
        }
        s->prev_sep = 0;
        return true;
    }
    else if (*s->r_current == '"' || *s->r_current == '\'')
    {
        s->in_string = *s->r_current;
        *(s->hl_current) = HL_STRING;
        s->r_current++;
        s->hl_current++;
        s->prev_sep = 0;
        return true;
    }
    return false;
}

static bool Highlight_Number(HighlightState *s)
{
    bool is_number = (isdigit(*s->r_current) && (s->prev_sep || *(s->hl_current - 1) == HL_NUMBER));
    bool is_float = (*s->r_current == '.' && s->hl_current > s->row->render.hl && *(s->hl_current - 1) == HL_NUMBER);

    if (is_number || is_float)
    {
        *(s->hl_current) = HL_NUMBER;
        s->r_current++;
        s->hl_current++;
        s->prev_sep = 0;
        return true;
    }
    return false;
}

static bool Highlight_Keywords(HighlightState *s)
{
    if (!s->prev_sep)
        return false;

    SyntaxGroup *groups = s->syntax->groups;
    for (size_t i = 0; i < s->syntax->group_num; i++)
    {
        char **keywords = groups[i].keywords;
        for (int j = 0; keywords[j]; j++)
        {
            int klen = strlen(keywords[j]);

            if (!memcmp(s->r_current, keywords[j], klen) && is_separator(*(s->r_current + klen)))
            {
                switch (groups[i].type)
                {
                case GROUP_TYPE_NORMAL:
                    if (!is_separator(*(s->r_current + klen))) continue;
                    break;
                case GROUP_TYPE_FUNCTION:
                    if (*(s->r_current + klen) != '(') continue;
                    break;
                case GROUP_TYPE_FORCE:
                    break;
                default:
                    continue;
                }

                memset(s->hl_current, groups[i].color, klen);
                s->r_current += klen;
                s->hl_current += klen;
                s->prev_sep = 0;
                return true;
            }
        }
    }
    return false;
}

void editorUpdateSyntax(TextBuffer *buf, int row_idx)
{
    Row *row = &buf->rows[row_idx];
    Syntax *syntax = buf->syntax;

    if (syntax == NULL)
        return;

    HighlightState s;
    s.row = row;
    s.syntax = syntax;
    s.r_current = row->render.c;
    s.hl_current = row->render.hl;
    s.in_string = 0;
    s.in_comment = (row->idx > 0 && editorRowHasOpenComment(&buf->rows[row->idx - 1]));
    s.prev_sep = 1;

    while (*s.r_current && isspace(*s.r_current))
    {
        s.r_current++;
        s.hl_current++;
    }

    while (*s.r_current)
    {
        if (Highlight_Skip(&s))               continue;
        if (Highlight_MultiLineComment(&s))   continue;
        if (Highlight_SingleLineComment(&s))  break;
        if (Highlight_String(&s))             continue;
        if (Highlight_Number(&s))             continue;
        if (Highlight_Keywords(&s))           continue;

        s.prev_sep = is_separator(*s.r_current);
        s.r_current++;
        s.hl_current++;
    }

    if (editorRowHasOpenComment(row) && row->idx + 1 < buf->numrows)
    {
        editorUpdateSyntax(buf, row->idx + 1);
    }
}

Style editorSyntaxToColor(unsigned char hl)
{
    switch (hl)
    {
    case HL_NORMAL:
        return STYLE_NORMAL;
    case HL_NONPRINT:
        return STYLE_INVERSE;
    case HL_COMMENT:
    case HL_MLCOMMENT:
        return (Style){COLOR_BRIGHT_BLACK, COLOR_BLACK, 0};
    case HL_KEYWORD1:
        return (Style){COLOR_YELLOW, COLOR_BLACK, 0};
    case HL_KEYWORD2:
        return (Style){COLOR_GREEN, COLOR_BLACK, 0};
    case HL_KEYWORD3:
        return (Style){COLOR_MAGENTA, COLOR_BLACK, 0};
    case HL_KEYWORD4:
        return (Style){COLOR_ORANGE, COLOR_BLACK, 0};
    case HL_KEYWORD5:
        return (Style){COLOR_CYAN, COLOR_BLACK, 0};
    case HL_STRING:
        return (Style){COLOR_GREEN, COLOR_BLACK, 0};
    case HL_NUMBER:
        return (Style){COLOR_RED, COLOR_BLACK, 0};
    case HL_MATCH:
        return (Style){COLOR_BLUE, COLOR_BLACK, ATTR_INVERSE};
    case HL_TAB:
        return (Style){COLOR_RED, COLOR_YELLOW, 0};
    default:
        return STYLE_NORMAL;
    }
}

static bool matchExtension(const char *filename, const char *extension)
{
    int flen = strlen(filename);
    int elen = strlen(extension);

    if (flen < elen)
        return false;

    for (int i = 0; i < elen; i++)
    {
        if (filename[flen - 1 - i] != extension[elen - 1 - i])
            return false;
    }

    return true;
}

void editorSelectSyntaxHighlight(TextBuffer *buf, const char *filename)
{
    for (size_t j = 0; j < sizeof(HLDB) / sizeof(HLDB[0]); j++)
    {
        Syntax *s = HLDB + j;
        for (size_t i = 0; s->filematch[i]; i++)
        {
            if (s->filematch[i][0] == '.')
            {
                if (matchExtension(filename, s->filematch[i]))
                {
                    buf->syntax = s;
                    return;
                }
            }
            else if (strcmp(filename, s->filematch[i]) == 0)
            {
                buf->syntax = s;
                return;
            }
        }
    }
}
