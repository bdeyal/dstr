# DString — A Rich Dynamic String Library for C and C++

DString is a dynamic string library written in C with an optional C++
wrapper. The core is a pure C implementation offering a Python-inspired
string API, Small String Optimization (SSO), a non-owning view type,
and optional PCRE2 regular expression support. The C++ wrapper provides
RAII, operator overloading,`std::format` integration, and STL compatibility
while adding zero logic of its own — every operation delegates to the C
implementation.

---

## Design Philosophy

Most C string libraries offer a thin wrapper around `malloc` with a handful
of functions. DString takes a different approach: it aims for the richness
of Python's `str` type while remaining idiomatic C. Functions like `dstr_partition`,
`dstr_increment`, `dstr_translate`, `dstr_squeeze`, and `dstr_zfill` are rarely
seen in C string libraries but solve real problems without requiring a higher-level
language.

The library is designed with three principles:

- **Correct first** — SSO pointer aliasing, self-referential append, overlap
  detection, and carry propagation across mixed alphanumeric/punctuation
  strings are all handled correctly.
- **Complete API** — the C API is complete and self-contained.
  The C++ wrapper is purely a vocabulary layer with no string logic of its own.
- **Tested across compilers and platforms** — GCC, Clang, MSVC, clang-cl,
  and Borland on Linux, FreeBSD, and Windows (32-bit and 64-bit).

---

## Key Features

### Small String Optimization (SSO)

Short strings are stored directly inside the `DSTR` object with no heap
allocation. The transition between SSO and heap storage is transparent to
callers and handled correctly in all operations including self-referential
append, insert, and replace.

```c
DSTR s = dstr_create("hello");     // stored in SSO buffer, no malloc
dstr_append(s, " world");          // still in SSO buffer
dstr_append(s, ", this is a longer string that exceeds SSO");  // promoted to heap
dstr_destroy(s);                   // free only if heap was used
```

Stack allocation with no heap involvement at all:

```c
INIT_DSTR(s);                      // declares struct DSTR_TYPE s on the stack
dstr_assign(&s, "hello");          // uses SSO buffer
dstr_append(&s, " world");         // stays in SSO
DONE_DSTR(s);                      // frees heap only if SSO was exceeded
```

## C API Overview

All functions follow the naming convention `dstr_<operation>_<suffix>` where
the suffix indicates the type of the argument (`sz` = null-terminated C string,
`ds` = DSTR, `bl` = buffer+length, `cc` = char+count).

### Creation and Destruction

```c
DSTR dstr_create_sz(const char* sz);           // from C string
DSTR dstr_create_bl(const char* buf, size_t n);// from buffer + length
DSTR dstr_create_cc(char c, size_t count);     // fill with character
DSTR dstr_create_ds(CDSTR src);                // copy of another DSTR
DSTR dstr_create_substr(CDSTR p, size_t pos, size_t count);
DSTR dstr_create_sprintf(const char* fmt, ...);
DSTR dstr_create_fromfile(const char* fname);  // slurp entire file
void dstr_destroy(DSTR p);

// Stack allocation (no malloc for short strings):
INIT_DSTR(name);    // declares struct DSTR_TYPE name on the stack
DONE_DSTR(name);    // releases heap storage if SSO was exceeded
```

### Assign, Append, Insert, Replace

Every mutation operation is available in four forms:

```c
// Assign (replace content):
int dstr_assign_sz(DSTR dest, const char* sz);
int dstr_assign_ds(DSTR dest, CDSTR src);
int dstr_assign_bl(DSTR dest, const char* buf, size_t len);
int dstr_assign_sprintf(DSTR dest, const char* fmt, ...);

// Append:
int dstr_append_sz(DSTR dest, const char* sz);
int dstr_append_ds(DSTR dest, CDSTR src);
int dstr_append_bl(DSTR dest, const char* buf, size_t len);
int dstr_append_sprintf(DSTR dest, const char* fmt, ...);

// Insert at position:
int dstr_insert_sz(DSTR dest, size_t index, const char* value);
int dstr_insert_ds(DSTR dest, size_t index, CDSTR src);
int dstr_insert_bl(DSTR dest, size_t index, const char* buf, size_t len);

// Replace range:
int dstr_replace_sz(DSTR dest, size_t pos, size_t len, const char* value);
int dstr_replace_all_sz(DSTR dest, const char* old, const char* new, size_t count);
```

