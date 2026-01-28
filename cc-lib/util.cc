
#include <sys/stat.h>
#include <array>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <algorithm>
#include <utility>

#include "util.h"

#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
   /* chdir */
#  include <direct.h>
   /* getpid */
#  include <process.h>
   /* time */
#  include <time.h>
   /* rename */
#  include <io.h>
   /* setclipboard */
#  include <windows.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
// This used to be included unconditionally in the header;
// not sure why? -tom7
#  include <dirent.h>
#endif

// Visual studio only.
#if defined(WIN32) && !defined(__MINGW32__) && !defined(__MINGW64__)
# pragma warning(disable: 4996)
#endif

#else /* posix */
   /* chdir, unlink */
#  include <unistd.h>
   /* getpid */
#  include <sys/types.h>
   /* isalnum */
#  include <cctype>
   /* directory stuff */
#  include <dirent.h>
#endif


auto itos(int i) -> string {
  std::array<char, 64> s{};
  std::snprintf(s.data(), s.size(), "%d", i);
  return s.data();
}

auto dtos(double d) -> string {
  std::array<char, 64> s{};
  std::snprintf(s.data(), s.size(), "%.2f", d);
  return s.data();
}

// TODO: I never tested this on posix.
auto Util::ListFiles(const string &s) -> vector<string> {
  vector<string> v;
  DIR *dir = opendir(s.c_str());
  if (dir == nullptr) return {};
  while (struct dirent *res = readdir(dir)) {
    string s = res->d_name;
    if (s != "." && s != "..") {
      v.push_back(std::move(s));
    }
  }
  closedir(dir);
  return v;
}

namespace {
struct linereal : public line {
  int x0, y0, x1, y1;
  int dx, dy;
  int stepx, stepy;
  int frac;

  ~linereal() override = default;

  linereal(int x0_, int y0_, int x1_, int y1_) :
    x0(x0_), y0(y0_), x1(x1_), y1(y1_) {


    dy = y1 - y0;
    dx = x1 - x0;

    if (dy < 0) {
      dy = -dy;
      stepy = -1;
    } else {
      stepy = 1;
    }

    if (dx < 0) {
      dx = -dx;
      stepx = -1;
    } else {
      stepx = 1;
    }

    dy <<= 1;
    dx <<= 1;

    if (dx > dy) {
      frac = dy - (dx >> 1);
    } else {
      frac = dx - (dy >> 1);
    }
  }

  auto next(int & cx, int & cy) -> bool override {
    if (dx > dy) {
      if (x0 == x1) return false;
      else {
	if (frac >= 0) {
	  y0 += stepy;
	  frac -= dx;
	}
	x0 += stepx;
	frac += dy;
	cx = x0;
	cy = y0;
	return true;
      }
    } else {
      if (y0 == y1) return false;
      else {
	if (frac >= 0) {
	  x0 += stepx;
	  frac -= dy;
	}
	y0 += stepy;
	frac += dx;
	cx = x0;
	cy = y0;
	return true;
      }
    }
  }

  void destroy() override {
    delete this;
  }

};

}  // namespace

auto line::create(int a, int b, int c, int d) -> line * {
  return new linereal(a, b, c, d);
}

auto Util::isdir(string f) -> bool {
  struct stat st;
  return (!stat(f.c_str(), &st)) && (st.st_mode & S_IFDIR);
}

auto Util::ExistsFile(string s) -> bool {
  struct stat st;

  return !stat(s.c_str(), &st);
}

auto Util::existsdir(string d) -> bool {
  return isdir(d); /* (ExistsFile(d) && isdir(d.c_str())); */
}

/* XXX what mode? */
auto Util::makedir(const string &d) -> bool {
# if defined(WIN32) || defined(__MINGW32__)
  return !mkdir(d.c_str());
# else /* posix */
  return !mkdir(d.c_str(), 0755);
# endif
}

auto Util::ptos(void * p) -> string {
  std::array<char, 64> s{};
  std::snprintf(s.data(), s.size(), "%p", p);
  return s.data();
}

auto Util::ReadFile(const string &s) -> string {
  if (Util::isdir(s) || s.empty()) return {};

  FILE * f = fopen(s.c_str(), "rb");
  if (!f) return {};
  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return {};
  }
  const long size_long = ftell(f);
  if (size_long < 0) {
    fclose(f);
    return {};
  }
  const auto size = static_cast<size_t>(size_long);
  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return {};
  }

  string ret(size, '\0');
  const size_t read = fread(ret.data(), 1, size, f);
  fclose(f);

  if (read != size) {
    ret.resize(read);
  }

  return ret;
}

