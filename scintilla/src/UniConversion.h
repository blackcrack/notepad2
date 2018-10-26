// Scintilla source code edit control
/** @file UniConversion.h
 ** Functions to handle UTF-8 and UTF-16 strings.
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.

#ifndef UNICONVERSION_H
#define UNICONVERSION_H

namespace Scintilla {

constexpr int UTF8MaxBytes = 4;

constexpr int unicodeReplacementChar = 0xFFFD;

size_t UTF8Length(std::wstring_view wsv) noexcept;
size_t UTF8PositionFromUTF16Position(std::string_view u8Text, size_t positionUTF16) noexcept;
void UTF8FromUTF16(std::wstring_view wsv, char *putf, size_t len) noexcept;
void UTF8FromUTF32Character(int uch, char *putf) noexcept;
size_t UTF16Length(std::string_view sv) noexcept;
size_t UTF16FromUTF8(std::string_view sv, wchar_t *tbuf, size_t tlen);
size_t UTF32FromUTF8(std::string_view sv, unsigned int *tbuf, size_t tlen);
unsigned int UTF16FromUTF32Character(unsigned int val, wchar_t *tbuf) noexcept;
bool UTF8IsValid(std::string_view sv) noexcept;
std::string FixInvalidUTF8(const std::string &text);

extern const unsigned char UTF8ClassifyTable[256];

enum {
	UTF8ClassifyMaskOctetCount = 7,
	UTF8ClassifyMaskTrailByte = 8,
};

inline int UTF8BytesOfLead(unsigned char ch) noexcept {
	return UTF8ClassifyTable[ch] & UTF8ClassifyMaskOctetCount;
}

inline int UnicodeFromUTF8(const unsigned char *us) noexcept {
	switch (UTF8BytesOfLead(us[0])) {
	case 1:
		return us[0];
	case 2:
		return ((us[0] & 0x1F) << 6) + (us[1] & 0x3F);
	case 3:
		return ((us[0] & 0xF) << 12) + ((us[1] & 0x3F) << 6) + (us[2] & 0x3F);
	default:
		return ((us[0] & 0x7) << 18) + ((us[1] & 0x3F) << 12) + ((us[2] & 0x3F) << 6) + (us[3] & 0x3F);
	}
}

inline bool UTF8IsTrailByte(unsigned char ch) noexcept {
	return (UTF8ClassifyTable[ch] & UTF8ClassifyMaskTrailByte) != 0;
}

constexpr bool UTF8IsAscii(unsigned char ch) noexcept {
	return (ch & 0x80) == 0;
}

constexpr bool UTF8IsAscii(unsigned int ch) noexcept {
	return ch < 0x80;
}

enum {
	UTF8MaskWidth = 0x7, UTF8MaskInvalid = 0x8
};
int UTF8ClassifyMulti(const unsigned char *us, size_t len) noexcept;
inline int UTF8Classify(const unsigned char *us, size_t len) noexcept {
	if (UTF8IsAscii(us[0])) {
		return 1;
	}
	return UTF8ClassifyMulti(us, len);
}
inline int UTF8Classify(std::string_view sv) noexcept {
	return UTF8Classify(reinterpret_cast<const unsigned char *>(sv.data()), sv.length());
}

// Similar to UTF8Classify but returns a length of 1 for invalid bytes
// instead of setting the invalid flag
inline int UTF8DrawBytes(const unsigned char *us, int len) noexcept {
	if (UTF8IsAscii(us[0])) {
		return 1;
	}

	const int utf8StatusNext = UTF8ClassifyMulti(us, len);
	return (utf8StatusNext & UTF8MaskInvalid) ? 1 : (utf8StatusNext & UTF8MaskWidth);
}

// Line separator is U+2028 \xe2\x80\xa8
// Paragraph separator is U+2029 \xe2\x80\xa9
constexpr int UTF8SeparatorLength = 3;
inline bool UTF8IsSeparator(const unsigned char *us) noexcept {
	const unsigned int value = (us[0] << 16) | (us[1] << 8) | us[2];
	return value == 0xe280a8 || value == 0xe280a9;
}

// NEL is U+0085 \xc2\x85
constexpr int UTF8NELLength = 2;
inline bool UTF8IsNEL(const unsigned char *us) noexcept {
	const unsigned int value = (us[0] << 8) | us[1];
	return value == 0xc285;
}

enum {
	SURROGATE_LEAD_FIRST = 0xD800,
	SURROGATE_LEAD_LAST = 0xDBFF,
	SURROGATE_TRAIL_FIRST = 0xDC00,
	SURROGATE_TRAIL_LAST = 0xDFFF,
	SUPPLEMENTAL_PLANE_FIRST = 0x10000,
};

constexpr unsigned int UTF16CharLength(wchar_t uch) noexcept {
	return ((uch >= SURROGATE_LEAD_FIRST) && (uch <= SURROGATE_LEAD_LAST)) ? 2 : 1;
}

constexpr unsigned int UTF16LengthFromUTF8ByteCount(unsigned int byteCount) noexcept {
	return (byteCount < 4) ? 1 : 2;
}

}

#endif