Overlap between source and destination is handled correctly in all cases —
appending a substring of a string to itself works as expected.

### Search

```c
// Forward search:
size_t dstr_find_sz(CDSTR p, size_t pos, const char* s);   // returns DSTR_NPOS if not found
size_t dstr_find_c(CDSTR p, size_t pos, char c);
size_t dstr_ifind_sz(CDSTR p, size_t pos, const char* s);  // case-insensitive

// Reverse search:
size_t dstr_rfind_sz(CDSTR p, size_t pos, const char* s);
size_t dstr_rfind_c(CDSTR p, size_t pos, char c);
size_t dstr_irfind_sz(CDSTR p, size_t pos, const char* s); // case-insensitive

// Find first/last of character set:
size_t dstr_ffo_sz(CDSTR p, size_t pos, const char* set);  // find first of
size_t dstr_ffno_sz(CDSTR p, size_t pos, const char* set); // find first not of
size_t dstr_flo_sz(CDSTR p, size_t pos, const char* set);  // find last of
size_t dstr_flno_sz(CDSTR p, size_t pos, const char* set); // find last not of

// Counting:
size_t dstr_count_sz(CDSTR p, const char* s);  // non-overlapping occurrences
size_t dstr_icount_sz(CDSTR p, const char* s); // case-insensitive
```

### Python-Inspired String Operations

These operations are rarely found in C string libraries but solve common
real-world problems:

#### `dstr_partition` / `dstr_rpartition`

Splits a string into three parts around a separator, like Python's
`str.partition()`:

```c
struct DSTR_PartInfo info;
dstr_partition(s, "://", &info);
// "https://example.com" -> left="https", mid="://", right="example.com"
```

#### `dstr_translate` / `dstr_squeeze` — Ruby/POSIX `tr`

Translates characters according to a mapping, supporting character ranges
including descending ranges (unlike POSIX `tr` and Ruby which reject them):

```c
// Standard forward range:
dstr_translate(s, "a-z", "A-Z");    // lowercase to uppercase

// Descending range (unique to DString — rejected by POSIX tr and Ruby):
dstr_translate(s, "Z-A", "a-z");    // Atbash cipher: Z→a, Y→b, ..., A→z

// Mixed ascending and descending in one pattern:
dstr_translate(s, "a-mz-n", "A-MZ-N");  // maps all 26 lowercase to uppercase
                                          // via two range segments

// Complement (translate everything NOT in set):
dstr_translate(s, "^aeiou", "-");   // replace all non-vowels with '-'

// Delete (NULL replacement):
dstr_translate(s, "aeiou", NULL);   // delete all vowels

// Squeeze consecutive duplicate characters:
dstr_squeeze(s, "a-z");             // "aaabbbccc" -> "abc"
dstr_translate_squeeze(s, "a-z", "A-Z");  // translate then squeeze
```

#### `dstr_increment` — Ruby-Style Successor Function

Increments a string by treating alphanumeric characters as a mixed-base counter
with carry propagation across punctuation boundaries:

```c
dstr_increment(dstr_create("az"))      // -> "ba"
dstr_increment(dstr_create("zz"))      // -> "aaa"
dstr_increment(dstr_create("ZZ"))      // -> "AAA"
dstr_increment(dstr_create("99"))      // -> "100"
dstr_increment(dstr_create("THX1138")) // -> "THX1139"
dstr_increment(dstr_create("ZZZ9999")) // -> "AAAA0000"

// Carry propagates across punctuation, matching type:
dstr_increment(dstr_create("hell!z99"))    // -> "helm!a00"
dstr_increment(dstr_create("abc-z99"))     // -> "abd-a00"

// Digit carry does not propagate through alpha and vice versa:
dstr_increment(dstr_create("a!99"))    // -> "a!100"  (digit carry, alpha left)
dstr_increment(dstr_create("1!zz"))    // -> "1!aaa"  (alpha carry, digit left)
```

#### `dstr_expand_tabs` / `dstr_zfill`

```c
dstr_expand_tabs(s, 8);             // expand tabs to 8-space tab stops
dstr_zfill(s, 10);                  // "42" -> "0000000042", "-5" -> "-000000005"
```