auto Util::ReadFileToLines(const string &f) -> vector<string> {
  return SplitToLines(ReadFile(f));
}

auto Util::SplitToLines(const string &s) -> vector<string> {
  vector<string> v;
  string line;
  // PERF don't need to do so much copying.
  for (char i : s) {
    if (i == '\r')
      continue;
    else if (i == '\n') {
      v.push_back(line);
      line.clear();
    } else {
      line += i;
    }
  }
  return v;
}

auto Util::ReadFileToMap(const string &f) -> map<string, string> {
  map<string, string> m;
  vector<string> lines = ReadFileToLines(f);
  for (auto rest : lines) {
    string tok = chop(rest);
    rest = losewhitel(rest);
    m.insert(make_pair(tok, rest));
  }
  return m;
}

// PERF memcpy
auto Util::ReadFileBytes(const string &f) -> vector<unsigned char> {
  string s = ReadFile(f);
  vector<unsigned char> bytes;
  bytes.reserve(s.size());
  for (char i : s) {
    bytes.push_back((unsigned char)i);
  }
  return bytes;
}


static auto hasmagicf(FILE * f, const string & mag) -> bool {
  char * hdr = (char*)malloc(mag.length());
  if (!hdr) return false;

  /* we may not even be able to read sizeof(header) bytes! */
  if (mag.length() != fread(hdr, 1, mag.length(), f)) {
    free(hdr);
    return false;
  }

  for (unsigned int i = 0; i < mag.length(); i++) {
    if (hdr[i] != mag[i]) {
      free(hdr);
      return false;
    }
  }

  free(hdr);
  return true;
}

auto Util::hasmagic(string s, const string &mag) -> bool {
  FILE * f = fopen(s.c_str(), "rb");
  if (!f) return false;

  bool hm = hasmagicf(f, mag);

  fclose(f);
  return hm;
}

auto Util::readfilemagic(string s, const string &mag) -> string {
  if (isdir(s) || s.empty()) return {};

  // printf("opened %s\n", s.c_str());

  /* PERF try this: and see! */
  // printf("Readfile '%s'\n", s.c_str());

  FILE * f = fopen(s.c_str(), "rb");

  if (!f) return "";


  if (!hasmagicf(f, mag)) {
    fclose(f);
    return "";
  }

  /* ok, now just read file */

  if (fseek(f, 0, SEEK_END) != 0) {
    fclose(f);
    return {};
  }
  const long size_long = ftell(f);
  if (size_long < 0) {
    fclose(f);
    return {};
  }
  const auto size = static_cast<size_t>(size_long);
  if (fseek(f, 0, SEEK_SET) != 0) {
    fclose(f);
    return {};
  }

  string ret(size, '\0');
  const size_t read = fread(ret.data(), 1, size, f);
  fclose(f);

  if (read != size) {
    ret.resize(read);
  }

  return ret;
}

auto Util::WriteFile(const string &fn, const string &s) -> bool {

  FILE * f = fopen(fn.c_str(), "wb");
  if (!f) return false;

  /* XXX check failure */
  fwrite(s.c_str(), 1, s.length(), f);

  fclose(f);

  return true;
}

auto Util::WriteFileBytes(const string &fn,
			  const vector<unsigned char> &bytes) -> bool {
  FILE * f = fopen(fn.c_str(), "wb");
  if (!f) return false;

  /* XXX check failure */
  fwrite(&bytes[0], 1, bytes.size(), f);

  fclose(f);

  return true;
}

auto Util::sizes(int i) -> string {
  string s = "    ";
  s[0] = 255&(i >> 24);
  s[1] = 255&(i >> 16);
  s[2] = 255&(i >> 8);
  s[3] = 255& i;
  return s;
}

/* XXX these have terrible names */

/* represent int i (as i mod (2^(b/8)))
   using only b bytes */
auto Util::shint(int b, int i) -> string {
  return sizes(i).substr(4-b, b);
}

/* inverse of shint. does not check that
   there is enough room in s to read b bytes
   from idx ... */
auto Util::shout(int b, string s, unsigned int & idx) -> int {
  int r = 0;
  while (b--) {
    r = ((unsigned char)s[idx++]) + (r<<8);
  }
  return r;
}

