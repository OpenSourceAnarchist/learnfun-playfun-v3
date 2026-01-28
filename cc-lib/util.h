
#ifndef __UTIL_H
#define __UTIL_H

#include <cstdlib>
#include <map>
#include <cstdio>
#include <string>
#include <vector>

using namespace std;

#ifdef WIN32
#   define DIRSEP  "\\"
#   define DIRSEPC '\\'
#else
#   define DIRSEP  "/"
#   define DIRSEPC '/'
#endif

#define UTIL_PI 3.141592653589f

auto itos(int i) -> string;
auto stoi(string s) -> int;
auto dtos(double d) -> string;

struct Util {

  static auto ReadFile(const string &f) -> string;
  static auto WriteFile(const string &f, const string &s) -> bool;

  // Reads the lines in the file to the vector. Ignores all
  // carriage returns, including ones not followed by newline.
  static auto ReadFileToLines(const string &f) -> vector<string>;

  static auto SplitToLines(const string &s) -> vector<string>;

  // As above, but treat the first token on each line as a map
  // key. Ignores empty lines.
  static auto ReadFileToMap(const string &f) -> map<string, string>;

  static auto ReadFileBytes(const string &f) -> vector<unsigned char>;
  static auto WriteFileBytes(const string &f, const vector<unsigned char> &b) -> bool;

  static auto ListFiles(const string &dir) -> vector<string>;

  // XXX terrible names
  static auto shout(int, string, unsigned int &) -> int;
  static auto shint(int b, int i) -> string;
  /* converts int to byte string that represents it */
  static auto sizes(int i) -> string;

  /* only read if the file begins with the magic string */
  static auto hasmagic(string, const string &magic) -> bool;
  static auto readfilemagic(string, const string &magic) -> string;


  static auto ptos(void *) -> string;
  static auto hash(string s) -> unsigned int;
  /* give /home/tom/ of /home/tom/.bashrc */
  static auto pathof(string s) -> string;
  static auto fileof(string s) -> string;

  static auto ensureext(string f, string ext) -> string;
  static auto lcase(string in) -> string;
  static auto ucase(string in) -> string;

  static auto ExistsFile(string) -> bool;

  /* dirplus("/usr/local", "core") and
     dirplus("/usr/local/", core")  both give  "/usr/local/core"
     dirplus("/usr/local", "/etc/passwd")  gives "/etc/passwd"  */
  static auto dirplus(const string &dir, const string &file) -> string;

  /* spec is a character spec like "A-Z0-9`,."
     XXX document */
  static auto matchspec(string spec, char c) -> bool;

  /* An ordering on strings that gives a more "natural" sort:
     Tutorial 1, ..., Tutorial 9, Tutorial 10, Tutorial 11, ...
     rather than
     Tutorial 1, Tutorial 10, Tutorial 11, ..., Tutorial 2, Tutorial 20, ...
  */
  static auto natural_compare(const string & l, const string & r) -> int;

  /* Same as above, but ignore 'the' at the beginning */
  static auto library_compare(const string & l, const string & r) -> int;

  /* Is string s alphabetized under char k? */
  static auto library_matches(char k, const string & s) -> bool;

  /* open a new file. if it exists, return 0 */
  static auto open_new(string s) -> FILE *;
  /* 0 on failure */
  static auto changedir(string s) -> int;
  static auto random() -> int;
  /* random in 0.0 .. 1.0 */
  static auto randfrac() -> float;
  static auto getpid() -> int;
  /* anything ending with \n. ignores \r.
     modifies str. */
  static auto getline(string & str) -> string;
  /* same, for open file. */
  static auto fgetline(FILE * f) -> string;

  /* chop the first token (ignoring whitespace) off
     of line, modifying line. */
  static auto chop(string & line) -> string;

  /* number of entries (not . or ..) in dir d */
  static auto dirsize(string d) -> int;

  /* mylevels/good_tricky   to
     mylevels               to
     . */
  static auto cdup(const string & dir) -> string;

  /* true iff big ends with small */
  static auto endswith(string big_, string small_) -> bool;
  /* starts */
  static auto startswith(string big_, string small_) -> bool;

  /* split the string up to the first
     occurrence of character c. The character
     is deleted from both the returned string and
     the line */
  static auto chopto(char c, string & line) -> string;

  /* erase any whitespace up to the first
     non-whitespace char. */
  static auto losewhitel(const string & s) -> string;

  /* try to remove the file. If it
     doesn't exist or is successfully
     removed, then return true. */
  static auto remove(string f) -> bool;

  /* move a file from src to dst. Return
     true on success. */
  static auto move(string src, string dst) -> bool;

  /* make a copy by reading/writing */
  static auto copy(string src, string dst) -> bool;

  static auto tempfile(string suffix) -> string;

  /* does this file exist and is it a directory? */
  static auto isdir(string s) -> bool;

  /* same as isdir */
  static auto existsdir(string) -> bool;

  static auto makedir(const string &s) -> bool;

  /* try to launch the url with the default browser;
     doesn't work on all platforms. true on success */
  static auto launchurl(const string &) -> bool;

  /* creates directories for f */
  static void createpathfor(string f);

  /* open, creating directories if necessary */
  static auto fopenp(string f, string mode) -> FILE *;

  /* replace all occurrences of 'findme' with 'replacewith' in 'src' */
  static auto replace(string src, string findme, string replacewith) -> string;

  /* called minimum, maximum because some includes
     define these with macros, ugh */
  static auto minimum(int a, int b) -> int {
    if (a < b) return a;
    else return b;
  }

  static auto maximum(int a, int b) -> int {
    if (a > b) return a;
    else return b;
  }

};

/* drawing lines with Bresenham's algorithm */
struct line {
  static auto create(int x0, int y0, int x1, int y1) -> line *;
  virtual void destroy() = 0;
  virtual auto next(int & x, int & y) -> bool = 0;
  virtual ~line() = default;
};

/* union find structure, n.b. union is a keyword */
struct onionfind {
  int * arr;

  auto find(int) -> int;
  void onion(int,int);

  onionfind(int);
  ~onionfind () { delete [] arr; }

  private:

  onionfind(const onionfind &) { abort (); }
};

/* treats strings as buffers of bits */
struct bitbuffer {
  /* read n bits from the string s from bit offset idx.
     (high bits first)
     write that to output and return true.
     if an error occurs (such as going beyond the end of the string),
     then return false, perhaps destroying idx and output */
  static auto nbits(string s, int n, int & idx, unsigned int & output) -> bool;

  /* create a new empty bit buffer */
   bitbuffer() = default;

  /* appends bits to the bit buffer */
  void writebits(int width, unsigned int thebits);

  /* get the contents of the buffer as a string,
     padded at the end if necessary */
  auto getstring() -> string;

  /* give the number of bytes needed to store n bits */
  static auto ceil(int bits) -> int;


  ~bitbuffer() { free(data); }

  private:
   unsigned char * data{nullptr};
  int size{0};
  int bits{0};
};

#endif