#### `dstr_align_left` / `dstr_align_right` / `dstr_align_center`

```c
dstr_align_right(s, 20, ' ');       // right-justify in 20 chars
dstr_align_center(s, 20, '*');      // center with '*' fill
dstr_align_left(s, 20, '-');        // left-justify with '-' fill
```

#### `dstr_multiply`

```c
dstr_multiply(s, 5);                // "ab" -> "ababababab"
```

### Case and Classification

```c
// Case conversion (ASCII only, SIMD-friendly):
void dstr_ascii_upper(DSTR p);
void dstr_ascii_lower(DSTR p);
void dstr_ascii_swapcase(DSTR p);
void dstr_title(DSTR p);            // "hello world" -> "Hello World"

// Classification (all characters must match):
bool dstr_isalpha(CDSTR p);
bool dstr_isdigits(CDSTR p);
bool dstr_isalnum(CDSTR p);
bool dstr_isupper(CDSTR p);
bool dstr_islower(CDSTR p);
bool dstr_isspace(CDSTR p);
bool dstr_isblank(CDSTR p);
bool dstr_isprintable(CDSTR p);
bool dstr_isidentifier(CDSTR p);    // valid C/Python identifier
bool dstr_istitle(CDSTR p);         // title case
bool dstr_isascii(CDSTR p);
bool dstr_isxdigits(CDSTR p);       // all hex digits
```

### Trim and Strip

```c
void dstr_trim(DSTR p);             // strip whitespace from both ends
void dstr_trim_left(DSTR p);
void dstr_trim_right(DSTR p);

// Strip specific character:
void dstr_lstrip_c(DSTR p, char c);
void dstr_rstrip_c(DSTR p, char c);

// Strip any character from a set:
void dstr_lstrip_sz(DSTR p, const char* set);
void dstr_rstrip_sz(DSTR p, const char* set);
```

### Prefix and Suffix

```c
bool dstr_prefix_sz(CDSTR p, const char* s);    // startswith
bool dstr_suffix_sz(CDSTR p, const char* s);    // endswith
bool dstr_iprefix_sz(CDSTR p, const char* s);   // case-insensitive
bool dstr_isuffix_sz(CDSTR p, const char* s);

void dstr_remove_prefix(DSTR p, const char* s); // strip prefix if present
void dstr_remove_suffix(DSTR p, const char* s); // strip suffix if present
void dstr_iremove_prefix(DSTR p, const char* s);
void dstr_iremove_suffix(DSTR p, const char* s);
```

### Numeric Conversion

The `dstr_atoi` / `dstr_atoll` functions support multiple bases with a
single call, automatically detecting the prefix:

```c
// Base auto-detection:
dstr_atoi(dstr_create("0b110111"))  // binary -> 55
dstr_atoi(dstr_create("0xABCD"))    // hex    -> 43981
dstr_atoi(dstr_create("\\177"))     // octal  -> 127
dstr_atoi(dstr_create("007"))       // decimal -> 7 (NOT octal)

// Integer to string (all bases 2-36):
dstr_itos(dest, -9223372036854775808LL);    // LLONG_MIN handled correctly
dstr_itos_ul(dest, 255, 16);               // "ff"
dstr_itos_ul(dest, 255, 2);                // "11111111"

// Full conversion suite with error reporting:
int   dstr_to_int(CDSTR p, size_t* index, int base);
long  dstr_to_long(CDSTR p, size_t* index, int base);
long long dstr_to_llong(CDSTR p, size_t* index, int base);
double    dstr_to_double(CDSTR p, size_t* index);
```

### Hashing

Uses xxHash (xxh32/xxh64 depending on platform) for fast, high-quality hashing.
The default seed is stable across runs for reproducible behavior. Call
`DString::randomize_hash_seed()` to enable per-process randomization for
hash table security:

```c
size_t h = dstr_hash(s, 0);    // 0 = use managed seed
size_t h = dstr_hash(s, 42);   // explicit fixed seed for reproducibility
```

### I/O

```c
int dstr_fgetline(DSTR p, FILE* fp);   // read one line (no newline in result)
int dstr_fgets(DSTR p, FILE* fp);      // read one whitespace-delimited token
int dstr_slurp_stream(DSTR p, FILE* fp); // read entire stream
DSTR dstr_create_fromfile(const char* fname); // slurp entire file
```