auto Util::hash(string s) -> unsigned int {
  unsigned int h = 0x714FA5DD;
  for (unsigned int i = 0; i < s.length(); i ++) {
    h = (h << 11) | (h >> (32 - 11));
    h *= 3113;
    h ^= (unsigned char)s[i];
  }
  return h;
}

auto Util::lcase(string in) -> string {
  string out;
  for (unsigned int i = 0; i < in.length(); i++) {
    if (in[i] >= 'A' &&
	in[i] <= 'Z') out += in[i]|32;

    else out += in[i];
  }
  return out;
}

auto Util::ucase(string in) -> string {
  string out;
  for (unsigned int i = 0; i < in.length(); i++) {
    if (in[i] >= 'a' &&
	in[i] <= 'z') out += (in[i] & (~ 32));

    else out += in[i];
  }
  return out;
}

auto Util::fileof(string s) -> string {
  for (long long int i = s.length() - 1; i >= 0; i --) {
    if (s[i] == DIRSEPC) {
      return s.substr(i + 1, s.length() - (i + 1));
    }
  }
  return s;
}

auto Util::pathof(string s) -> string {
  if (s == "") return ".";
  for (long long int i = s.length() - 1; i >= 0; i --) {
    if (s[i] == DIRSEPC) {
      return s.substr(0, i);
    }
  }
  return ".";
}

/* XX can use endswith below */
auto Util::ensureext(string f, string ext) -> string {
  if (f.length() < ext.length())
    return f + ext;
  else {
    if (f.substr(f.length() - ext.length(),
		 ext.length()) != ext)
      return f + ext;
    else return f;
  }
}

auto Util::endswith (string big, string small_) -> bool {
  if (small_.length() > big.length()) return false;
  return big.substr(big.length() - small_.length(),
		    small_.length()) == small_;
}

auto Util::startswith (string big, string small_) -> bool {
  if (small_.length() > big.length()) return false;
  return big.starts_with(small_);
}

auto Util::changedir(string s) -> int {
  return !chdir(s.c_str());
}

auto Util::getpid() -> int {
  return ::getpid();
}

auto stoi(string s) -> int {
  return atoi(s.c_str());
}

/* XXX race. should use creat
   with O_EXCL on unix, at least. */
auto Util::open_new(string fname) -> FILE * {
  if (!ExistsFile(fname))
    return fopen(fname.c_str(), "wb+");
  else return nullptr;
}

auto Util::getline(string & chunk) -> string {
  string ret;
  for (unsigned int i = 0; i < chunk.length(); i ++) {
    if (chunk[i] == '\r') continue;
    else if (chunk[i] == '\n') {
      chunk = chunk.substr(i + 1, chunk.length() - (i + 1));
      return ret;
    } else ret += chunk[i];
  }
  /* there doesn't need to be a final trailing newline. */
  chunk = "";
  return ret;
}

/* PERF */
auto Util::fgetline(FILE * f) -> string {
  string out;
  int c;
  while ( (c = fgetc(f)), ((c != EOF) && (c != '\n')) ) {
    /* ignore CR */
    if (c != '\r') {
      out += (char)c;
    }
  }
  return out;
}

/* PERF use substr instead of accumulating: this is used
   frequently in the net stuff */
/* return first token in line, removing it from 'line' */
auto Util::chop(string & line) -> string {
  for (unsigned int i = 0; i < line.length(); i ++) {
    if (line[i] != ' ') {
      string acc;
      for (unsigned int j = i; j < line.length(); j ++) {
	if (line[j] == ' ') {
	  line = line.substr(j, line.length() - j);
	  return acc;
	} else acc += line[j];
      }
      line = "";
      return acc;
    }
  }
  /* all whitespace */
  line = "";
  return "";
}

/* PERF same */
auto Util::chopto(char c, string & line) -> string {
  string acc;
  for (unsigned int i = 0; i < line.length(); i ++) {
    if (line[i] != c) {
      acc += line[i];
    } else {
      if (i < (line.length() - 1)) {
	line = line.substr(i + 1, line.length() - (i + 1));
	return acc;
      } else {
	line = "";
	return acc;
      }
    }
  }
  /* character didn't appear; treat as an invisible
     occurrence at the end */
  line = "";
  return acc;
}

auto Util::losewhitel(const string & s) -> string {
  for (unsigned int i = 0; i < s.length(); i ++) {
    switch(s[i]) {
    case ' ':
    case '\n':
    case '\r':
    case '\t':
      /* keep going ... */
      break;
    default:
      return s.substr(i, s.length() - i);
    }
  }
  /* all whitespace */
  return "";
}

