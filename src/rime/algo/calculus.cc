//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2012-01-17 GONG Chen <chen.sst@gmail.com>
//
#include <rime/string_utils.hpp>
#include <utf8.h>
#include <rime/algo/calculus.h>
#include <rime/common.h>

namespace rime {

const double kAbbreviationPenalty = -0.6931471805599453;   // log(0.5)
const double kFuzzySpellingPenalty = -0.6931471805599453;  // log(0.5)
const double kCorrectionPenalty = -4.605170185988091;      // log(0.01)

namespace {

// Upstream librime compiles xform/derive/abbrev/erase patterns with
// boost::regex, which implements PCRE. This fork replaced it with std::regex to
// drop the Boost dependency, but std::regex implements ECMAScript, and the two
// disagree about the PCRE shorthand classes in a way that fails silently:
//
//   ^(\l+)\d$   against "zhang1"  PCRE: matches.  ECMAScript: \l is an escape
//                                         for the literal letter 'l', so it
//                                         matches "lll1" and not "zhang1".
//   ^(\u)\d$    against "Z1"       PCRE: matches.  ECMAScript: throws
//                                         std::regex_error outright.
//
// A schema that works upstream therefore either stops matching or crashes here,
// and nothing reports it. Rewrite the shorthands ECMAScript lacks or reads
// differently, so behaviour matches upstream without pulling Boost back in.
//
// \d \D \w \W \s \S are deliberately left alone: ECMAScript defines them with the
// same meaning as PCRE. \p{...} is not translated — ECMAScript has no Unicode
// property support, and silently matching something else would be worse than
// failing, so it is left to raise the regex_error it always did.
string TranslatePcreClasses(const string& pattern) {
  string out;
  out.reserve(pattern.size() + 8);
  bool in_class = false;  // inside [...]
  for (size_t i = 0; i < pattern.size(); ++i) {
    char c = pattern[i];
    if (c == '\\' && i + 1 < pattern.size()) {
      char next = pattern[i + 1];
      if (next == '\\') {  // an escaped backslash: copy both, stay literal
        out += c;
        out += next;
        ++i;
        continue;
      }
      // Inside a character class a bare range has to stay bare, because
      // "[[a-z]]" would nest instead of unioning. Outside one, the class form is
      // required — and it composes with quantifiers: \l+ -> [a-z]+.
      const char* expanded = nullptr;
      switch (next) {
        case 'l': expanded = in_class ? "a-z"   : "[a-z]"; break;
        case 'u': expanded = in_class ? "A-Z"   : "[A-Z]"; break;
        default: break;
      }
      if (expanded) {
        out += expanded;
      } else {
        out += c;
        out += next;
      }
      ++i;
      continue;
    }
    if (c == '[' && !in_class)
      in_class = true;
    else if (c == ']' && in_class)
      in_class = false;
    out += c;
  }
  return out;
}

}  // namespace

Calculus::Calculus() {
  Register("xlit", &Transliteration::Parse);
  Register("xform", &Transformation::Parse);
  Register("erase", &Erasion::Parse);
  Register("derive", &Derivation::Parse);
  Register("fuzz", &Fuzzing::Parse);
  Register("abbrev", &Abbreviation::Parse);
}

void Calculus::Register(const string& token, Calculation::Factory* factory) {
  factories_[token] = factory;
}

Calculation* Calculus::Parse(const string& definition) {
  size_t sep = definition.find_first_not_of("zyxwvutsrqponmlkjihgfedcba");
  if (sep == string::npos)
    return NULL;
  vector<string> args;
  boost::split(args, definition,
               boost::is_from_range(definition[sep], definition[sep]));
  if (args.empty())
    return NULL;
  auto it = factories_.find(args[0]);
  if (it == factories_.end())
    return NULL;
  Calculation* result = (*it->second)(args);
  return result;
}

// Transliteration

Calculation* Transliteration::Parse(const vector<string>& args) {
  if (args.size() < 3)
    return NULL;
  const string& left(args[1]);
  const string& right(args[2]);
  const char* pl = left.c_str();
  const char* pr = right.c_str();
  uint32_t cl, cr;
  map<uint32_t, uint32_t> char_map;
  while ((cl = utf8::unchecked::next(pl)), (cr = utf8::unchecked::next(pr)),
         cl && cr) {
    char_map[cl] = cr;
  }
  if (cl == 0 && cr == 0) {
    the<Transliteration> x(new Transliteration);
    x->char_map_.swap(char_map);
    return x.release();
  }
  return NULL;
}

bool Transliteration::Apply(Spelling* spelling) {
  if (!spelling || spelling->str.empty())
    return false;
  bool modified = false;
  const char* p = spelling->str.c_str();
  const int buffer_len = 256;
  char buffer[buffer_len] = "";
  char* q = buffer;
  uint32_t c;
  while ((c = utf8::unchecked::next(p))) {
    if (q - buffer > buffer_len - 7) {  // insufficient space
      modified = false;
      break;
    }
    if (char_map_.find(c) != char_map_.end()) {
      c = char_map_[c];
      modified = true;
    }
    q = utf8::unchecked::append(c, q);
  }
  if (modified) {
    *q = '\0';
    spelling->str.assign(buffer);
  }
  return modified;
}

// Transformation

Calculation* Transformation::Parse(const vector<string>& args) {
  if (args.size() < 3)
    return NULL;
  const string& left(args[1]);
  const string& right(args[2]);
  if (left.empty())
    return NULL;
  the<Transformation> x(new Transformation);
  x->pattern_.assign(TranslatePcreClasses(left));
  x->replacement_.assign(right);
  return x.release();
}

bool Transformation::Apply(Spelling* spelling) {
  if (!spelling || spelling->str.empty())
    return false;
  string result = std::regex_replace(spelling->str, pattern_, replacement_);
  if (result == spelling->str)
    return false;
  spelling->str.swap(result);
  return true;
}

// Erasion

Calculation* Erasion::Parse(const vector<string>& args) {
  if (args.size() < 2)
    return NULL;
  const string& pattern(args[1]);
  if (pattern.empty())
    return NULL;
  the<Erasion> x(new Erasion);
  x->pattern_.assign(TranslatePcreClasses(pattern));
  return x.release();
}

bool Erasion::Apply(Spelling* spelling) {
  if (!spelling || spelling->str.empty())
    return false;
  if (!std::regex_match(spelling->str, pattern_))
    return false;
  spelling->str.clear();
  return true;
}

// Derivation

Calculation* Derivation::Parse(const vector<string>& args) {
  if (args.size() < 3)
    return NULL;

  const string& left(args[1]);
  const string& right(args[2]);
  if (left.empty())
    return NULL;

  if (args.size() > 3) {
    const string& tag = args[3];
    // 糾錯
    if (tag == "correction") {
      the<Correction> x(new Correction);
      x->pattern_.assign(TranslatePcreClasses(left));
      x->replacement_.assign(right);
      return x.release();
    }
    // 簡拼
    if (tag == "abbrev") {
      the<Abbreviation> x(new Abbreviation);
      x->pattern_.assign(TranslatePcreClasses(left));
      x->replacement_.assign(right);
      return x.release();
    }
    // 模糊音
    if (tag == "fuzz") {
      the<Fuzzing> x(new Fuzzing);
      x->pattern_.assign(TranslatePcreClasses(left));
      x->replacement_.assign(right);
      return x.release();
    }
    // tag 無法識別, 作爲普通 derive 處理
  }

  the<Derivation> x(new Derivation);
  x->pattern_.assign(TranslatePcreClasses(left));
  x->replacement_.assign(right);
  return x.release();
}

// Fuzzing

Calculation* Fuzzing::Parse(const vector<string>& args) {
  if (args.size() < 3)
    return NULL;
  const string& left(args[1]);
  const string& right(args[2]);
  if (left.empty())
    return NULL;
  the<Fuzzing> x(new Fuzzing);
  x->pattern_.assign(TranslatePcreClasses(left));
  x->replacement_.assign(right);
  return x.release();
}

bool Fuzzing::Apply(Spelling* spelling) {
  bool result = Transformation::Apply(spelling);
  if (result) {
    spelling->properties.type = kFuzzySpelling;
    spelling->properties.credibility += kFuzzySpellingPenalty;
  }
  return result;
}

// Abbreviation

Calculation* Abbreviation::Parse(const vector<string>& args) {
  if (args.size() < 3)
    return NULL;
  const string& left(args[1]);
  const string& right(args[2]);
  if (left.empty())
    return NULL;
  the<Abbreviation> x(new Abbreviation);
  x->pattern_.assign(TranslatePcreClasses(left));
  x->replacement_.assign(right);
  return x.release();
}

bool Abbreviation::Apply(Spelling* spelling) {
  bool result = Transformation::Apply(spelling);
  if (result) {
    spelling->properties.type = kAbbreviation;
    spelling->properties.credibility += kAbbreviationPenalty;
  }
  return result;
}

// Correction

bool Correction::Apply(Spelling* spelling) {
  bool result = Transformation::Apply(spelling);
  if (result) {
    spelling->properties.is_correction = true;
    spelling->properties.credibility += kCorrectionPenalty;
  }
  return result;
}

}  // namespace rime