### Regular Expressions (optional, requires PCRE2)

Build without regex: `#define NO_DSTRING_REGEX`

```c
// Exact match (entire string):
bool dstr_regex_exact(CDSTR subject, const char* pattern, size_t offset);

// Match anywhere in string (returns offset or DSTR_NPOS):
size_t dstr_regex_contains(CDSTR subject, const char* pattern, size_t offset);

// Single match with position and length:
DSTR_Regex_Match match;
int rc = dstr_regex_match(subject, pattern, 0, &match, "i");
// match.offset, match.length

// Named capture groups:
DSTR_Match_Vector groups;
dstr_regex_match_groups(subject, "(?<year>\\d{4})-(?<month>\\d{2})", 0, &groups, NULL);
// groups.matches[1].name = "year", groups.matches[2].name = "month"
dstr_regex_mvector_free(&groups);

// Substitution (global with 'g' flag):
dstr_regex_substitute(s, "\\bfoo\\b", 0, "bar", "g");

// Options string (like regex flags in scripting languages):
// i=case insensitive, m=multiline, s=dotall, g=global, x=extended
// A=anchored, U=ungreedy, D=dollar end only
```

The regex engine caches up to 40 compiled patterns per thread using an LRU
strategy, so repeated use of the same pattern incurs no recompilation overhead.

---

## Short Names

The library provides short aliases for the most commonly used functions.
These are defined in `dstr.h` unless `DSTR_CLEAN_NAMESPACE` is defined.
For the complete list see `dstr.h` and `test/test_dstr.c`.

### Creation and Destruction

| Short name | Full name | Description |
|------------|-----------|-------------|
| `dstrnew(sz)` | `dstr_create_sz` | Create from C string |
| `dstrnew_empty()` | `dstr_create_empty` | Create empty string |
| `dstrnew_cc(c, n)` | `dstr_create_cc` | Create from char × count |
| `dstrnew_bl(buf, n)` | `dstr_create_bl` | Create from buffer + length |
| `dstrnew_ds(ds)` | `dstr_create_ds` | Copy of another DSTR |
| `dstrnew_slurp(fname)` | `dstr_create_fromfile` | Slurp entire file |
| `dstrnew_sprintf(fmt, ...)` | `dstr_create_sprintf` | Create via printf format |
| `dstrfree(p)` | `dstr_destroy` | Destroy and free |
| `dcleandata(p)` | `dstr_clean_data` | Release heap (stack-allocated DSTR) |

### Access

| Short name | Full name | Description |
|------------|-----------|-------------|
| `dstrdata(p)` | `dstr_cstr` | `const char*` pointer |
| `dstrlen(p)` | `dstr_length` | Length in bytes |
| `dstrcap(p)` | `dstr_capacity` | Capacity in bytes |
| `dstrempty(p)` | `dstr_isempty` | True if empty |
| `dstrfront(p)` | `dstr_front` | First character |
| `dstrback(p)` | `dstr_back` | Last character |
| `dstrgetc(p, i)` | `dstr_getchar` | Character at index |
| `dstrputc(p, i, c)` | `dstr_putchar` | Set character at index |
| `dstrgetc_s(p, i)` | `dstr_getchar_safe` | Character at signed index (-1 = last) |

### Assign and Append

| Short name | Full name | Description |
|------------|-----------|-------------|
| `dstrcpy(dest, sz)` | `dstr_assign_sz` | Assign from C string |
| `dstrcpy_ds(dest, src)` | `dstr_assign_ds` | Assign from DSTR |
| `dstrcpy_bl(dest, buf, n)` | `dstr_assign_bl` | Assign from buffer + length |
| `dsprintf(dest, fmt, ...)` | `dstr_assign_sprintf` | Assign via printf format |
| `dstrcat(dest, sz)` | `dstr_append_sz` | Append C string |
| `dstrcat_ds(dest, src)` | `dstr_append_ds` | Append DSTR |
| `dstrcat_c(dest, c)` | `dstr_append_char` | Append single character |
| `dstrcat_bl(dest, buf, n)` | `dstr_append_bl` | Append buffer + length |
| `dstrcat_sprintf(dest, fmt, ...)` | `dstr_append_sprintf` | Append via printf format |