auto Util::tempfile(string suffix) -> string {
  static int tries = 0;

  string fname;
  do {
    fname = to_string(tries) + "_" + to_string(getpid()) + "_" +
            to_string(random()) + suffix;
    ++tries;
  } while (ExistsFile(fname));

  return fname;
}

/* break up the strings into tokens. A token is either
   a single character (non-numeral) or a sequence of
   numerals that are interpreted as a number. We then
   do lexicographic comparison on this stream of tokens.
   assumes ascii.

   l < r     -1
   l = r      0
   l > r      1

   XXX this treats

   abc 0123 def
   abc 123 def

   as equal strings. perhaps don't allow 0 to start a
   number?

   n.b. it is easy to overflow here, so perhaps comparing
   as we go is better
*/
auto Util::natural_compare(const string & l, const string & r) -> int {

  for (int caseless = 0; caseless < 2; caseless ++) {

    unsigned int il = 0;
    unsigned int ir = 0;

    while (il < l.length() || ir < r.length()) {
      /* if out of tokens in either string, it comes first. */
      if (il >= l.length()) return -1;
      if (ir >= r.length()) return 1;

      int lc = (unsigned char)l[il];
      int rc = (unsigned char)r[ir];

      if (lc >= '0' && lc <= '9') {
	if (rc >= '0' && rc <= '9') {
	  /* compare ints */
	  int ll = 0;
	  int rr = 0;

	  while (il < l.length() && l[il] >= '0' && l[il] <= '9') {
	    ll *= 10;
	    ll += (l[il] - '0');
	    il ++;
	  }

	  while (ir < r.length() && r[ir] >= '0' && r[ir] <= '9') {
	    rr *= 10;
	    rr += (r[ir] - '0');
	    ir ++;
	  }

	  if (ll < rr) return -1;
	  if (ll > rr) return 1;
	  /* otherwise continue... */

	  il ++;
	  ir ++;
	} else {
	  /* treat numbers larger than any char. */
	  return 1;
	}
      } else {
	if (rc >= '0' && rc <= '9') {
	  return -1;
	} else {
	  /* compare chars */
	  if ((rc|32) >= 'a' && (rc|32) <= 'z' &&
	      (lc|32) >= 'a' && (rc|32) <= 'z' &&
	      !caseless) {

	    /* letters are case-insensitive */
	    if ((lc|32) < (rc|32)) return -1;
	    if ((lc|32) > (rc|32)) return 1;
	  } else {
	    if (lc < rc) return -1;
	    if (lc > rc) return 1;
	  }

	  /* same so far. continue... */

	  il ++;
	  ir ++;
	}
      }

    }
    /* strings look equal when compared
       as case-insensitive. so try again
       sensitive */
  }

  /* strings are case-sensitive equal! */

  return 0;
}

/* same as above, but ignore "the" at beginning */
/* XXX also ignore symbols ie ... at the beginning */
auto Util::library_compare(const string & l, const string & r) -> int {

  /* XXX currently IGNOREs symbols, which could give incorrect
     results for strings that are equal other than their
     leading punctuation */
  unsigned int idxl = 0;
  unsigned int idxr = 0;
  while (idxl < l.length() && (!isalnum(l[idxl]))) idxl++;
  while (idxr < r.length() && (!isalnum(r[idxr]))) idxr++;

  bool thel = false;
  bool ther = false;
  if (l.length() >= (5 + idxl) &&
      (l[idxl + 0]|32) == 't' &&
      (l[idxl + 1]|32) == 'h' &&
      (l[idxl + 2]|32) == 'e' &&
      (l[idxl + 3])    == ' ') thel = true;

  if (r.length() >= (5 + idxr) &&
      (r[idxr + 0]|32) == 't' &&
      (r[idxr + 1]|32) == 'h' &&
      (r[idxr + 2]|32) == 'e' &&
      (r[idxr + 3])    == ' ') ther = true;

  if (thel != ther) {
    if (thel) idxl += 4;
    else idxr += 4;
  }

  return natural_compare (l.substr(idxl, l.length() - idxl),
			  r.substr(idxr, r.length() - idxr));
}

