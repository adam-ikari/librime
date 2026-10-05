//
// Copyright RIME Developers
// Distributed under the BSD License
//
// 2012-01-17 GONG Chen <chen.sst@gmail.com>
//
#include <cmath>
#include <gtest/gtest.h>
#include <rime/common.h>
#include <rime/algo/calculus.h>

static const char* kTransliteration =
    "xlit abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* kTransformation = "xform/^([zcs])h(.*)$/$1$2/";
static const char* kErasion = "erase/^[czs]h[aoe]ng?$/";
static const char* kDerivation = "derive/^([zcs])h/$1/";
static const char* kAbbreviation = "abbrev/^([zcs]h).*$/$1/";

TEST(RimeCalculusTest, Transliteration) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kTransliteration));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("abracadabra");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("ABRACADABRA", s.str);
}

TEST(RimeCalculusTest, Transformation) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kTransformation));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("sang", s.str);
  // non-matching case
  s.str = "bang";
  EXPECT_FALSE(c->Apply(&s));
}

TEST(RimeCalculusTest, Erasion) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kErasion));
  ASSERT_TRUE(bool(c));
  EXPECT_FALSE(c->addition());
  EXPECT_TRUE(c->deletion());
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("", s.str);
  // non-matching case
  s.str = "bang";
  EXPECT_FALSE(c->Apply(&s));
}

TEST(RimeCalculusTest, Derivation) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kDerivation));
  ASSERT_TRUE(bool(c));
  EXPECT_TRUE(c->addition());
  EXPECT_FALSE(c->deletion());
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("sang", s.str);
  // non-matching case
  s.str = "bang";
  EXPECT_FALSE(c->Apply(&s));
}

TEST(RimeCalculusTest, Abbreviation) {
  rime::Calculus calc;
  rime::the<rime::Calculation> c(calc.Parse(kAbbreviation));
  ASSERT_TRUE(bool(c));
  EXPECT_TRUE(c->addition());
  EXPECT_FALSE(c->deletion());
  rime::Spelling s("shang");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("sh", s.str);
  EXPECT_EQ(rime::kAbbreviation, s.properties.type);
  EXPECT_DOUBLE_EQ(log(0.5), s.properties.credibility);
}

// Regression: this fork compiles calculus patterns with std::regex, which
// implements ECMAScript rather than PCRE. The PCRE shorthand classes \l and \u
// are therefore either misread (\l becomes the literal letter 'l') or rejected
// outright (\u throws std::regex_error). These tests pin the translated
// behaviour so a future regex change cannot silently re-break it — a schema
// that works upstream used to stop matching here, with no error reported
// anywhere.
TEST(RimeCalculusTest, PcreLowercaseClassOutsideCharacterClass) {
  rime::Calculus calc;
  // \l+ must mean "one or more lowercase letters", not "one or more 'l'".
  rime::the<rime::Calculation> c(calc.Parse("xform/^(\\l+)\\d$/$1/"));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("zhang1");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("zhang", s.str);

  // "llll1" matches under BOTH readings (llll is four lowercase letters), so it
  // cannot discriminate. The assertion above is the one that matters: under the
  // ECMAScript misreading this pattern does not match "zhang1" at all.
  rime::Spelling s2("ll1");
  EXPECT_TRUE(c->Apply(&s2));
  EXPECT_EQ("ll", s2.str);

  // Uppercase is outside \l, so it must be left alone.
  rime::Spelling s3("NIHAO1");
  EXPECT_FALSE(c->Apply(&s3));
  EXPECT_EQ("NIHAO1", s3.str);
}

TEST(RimeCalculusTest, PcreUppercaseClassThrowsWithoutTranslation) {
  rime::Calculus calc;
  // \u is not merely misread by ECMAScript, it makes std::regex throw. Without
  // the translation this Parse would blow up rather than return a calculation.
  rime::the<rime::Calculation> c(calc.Parse("xform/^(\\u+)\\d$/$1_/"));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("NIHAO1");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("NIHAO_", s.str);
}

TEST(RimeCalculusTest, PcreClassInsideCharacterClassStaysAUnion) {
  rime::Calculus calc;
  // Inside [...] a bare range is required: rewriting \l to "[a-z]" there would
  // nest ([[a-z]]) instead of unioning.
  rime::the<rime::Calculation> c(calc.Parse("erase/^[a\\l]9$/"));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("a9");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("", s.str);
  rime::Spelling s2("z9");
  EXPECT_TRUE(c->Apply(&s2));
  EXPECT_EQ("", s2.str);
  // 'b' is inside [a-l], so it must erase. An uppercase letter is not, and that
  // is the case that distinguishes a union from a nested "[[a-z]]".
  rime::Spelling s3("B9");
  EXPECT_FALSE(c->Apply(&s3));
  EXPECT_EQ("B9", s3.str);
}

TEST(RimeCalculusTest, EscapedBackslashIsNotTreatedAsAClassShorthand) {
  rime::Calculus calc;
  // "\\l" is an escaped backslash followed by 'l'. Translating it would corrupt
  // a pattern that legitimately wants a literal backslash.
  rime::the<rime::Calculation> c(calc.Parse("xform/^a\\\\l$/b/"));
  ASSERT_TRUE(bool(c));
  rime::Spelling s("a\\l");
  EXPECT_TRUE(c->Apply(&s));
  EXPECT_EQ("b", s.str);
}