### Search and Test

| Short name | Full name | Description |
|------------|-----------|-------------|
| `dstrstr(p, pos, sz)` | `dstr_find_sz` | Find substring (returns index or NPOS) |
| `dstrchr(p, pos, c)` | `dstr_find_c` | Find character |
| `dstrrstr(p, pos, sz)` | `dstr_rfind_sz` | Reverse find substring |
| `dstrrchr(p, pos, c)` | `dstr_rfind_c` | Reverse find character |
| `dstrstr_i(p, pos, sz)` | `dstr_ifind_sz` | Case-insensitive find |
| `dstrhas(p, sz)` | `dstr_contains_sz` | True if substring found |
| `dstartswith(p, sz)` | `dstr_prefix_sz` | True if starts with |
| `dendswith(p, sz)` | `dstr_suffix_sz` | True if ends with |
| `dstrcount(p, sz)` | `dstr_count_sz` | Count non-overlapping occurrences |

### Modify

| Short name | Full name | Description |
|------------|-----------|-------------|
| `dstrclear(p)` | `dstr_clear` | Set to empty (keep allocation) |
| `dstrtrim(p)` | `dstr_trim` | Strip whitespace both ends |
| `dstrtrim_l(p)` | `dstr_trim_left` | Strip leading whitespace |
| `dstrtrim_r(p)` | `dstr_trim_right` | Strip trailing whitespace |
| `dstrupper(p)` | `dstr_ascii_upper` | ASCII uppercase in place |
| `dstrlower(p)` | `dstr_ascii_lower` | ASCII lowercase in place |
| `dstrrev(p)` | `dstr_reverse` | Reverse in place |
| `dstrchop(p)` | `dstr_chop` | Remove last character |
| `dstrerase(p, pos, n)` | `dstr_remove` | Remove n bytes at pos |
| `drmchar(p, c)` | `dstr_remove_char` | Remove all occurrences of char |
| `dreplaceall(dest, old, new, n)` | `dstr_replace_all_sz` | Replace occurrences |
| `dstrmult(p, n)` | `dstr_multiply` | Repeat string n times |
| `dstrtrans(p, from, to)` | `dstr_translate` | Translate character ranges |
| `dstrsqz(p, set)` | `dstr_squeeze` | Squeeze consecutive duplicates |
| `dstrinc(p)` | `dstr_increment` | Successor (Ruby-style) |

### Comparison

| Short name | Full name | Description |
|------------|-----------|-------------|
| `dstrcmp(p, sz)` | `dstr_compare_sz` | strcmp-style comparison |
| `dstricmp(p, sz)` | `dstr_icompare_sz` | Case-insensitive comparison |
| `dstreq(p, sz)` | `dstr_equal_sz` | True if equal |
| `dstreq_i(p, sz)` | `dstr_iequal_sz` | Case-insensitive equal |
| `dstreq_ds(p, q)` | `dstr_equal_ds` | True if two DSTRs equal |

### Conversion

| Short name | Full name | Description |
|------------|-----------|-------------|
| `datoi(p)` | `dstr_atoi` | String to long (multi-base) |
| `datoll(p)` | `dstr_atoll` | String to long long (multi-base) |
| `ditos(dest, n)` | `dstr_itos` | Long long to string |
| `dstrtoi(p, idx, base)` | `dstr_to_int` | Convert with error reporting |
| `dstrtod(p, idx)` | `dstr_to_double` | String to double |
| `dstrhash(p, seed)` | `dstr_hash` | xxHash of string |

### Stack Allocation Macros

```c
INIT_DSTR(name)     // declare struct DSTR_TYPE name on the stack, initialized
DONE_DSTR(name)     // release heap storage if SSO capacity was exceeded
```

Example:
```c
INIT_DSTR(buf);
dstrcpy(&buf, "hello");
dstrcat(&buf, " world");
printf("%s\n", dstrdata(&buf));
DONE_DSTR(buf);
```
---

## C++ Wrapper

The C++ wrapper in `dstring.hpp` provides:

- `DString` — owning string with RAII, move semantics, operator overloading
- `DStringView` — non-owning view (analogous to `std::string_view`)
- Full STL container interface: `begin`/`end`, `cbegin`/`cend`,
  `rbegin`/`rend`, `size`, `empty`, `push_back`, `pop_back`
