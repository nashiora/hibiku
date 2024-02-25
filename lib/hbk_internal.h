#ifndef HBK_API_H
#define HBK_API_H

#include <hibiku.h>

#define HBK_ASSERT(condition, message) \
    do {                               \
    } while (0)
#define HBK_TODO(message) HBK_ASSERT(false, "TODO: " message)
#define HBK_UNREACHABLE   HBK_ASSERT(false, "unreachable")

#define ANSI_COLOR_RESET             "\x1b[0m"
#define ANSI_COLOR_BLACK             "\x1b[30m"
#define ANSI_COLOR_RED               "\x1b[31m"
#define ANSI_COLOR_GREEN             "\x1b[32m"
#define ANSI_COLOR_YELLOW            "\x1b[33m"
#define ANSI_COLOR_BLUE              "\x1b[34m"
#define ANSI_COLOR_MAGENTA           "\x1b[35m"
#define ANSI_COLOR_CYAN              "\x1b[36m"
#define ANSI_COLOR_WHITE             "\x1b[37m"
#define ANSI_COLOR_BRIGHT_BLACK      "\x1b[30;1m"
#define ANSI_COLOR_BRIGHT_RED        "\x1b[31;1m"
#define ANSI_COLOR_BRIGHT_GREEN      "\x1b[32;1m"
#define ANSI_COLOR_BRIGHT_YELLOW     "\x1b[33;1m"
#define ANSI_COLOR_BRIGHT_BLUE       "\x1b[34;1m"
#define ANSI_COLOR_BRIGHT_MAGENTA    "\x1b[35;1m"
#define ANSI_COLOR_BRIGHT_CYAN       "\x1b[36;1m"
#define ANSI_COLOR_BRIGHT_WHITE      "\x1b[37;1m"
#define ANSI_BG_COLOR_BLACK          "\x1b[40m"
#define ANSI_BG_COLOR_RED            "\x1b[41m"
#define ANSI_BG_COLOR_GREEN          "\x1b[42m"
#define ANSI_BG_COLOR_YELLOW         "\x1b[43m"
#define ANSI_BG_COLOR_BLUE           "\x1b[44m"
#define ANSI_BG_COLOR_MAGENTA        "\x1b[45m"
#define ANSI_BG_COLOR_CYAN           "\x1b[46m"
#define ANSI_BG_COLOR_WHITE          "\x1b[47m"
#define ANSI_BG_COLOR_BRIGHT_BLACK   "\x1b[40;1m"
#define ANSI_BG_COLOR_BRIGHT_RED     "\x1b[41;1m"
#define ANSI_BG_COLOR_BRIGHT_GREEN   "\x1b[42;1m"
#define ANSI_BG_COLOR_BRIGHT_YELLOW  "\x1b[43;1m"
#define ANSI_BG_COLOR_BRIGHT_BLUE    "\x1b[44;1m"
#define ANSI_BG_COLOR_BRIGHT_MAGENTA "\x1b[45;1m"
#define ANSI_BG_COLOR_BRIGHT_CYAN    "\x1b[46;1m"
#define ANSI_BG_COLOR_BRIGHT_WHITE   "\x1b[47;1m"
#define ANSI_STYLE_BOLD              "\x1b[1m"
#define ANSI_STYLE_UNDERLINE         "\x1b[4m"
#define ANSI_STYLE_REVERSED          "\x1b[7m"

#define COLCAT(A, B) A##B
#define COL(X)       (use_color ? COLCAT(ANSI_COLOR_, X) : "")

typedef struct hbk_arena hbk_arena;

/// @brief "Intern"s the data of the given string. To intern a string,
/// the state creates a copy of it that it owns and returns to you a view.
/// This way, all instances of that string can point to the same memory.
/// An interned string is immutable.
hbk_string_view hbk_state_intern_string(hbk_state* state, hbk_string string);
/// @brief "Intern"s the data of the given string. To intern a string,
/// the state creates a copy of it that it owns and returns to you a view.
/// This way, all instances of that string can point to the same memory.
/// An interned string is immutable.
hbk_string_view hbk_state_intern_string_view(hbk_state* state, hbk_string_view sv);
/// @brief "Intern"s the data of the given string. To intern a string,
/// the state creates a copy of it that it owns and returns to you a view.
/// This way, all instances of that string can point to the same memory.
/// An interned string is immutable.
hbk_string_view hbk_state_intern_cstring(hbk_state* state, const char* string);

hbk_arena* hbk_arena_create();
void hbk_arena_destroy(hbk_arena* arena);
void* hbk_arena_alloc(hbk_arena* arena, size_t count);

#endif // !HBK_API_H