/* XXX impossible to specify a spec for just ^ */
auto Util::matchspec(string spec, char c) -> bool {
  if (!spec.length()) return false;
  else if (spec[0] == '^')
  return !matchspec(spec.substr(1, spec.length() - 1), c);

  /* now loop looking for c in string, or ranges */
  for (unsigned int i = 0; i < spec.length(); i ++) {
    /* ok if starts range, since they are inclusive */
    if (spec[i] == c) return true;

    /* handle ranges */
    if (spec[i] == '-') {
      /* can't be first or last */
      if (i && i < (spec.length() - 1)) {
	if (spec[i - 1] <= c &&
	    spec[i + 1] >= c) return true;
	/* skip dash and next char */
	i ++;
      }
    }
  }
  return false; /* no match */
}


auto Util::library_matches(char k, const string & s) -> bool {
  /* skip symbolic */
  unsigned int idx = 0;
  while (idx < s.length() && (!isalnum(s[idx]))) idx++;

  /* skip 'the' */
  if (s.length() >= (idx + 5) &&
      (s[idx]|32) == 't' &&
      (s[idx + 1]|32) == 'h' &&
      (s[idx + 2]|32) == 'e' &&
      (s[idx + 3])    == ' ') return (s[idx + 4]|32) == (k|32);
  else return (s.length() > 0 && (s[idx]|32) == (k|32));
}

/* try a few methods to remove a file.
   An executable can't remove itself in
   Windows 98, though.
*/
auto Util::remove(string f) -> bool {
  if (!ExistsFile(f.c_str())) return true;
  else {
# ifdef WIN32
    /* We can do this by:
       rename tmp  delme1234.exe
       exec(delme1234.exe "-replace" "escape.exe")
          (now, the program has to have a flag -replace
	   that instructs it to replace escape.exe
	   with itself, then exit)
       .. hopefully exec will unlock the original
       process's executable!! */

    /* try unlinking. if that fails,
       rename it away. */
    if (0 == unlink(f.c_str())) return true;

    string fname = tempfile(".deleteme");
    if (0 == rename(f.c_str(), fname.c_str())) return true;

# else /* posix */
    if (0 == unlink(f.c_str())) return true;
# endif
  }
  return false;

}

auto Util::move(string src, string dst) -> bool {
# if defined(WIN32) || defined(__MINGW32__)
  if (0 == rename(src.c_str(), dst.c_str()))
    return true;
  else return false;

# else /* posix */
  /* XXX actually, posix has rename too. */
  if (0 == link(src.c_str(), dst.c_str())) {
    /* succeed regardless of whether we
       can remove the old link or not. */
    unlink(src.c_str());
    return true;
  } else {
    /* try copy and unlink... (link doesn't work on AFS?) */
    if (copy(src, dst)) {
      unlink(src.c_str());
      return true;
    } return false;
  }
# endif
}


auto Util::copy(string src, string dst) -> bool {
  FILE * s = fopen(src.c_str(), "rb");
  if (!s) return false;
  FILE * d = fopen(dst.c_str(), "wb+");
  if (!d) { fclose(s); return false; }

  std::array<char, 256> buf{};
  size_t x = 0;
  do {
    /* XXX doesn't distinguish error from EOF, but... */
    x = fread(buf.data(), 1, buf.size(), s);
    if (x != 0U) {
      if (fwrite(buf.data(), 1, x, d) < x) {
        fclose(s);
        fclose(d);
        return false;
      }
    }
  } while (x == buf.size());

  fclose(s);
  fclose(d);
  return true;
}

auto Util::dirplus(const string &dir_, const string &file) -> string {
  if (dir_.empty()) return file;
  if (!file.empty() && file[0] == DIRSEPC) return file;
  string dir = dir_;
  if (dir[dir.size() - 1] != DIRSEPC)
    dir += DIRSEPC;
  return dir + file;
}

auto Util::cdup(const string & dir) -> string {
  /* XXX right second argument to rfind? I want to find the last / */
  const size_t idx = dir.rfind(DIRSEP, dir.length() - 1);
  if (idx != string::npos) {
    if (idx != 0U) return dir.substr(0, idx);
    return ".";
  }
  return ".";
}

void Util::createpathfor (string f) {
  string s;
  for (unsigned int i = 0; i < f.length();  i++) {
    if (f[i] == DIRSEPC) {
      /* initial / will cause s == "" for first
	 appearance */
      if (s != "") makedir(s);
    }
    s += f[i];
  }
}

auto Util::fopenp(string f, string m) -> FILE * {
  createpathfor (f);
  return fopen(f.c_str(), m.c_str());
}