- `std::hash` specialization for use in `std::unordered_map`
- `std::formatter` specialization for `std::format` and `std::print` (C++20)
- `operator<=>` (C++20) with fallback comparison operators (C++17)
- `std::ranges` compatibility via `enable_borrowed_range` for `DStringView`
- Implicit conversion to `std::string_view` (C++17)
- `DString::format()` — equivalent to `std::format` returning `DString`

```cpp
DString s = "hello";
s += " world";
s.upper_inplace();                          // "HELLO WORLD"

DString r = s.replace_all("WORLD", "C++"); // returns new string
s.tr("A-Z", "a-z");                        // in-place translate

// C++20 format integration:
DString msg = DString::format("Hello {}, you are {} years old", name, age);

// Works with std::unordered_map directly:
std::unordered_map<DString, int> map;
map["key"] = 42;

// Range-for:
for (char c : s) { /* ... */ }

// Regex (C++ wrapper):
DString::MatchVector groups;
s.match_groups("(\\w+)@(\\w+)", groups);
for (const auto& m : groups)
    std::cout << s.substr(m.offset, m.length) << "\n";
```

The `inplace` / non-`inplace` naming convention provides both mutation and
functional styles consistently:

```cpp
s.upper_inplace();          // mutates s
DString u = s.upper();      // returns new string, s unchanged
s.trim_inplace();
DString t = s.trim();
```

### Examples
More examples of C++ usage can be seen in test file test_dstring.cpp

---

## Building

### Linux

```bash
make                        # default: gcc, C++20, -O2 -march=x86-64-v3
make COMP=clang             # use clang/clang++
make COMP=gcc               # use gcc/g++ explicitly
make SANITIZE=1             # enable AddressSanitizer
make test                   # build and run all tests
make testvg                 # run tests under valgrind
make install
```

### FreeBSD

```bash
gmake                       # BSD make not supported, use gmake
gmake COMP=gcc              # Default in FreeBSD is clang
```

### Windows (MSVC / clang-cl)

```cmd
nmake -f NMakefile              # default: cl.exe, C++20
nmake -f NMakefile COMP=clang   # use clang-cl
nmake -f NMakefile test
```

### Windows (Borland)

```cmd
nmake -f NMakefile COMP=borland          # 64-bit
nmake -f NMakefile COMP=borland PLAT=x86 # 32-bit
```

### Cross-compilation (MinGW)

```bash
make COMP=mingw64           # cross-compile for Windows 64-bit
make COMP=mingw32           # cross-compile for Windows 32-bit
```

---

## Tested Compilers and Platforms

| Compiler | Platform | C | C++ | Regex |
|----------|----------|---|-----|-------|
| GCC | Linux | ✓ | ✓ | ✓ |
| Clang | Linux | ✓ | ✓ | ✓ |
| TCC | Linux | ✓ | — | — |
| GCC | FreeBSD 15 | ✓ | ✓ | ✓ |
| Clang | FreeBSD 15 | ✓ | ✓ | ✓ |
| MSVC (VS 2022) | Windows 11 | ✓ | ✓ | ✓ |
| clang-cl | Windows 11 | ✓ | ✓ | ✓ |
| Borland bcc64x | Windows 11 | ✓ | ✓ | ✓ |
| Borland bcc32x | Windows 11 | ✓ | ✓ | ✓ |
| MinGW64 (cross) | Linux → Win11 | ✓ | ✓ | ✓ |
| MinGW32 (cross) | Linux → Win11 | ✓ | ✓ | ✓ |

---

## Dependencies

| Component | Dependency | Optional |
|-----------|-----------|----------|
| Core C library | None | — |
| C++ wrapper | C++11 or later | — |
| Regex | PCRE2 | Yes (`NO_DSTRING_REGEX`) |
| Hash | xxHash (bundled) | — |

xxHash is bundled as a single header (`deps/xxhash.h`) and requires no
separate installation.

---

## License

Copyright © 2025 Eyal Ben-David.
Distributed under the GNU GPL v3.0.
See `LICENSE` for the full license text.

The regex implementation (`dstr_regex.c`) is partially based on the POCO C++
Libraries, Copyright © 2006-2023 Applied Informatics Software Engineering GmbH,
licensed under the Boost Software License 1.0.