auto Util::replace(string src, string findme, string rep) -> string {
  long long int idx = src.length() - 1;

  if (findme.length() < 1) return src;

  /* idx represents the position in src which, for all chars greater
     than it, there begins no match of findme */
  while (idx >= 0 && idx != (signed)string::npos) {
    idx = src.rfind(findme, idx);
    if (idx != (signed)string::npos) {
      /* do replacement */
      src.replace(idx, findme.length(), rep);
      /* want to ensure termination even if rep contains findmes */
      idx -= findme.length();
    } else break;
  }
  return src;
}

/* implementation of imperative union-find is
   beautifully simple */
void onionfind::onion(int a, int b) {
  if (find(a) != find(b)) arr[find(a)] = b;
}

auto onionfind::find(int a) -> int {
  if (arr[a] == -1) return a;
  else return arr[a] = find(arr[a]);
}

onionfind::onionfind(int size) {
  arr = new int[size];
  for (int i = 0; i < size; i++) arr[i] = -1;
}

auto bitbuffer::ceil(int bits) -> int {
  return (bits >> 3) + !!(bits & 7);
}


auto bitbuffer::nbits(string s, int n, int & idx, unsigned int & out) -> bool {
# define NTHBIT(x) !! (s[(x) >> 3] & (1 << (7 - ((x) & 7))))

  out = 0;

  while (n--) {
    out <<= 1;
    /* check bounds */
    if ((unsigned)(idx >> 3) >= s.length()) return false;
    out |= NTHBIT(idx);
    idx ++;
  }
  return true;

# undef NTHBIT
}

auto bitbuffer::getstring() -> string {
  int n = ceil(bits);
  if (data) return string((char *)data, n);
  else return "";
}

void bitbuffer::writebits(int n, unsigned int b) {
  /* assumes position already holds 0 */
# define WRITEBIT(x, v) data[(x) >> 3] |= ((!!v) << (7 - ((x) & 7)))

  /* printf("writebits(%d, %d)\n", n, b); */

  for (int i = 0; i < n; i ++) {
    int bytes_needed = ceil(bits + 1);

    /* allocate more */
    if (bytes_needed > size) {
      int nsize = (size + 1) * 2;
      auto * tmp =
	(unsigned char *) malloc(nsize * sizeof (unsigned char));
      if (!tmp) abort();
      memset(tmp, 0, nsize);
      memcpy(tmp, data, size);
      free(data);
      data = tmp;
      size = nsize;
    }

    int bit = !!(b & (1 << (n - (i + 1))));
    /* printf("  write %d at %d\n", bit, bits); */
    WRITEBIT(bits, bit);
    bits++;
  }

# undef WRITEBIT
}

#ifdef WIN32
// for ShellExecute
# include <shellapi.h>
# include <shlobj.h>
#endif

/* return true on success */
auto Util::launchurl([[maybe_unused]] const string & url) -> bool {
  /* XXX ??? */
#if 0
#ifdef OSX
  CFURLRef urlcfurl = CFURLCreateWithBytes(kCFAllocatorDefault,
					   (const UInt8*)url.c_str(),
					   (CFIndex)strlen(urlstring),
					   kCFStringEncodingASCII, NULL);
  if (urlcfurl) {
      OSStatus status = LSOpenCFURLRef(urlcfurl, NULL);
      CFRelease(urlcfurl);
      return (status == noErr);
    }
  return 0;
#endif
#endif

#ifdef WIN32
  return ((size_t)ShellExecute(NULL, "open", url.c_str(),
			       NULL, NULL, SW_SHOWNORMAL)) > 32;
#endif

  /* otherwise.. */
  return false;
}


auto Util::randfrac() -> float {
  return random() / (float)RAND_MAX;
}

/* XXX, could use better source of randomness (kernel)
   on systems that support it. But we don't have any
   real need for cryptographic randomness.

   web sequence numbers are chosen randomly, now, so we
   actually do.
*/
auto Util::random() -> int {
# if defined(WIN32) || defined(__MINGW32__)
  return ::rand();
# else
  return ::random();
# endif
}

namespace {
/* ensure that random is seeded */
struct RandomSeed {
  RandomSeed() {
# if defined(WIN32) || defined(__MINGW32__)
    srand((int)time(NULL) ^ getpid());
# else
    srandom(time(nullptr) ^ getpid());
# endif
    /* run it a bit */
    for (int i = 0; i < 256; i ++)
      (void)Util::random();
  }
};

[[maybe_unused]] RandomSeed randomseed__unused;
}  // namespace
