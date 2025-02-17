﻿#include "Common.hpp"
#include <memory>
#include <LuminoCore/Base/String.hpp>
#include <LuminoCore/Text/Encoding.hpp>

static UTF8 g_utf8Hokke[] = { 0xF0, 0xA9, 0xB8, 0xBD }; // ほっけ
static UTF16 g_utf16Hokke[] = { 0xD867, 0xDE3D };       // ほっけ
static UTF32 g_utf32Hokke[] = { 0x00029E3D };           // ほっけ
static UTF8 g_utf8Buf[] = {
    0xE6,
    0x97,
    0xA5, // '日'
    0xE6,
    0x9C,
    0xAC, // '本'
    0xE8,
    0xAA,
    0x9E, // '語'
};
static UTF16 g_utf16Buf[3] = { 0x65E5, 0x672C, 0x8A9E }; // "日本語"
static UTF32 g_utf32Buf[3] = { 0x65E5, 0x672C, 0x8A9E }; // "日本語"

//==============================================================================
//# TextEncoding
class Test_Text_TextEncoding : public ::testing::Test {
};

// simple conversion
TEST_F(Test_Text_TextEncoding, SimpleConversion) {
    // bytes -> String
    String str = TextEncoding::utf8Encoding()->decode(g_utf8Buf, sizeof(g_utf8Buf));
    ASSERT_EQ(3, str.length());
    ASSERT_EQ(0, memcmp(g_utf32Buf, str.c_str(), 6));

    // String -> bytes
    std::vector<byte_t> bytes = TextEncoding::utf8Encoding()->encode(str);
    ASSERT_EQ(9, bytes.size());
    ASSERT_EQ(0, memcmp(g_utf8Buf, bytes.data(), 9));
}

#if LN_USTRING32
#ifdef _WIN32
#include "../src/Text/Win32CodePageEncoding.hpp"

//==============================================================================
class Test_Text_Win32CodePageEncoding : public ::testing::Test {
};

TEST_F(Test_Text_Win32CodePageEncoding, convertFromUTF32) {
    auto encoding = makeRef<Win32CodePageEncoding>(932);
    std::unique_ptr<TextEncoder> encoder(encoding->createEncoder());

    const unsigned char narrowStr[7] = { 0x93, 0xFA, 0x96, 0x7B, 0x8C, 0xEA, 0x00 }; // "日本語";
    unsigned char narrowBuf[8] = {};
    TextEncodeResult result;

    // Convert one character at a time.
    {
        ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[0], 1, (byte_t*)narrowBuf, 2, &result));
        ASSERT_EQ(narrowStr[0], narrowBuf[0]);
        ASSERT_EQ(narrowStr[1], narrowBuf[1]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(2, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);

        ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[1], 1, (byte_t*)narrowBuf, 2, &result));
        ASSERT_EQ(narrowStr[2], narrowBuf[0]);
        ASSERT_EQ(narrowStr[3], narrowBuf[1]);

        ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[2], 1, (byte_t*)narrowBuf, 2, &result));
        ASSERT_EQ(narrowStr[4], narrowBuf[0]);
        ASSERT_EQ(narrowStr[5], narrowBuf[1]);
    }

    // Convert in bulk
    {
        ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[0], 3, (byte_t*)narrowBuf, 6, &result));
        ASSERT_EQ(0, strncmp((const char*)narrowStr, (const char*)narrowBuf, 6));
        ASSERT_EQ(3, result.usedElementCount);
        ASSERT_EQ(6, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }

    // Big string
    {
        const Char* utf32Str = U"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
        const char* narrowStr = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
        char narrowBuf[1024] = {};
        ASSERT_EQ(true, encoder->convertFromUTF32((const UTF32*)utf32Str, std::char_traits<Char>::length(utf32Str), (byte_t*)narrowBuf, 1024, &result));
        ASSERT_EQ(0, strncmp(narrowStr, narrowBuf, strlen(narrowStr)));
    }
}

TEST_F(Test_Text_Win32CodePageEncoding, convertToUTF32) {
    auto encoding = makeRef<Win32CodePageEncoding>(932);
    std::unique_ptr<TextDecoder> decoder(encoding->createDecoder());

    const unsigned char narrowStr[7] = { 0x93, 0xFA, 0x96, 0x7B, 0x8C, 0xEA, 0x00 }; // "日本語";
    UTF32 strBuf[8] = {};
    TextDecodeResult result;

    // Convert one character at a time.
    {
        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)&narrowStr[0], 1, strBuf, 3, &result));
        ASSERT_EQ(1, result.usedByteCount); // LeadByte consumed.
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)&narrowStr[1], 1, strBuf, 3, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]); // '日'
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)&narrowStr[0], 5, strBuf, 3, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]); // '日'
        ASSERT_EQ(g_utf32Buf[1], strBuf[1]); // '本'
        ASSERT_EQ(5, result.usedByteCount);
        ASSERT_EQ(8, result.outputByteCount);
        ASSERT_EQ(2, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)&narrowStr[5], 1, strBuf, 3, &result));
        ASSERT_EQ(g_utf32Buf[2], strBuf[0]); // '語'
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }

    // Convert in bulk
    {
        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)&narrowStr[0], 6, strBuf, 3, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]); // '日'
        ASSERT_EQ(g_utf32Buf[1], strBuf[1]); // '本'
        ASSERT_EQ(g_utf32Buf[2], strBuf[2]); // '語'
        ASSERT_EQ(6, result.usedByteCount);
        ASSERT_EQ(12, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }

    // Big string
    {
        const Char* utf32Str = U"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
        const char* narrowStr = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
        Char strBuf[1024] = {};
        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)narrowStr, std::char_traits<char>::length(narrowStr), (UTF32*)strBuf, 1024, &result));
        ASSERT_EQ(0, StringHelper::compare(strBuf, utf32Str, (int)StringHelper::strlen(utf32Str)));
    }
}

// SJIS として不正なシーケンス (SJIS 想定なのに URF-8 を入力した) のときに、クラッシュしないこと
TEST_F(Test_Text_Win32CodePageEncoding, convertToUTF32_Issue1) {
    auto encoding = makeRef<Win32CodePageEncoding>(932);
    std::unique_ptr<TextDecoder> decoder(encoding->createDecoder());
    TextDecodeResult result;

    const unsigned char narrowStr[] = { 0x45, 0x72, 0x72, 0x6f, 0x72, 0x3a, 0x20, 0x7d, 0x20, 0xe3, 0x81, 0x8c, 0xe7, 0x84, 0xa1, 0xe3, 0x81, 0x84, 0x0d, 0x0a };
    Char strBuf[1024] = {};
    ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)narrowStr, strlen((const char*)narrowStr), (UTF32*)strBuf, 1024, &result));
}

#endif
#endif

//==============================================================================
// UTF8 TextEncoding
class Test_Text_UTF8Encoding : public ::testing::Test {
};

#if LN_USTRING32
// bytes(UTF8) -> IStr(UTF32) conversion
TEST_F(Test_Text_UTF8Encoding, convertToUTF32) {
    std::unique_ptr<TextDecoder> decoder(TextEncoding::utf8Encoding()->createDecoder());

    UTF8 utf8Buf[] = {
        0xE6, 0x97, 0xA5, // '日'
        0xE6,
        0x9C,
        0xAC, // '本'
        0xE8,
        0xAA,
        0x9E, // '語'
    };
    UTF32 strBuf[3] = { 0, 0, 0 };
    TextDecodeResult result;

    // '日' を 1byte ずつ状態を保持しながら変換
    ASSERT_EQ(true, decoder->convertToUTF32(&utf8Buf[0], 1, &strBuf[0], 1, &result));
    ASSERT_EQ(0x0000, strBuf[0]);         // まだ文字は出ない
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
    ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない

    ASSERT_EQ(true, decoder->convertToUTF32(&utf8Buf[1], 1, &strBuf[0], 1, &result));
    ASSERT_EQ(0x0000, strBuf[0]);         // まだ文字は出ない
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
    ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない

    ASSERT_EQ(true, decoder->convertToUTF32(&utf8Buf[2], 1, &strBuf[0], 1, &result));
    ASSERT_EQ(0x65E5, strBuf[0]);         // '日'
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(4, result.outputByteCount); // 2バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた

    // 1文字変換 (サロゲート)
    ASSERT_EQ(true, decoder->convertToUTF32(&g_utf8Hokke[0], 4, &strBuf[0], 2, &result));
    ASSERT_EQ(g_utf32Hokke[0], strBuf[0]); // ほっけ
    ASSERT_EQ(4, result.usedByteCount);    // 4byte消費した
    ASSERT_EQ(4, result.outputByteCount);  // 4バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount);  // 1文字出てきた

    // '本' を一度に変換
    ASSERT_EQ(true, decoder->convertToUTF32(&utf8Buf[3], 3, &strBuf[1], 1, &result));
    ASSERT_EQ(0x672C, strBuf[1]);         // '本'
    ASSERT_EQ(3, result.usedByteCount);   // 3byte消費した
    ASSERT_EQ(4, result.outputByteCount); // 2バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた

    // '語' を 2, 1byte に分けて変換
    ASSERT_EQ(true, decoder->convertToUTF32(&utf8Buf[6], 2, &strBuf[2], 1, &result));
    ASSERT_EQ(0x0000, strBuf[2]);         // まだ文字は出ない
    ASSERT_EQ(2, result.usedByteCount);   // 2byte消費した
    ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
    ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない
    decoder->convertToUTF32(&utf8Buf[8], 1, &strBuf[2], 1, &result);
    ASSERT_EQ(0x8A9E, strBuf[2]);         // '語'
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(4, result.outputByteCount); // 2バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた

    // ASCII をまとめて変換
    {
        const UTF8* utf8str = (const UTF8*)"abcd";
        UTF32 strBuf[] = { 0, 0, 0, 0 };
        ASSERT_EQ(true, decoder->convertToUTF32(utf8str, 4, strBuf, 4, &result));
        ASSERT_EQ('a', strBuf[0]);
        ASSERT_EQ('b', strBuf[1]);
        ASSERT_EQ('c', strBuf[2]);
        ASSERT_EQ('d', strBuf[3]);
    }

    // "日本語" をまとめて変換
    {
        memset(strBuf, 0, sizeof(strBuf));
        ASSERT_EQ(true, decoder->convertToUTF32(&utf8Buf[0], 9, &strBuf[0], 3, &result));
        ASSERT_EQ(9, result.usedByteCount);
        ASSERT_EQ(12, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
        ASSERT_EQ(0, memcmp(strBuf, g_utf32Buf, sizeof(strBuf)));
    }
}

// IStr(UTF32) -> bytes(UTF8) conversion
TEST_F(Test_Text_UTF8Encoding, convertFromUTF32) {
    std::unique_ptr<TextEncoder> encoder(TextEncoding::utf8Encoding()->createEncoder());

    UTF8 utf8Buf[6];
    TextEncodeResult result;

    ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[0], 1, utf8Buf, 6, &result));
    ASSERT_EQ(0xE6, utf8Buf[0]);
    ASSERT_EQ(0x97, utf8Buf[1]);
    ASSERT_EQ(0xA5, utf8Buf[2]);           // '日'
    ASSERT_EQ(1, result.usedElementCount); // UTF32 1要素消費した
    ASSERT_EQ(3, result.outputByteCount);  // 3バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount);  // 1文字出てきた

    ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[1], 2, utf8Buf, 6, &result));
    ASSERT_EQ(0xE6, utf8Buf[0]);
    ASSERT_EQ(0x9C, utf8Buf[1]);
    ASSERT_EQ(0xAC, utf8Buf[2]); // '本'
    ASSERT_EQ(0xE8, utf8Buf[3]);
    ASSERT_EQ(0xAA, utf8Buf[4]);
    ASSERT_EQ(0x9E, utf8Buf[5]); // '語'
    ASSERT_EQ(2, result.usedElementCount);
    ASSERT_EQ(6, result.outputByteCount);
    ASSERT_EQ(2, result.outputCharCount);

    ASSERT_EQ(true, encoder->convertFromUTF32(g_utf32Hokke, 1, utf8Buf, 6, &result));
    ASSERT_EQ(g_utf8Hokke[0], utf8Buf[0]);
    ASSERT_EQ(g_utf8Hokke[1], utf8Buf[1]);
    ASSERT_EQ(g_utf8Hokke[2], utf8Buf[2]);
    ASSERT_EQ(g_utf8Hokke[3], utf8Buf[3]);
    ASSERT_EQ(1, result.usedElementCount);
    ASSERT_EQ(4, result.outputByteCount);
    ASSERT_EQ(1, result.outputCharCount);
}

//==============================================================================
//# UTF16 TextEncoding
class Test_Text_UTF16Encoding : public ::testing::Test {
};

//## bytes(UTF16) -> IStr(UTF32) conversion
TEST_F(Test_Text_UTF16Encoding, convertToUTF32) {
    std::unique_ptr<TextDecoder> decoder(TextEncoding::utf16Encoding()->createDecoder());
    UTF32 strBuf[3] = { 0, 0, 0 };
    auto* utf16Buf = (const byte_t*)g_utf16Buf;
    TextDecodeResult result;

    // input every 1bytes
    {
        ASSERT_EQ(true, decoder->convertToUTF32(&utf16Buf[0], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(0x0000, strBuf[0]);         // まだ文字は出ない
        ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
        ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
        ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない

        ASSERT_EQ(true, decoder->convertToUTF32(&utf16Buf[1], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(0x65E5, strBuf[0]);         // "日"
        ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
        ASSERT_EQ(4, result.outputByteCount); // 2byte書き込まれた
        ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた
    }

    // contains surrogate pair
    {
        auto bytes = (const byte_t*)g_utf16Hokke;

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[0], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[1], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[2], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[3], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(g_utf32Hokke[0], strBuf[0]); // "ほっけ"
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }

    // Convert in bulk
    {
        ASSERT_EQ(true, decoder->convertToUTF32((const byte_t*)g_utf16Buf, 6, strBuf, 3, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]); // '日'
        ASSERT_EQ(g_utf32Buf[1], strBuf[1]); // '本'
        ASSERT_EQ(g_utf32Buf[2], strBuf[2]); // '語'
        ASSERT_EQ(6, result.usedByteCount);
        ASSERT_EQ(12, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }
}

// IStr(UTF32) -> bytes(UTF16) conversion
TEST_F(Test_Text_UTF16Encoding, convertFromUTF32) {
    std::unique_ptr<TextEncoder> encoder(TextEncoding::utf16Encoding()->createEncoder());
    UTF16 utf16Buf[3] = { 0, 0, 0 };
    TextEncodeResult result;

    // input every 1bytes
    {
        auto* bytes = (byte_t*)utf16Buf;

        // 1文字変換
        ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Buf[0], 1, bytes, 6, &result));
        ASSERT_EQ(0x65E5, utf16Buf[0]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(2, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }

    // Convert in bulk
    {
        auto* bytes = (byte_t*)utf16Buf;

        ASSERT_EQ(true, encoder->convertFromUTF32(&g_utf32Hokke[0], 1, bytes, 6, &result));
        ASSERT_EQ(g_utf16Hokke[0], utf16Buf[0]);
        ASSERT_EQ(g_utf16Hokke[1], utf16Buf[1]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }
}

//==============================================================================
//# UTF32 TextEncoding
class Test_Text_UTF32Encoding : public ::testing::Test {
};

// bytes(UTF32) -> IStr(UTF32) conversion
TEST_F(Test_Text_UTF32Encoding, convertToUTF32) {
    std::unique_ptr<TextDecoder> decoder(TextEncoding::utf32Encoding()->createDecoder());
    UTF32 strBuf[3] = { 0, 0, 0 };
    TextDecodeResult result;

    // all at once
    {
        auto* bytes = (const byte_t*)g_utf32Buf;
        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[0], 12, &strBuf[0], 3, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]);
        ASSERT_EQ(g_utf32Buf[1], strBuf[1]);
        ASSERT_EQ(g_utf32Buf[2], strBuf[2]);
        ASSERT_EQ(12, result.usedByteCount);
        ASSERT_EQ(12, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }

    // input every 1bytes
    {
        auto* bytes = (const byte_t*)g_utf32Buf;

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[0], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[1], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[2], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF32(&bytes[3], 1, &strBuf[0], 1, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]); // '日'
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }
}

//## IStr(UTF32) -> bytes(UTF32) conversion
TEST_F(Test_Text_UTF32Encoding, convertFromUTF32) {
    std::unique_ptr<TextEncoder> encoder(TextEncoding::utf32Encoding()->createEncoder());
    UTF32 strBuf[3] = { 0, 0, 0 };
    TextEncodeResult result;

    // all at once
    {
        ASSERT_EQ(true, encoder->convertFromUTF32(g_utf32Buf, 3, (byte_t*)strBuf, 12, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]);
        ASSERT_EQ(g_utf32Buf[1], strBuf[1]);
        ASSERT_EQ(g_utf32Buf[2], strBuf[2]);
        ASSERT_EQ(3, result.usedElementCount);
        ASSERT_EQ(12, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }

    // input every 1 elements
    {
        ASSERT_EQ(true, encoder->convertFromUTF32(g_utf32Buf, 1, (byte_t*)strBuf, 4, &result));
        ASSERT_EQ(g_utf32Buf[0], strBuf[0]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }
}

#else
//## bytes(UTF8) -> UTF16 conversion
TEST_F(Test_Text_UTF8Encoding, BytesToUTF16Conversion) {
    std::unique_ptr<TextDecoder> decoder(TextEncoding::utf8Encoding()->createDecoder());

    UTF8 utf8Buf[] = {
        0xE6, 0x97, 0xA5, // '日'
        0xE6,
        0x9C,
        0xAC, // '本'
        0xE8,
        0xAA,
        0x9E, // '語'
    };
    UTF16 utf32Buf[3] = { 0, 0, 0 };
    TextDecodeResult result;

    // '日' を 1byte ずつ状態を保持しながら変換
    ASSERT_EQ(true, decoder->convertToUTF16(&utf8Buf[0], 1, &utf16Buf[0], 1, &result));
    ASSERT_EQ(0x0000, utf16Buf[0]);       // まだ文字は出ない
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
    ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない

    ASSERT_EQ(true, decoder->convertToUTF16(&utf8Buf[1], 1, &utf16Buf[0], 1, &result));
    ASSERT_EQ(0x0000, utf16Buf[0]);       // まだ文字は出ない
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
    ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない

    ASSERT_EQ(true, decoder->convertToUTF16(&utf8Buf[2], 1, &utf16Buf[0], 1, &result));
    ASSERT_EQ(0x65E5, utf16Buf[0]);       // '日'
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(2, result.outputByteCount); // 2バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた

    // 1文字変換 (サロゲート)
    ASSERT_EQ(true, decoder->convertToUTF16(&g_utf8Hokke[0], 4, &utf16Buf[0], 2, &result));
    ASSERT_EQ(0, memcmp(g_utf16Hokke, utf16Buf, 4)); // ほっけ
    ASSERT_EQ(4, result.usedByteCount);              // 4byte消費した
    ASSERT_EQ(4, result.outputByteCount);            // 4バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount);            // 1文字出てきた

    // '本' を一度に変換
    ASSERT_EQ(true, decoder->convertToUTF16(&utf8Buf[3], 3, &utf16Buf[1], 1, &result));
    ASSERT_EQ(0x672C, utf16Buf[1]);       // '本'
    ASSERT_EQ(3, result.usedByteCount);   // 3byte消費した
    ASSERT_EQ(2, result.outputByteCount); // 2バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた

    // '語' を 2, 1byte に分けて変換
    ASSERT_EQ(true, decoder->convertToUTF16(&utf8Buf[6], 2, &utf16Buf[2], 1, &result));
    ASSERT_EQ(0x0000, utf16Buf[2]);       // まだ文字は出ない
    ASSERT_EQ(2, result.usedByteCount);   // 2byte消費した
    ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
    ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない
    decoder->convertToUTF16(&utf8Buf[8], 1, &utf16Buf[2], 1, &result);
    ASSERT_EQ(0x8A9E, utf16Buf[2]);       // '語'
    ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
    ASSERT_EQ(2, result.outputByteCount); // 2バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた

    // "日本語" をまとめて変換
    memset(utf16Buf, 0, sizeof(utf16Buf));
    ASSERT_EQ(true, decoder->convertToUTF16(&utf8Buf[0], 9, &utf16Buf[0], 3, &result));
    ASSERT_EQ(9, result.usedByteCount);
    ASSERT_EQ(6, result.outputByteCount);
    ASSERT_EQ(3, result.outputCharCount);
    ASSERT_EQ(0, memcmp(utf16Buf, g_utf16Buf, sizeof(utf16Buf)));
}

//## UTF16 -> bytes(UTF8) conversion
TEST_F(Test_Text_UTF8Encoding, UTF16ToBytesConversion) {
    std::unique_ptr<TextEncoder> encoder(TextEncoding::utf8Encoding()->createEncoder());

    UTF8 utf8Buf[6];
    TextEncodeResult result;

    // 1文字変換
    ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Buf[0], 1, utf8Buf, 6, &result));
    ASSERT_EQ(0xE6, utf8Buf[0]);
    ASSERT_EQ(0x97, utf8Buf[1]);
    ASSERT_EQ(0xA5, utf8Buf[2]);           // '日'
    ASSERT_EQ(1, result.usedElementCount); // UTF16 1要素消費した
    ASSERT_EQ(3, result.outputByteCount);  // 3バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount);  // 1文字出てきた

    // 1文字変換 (サロゲート)
    ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Hokke[0], 2, utf8Buf, 6, &result));
    ASSERT_EQ(0, memcmp(utf8Buf, g_utf8Hokke, 4)); // ほっけ
    ASSERT_EQ(2, result.usedElementCount);         // UTF16 2要素消費した
    ASSERT_EQ(4, result.outputByteCount);          // 4バイト書き込まれた
    ASSERT_EQ(1, result.outputCharCount);          // 1文字出てきた

    // 2文字変換
    ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Buf[1], 2, utf8Buf, 6, &result));
    ASSERT_EQ(0xE6, utf8Buf[0]);
    ASSERT_EQ(0x9C, utf8Buf[1]);
    ASSERT_EQ(0xAC, utf8Buf[2]); // '本'
    ASSERT_EQ(0xE8, utf8Buf[3]);
    ASSERT_EQ(0xAA, utf8Buf[4]);
    ASSERT_EQ(0x9E, utf8Buf[5]);          // '語'
    ASSERT_EQ(6, result.outputByteCount); // 6バイト書き込まれた
    ASSERT_EQ(2, result.outputCharCount); // 2文字出てきた
}

//==============================================================================
//# UTF16 TextEncoding
class Test_Text_UTF16Encoding : public ::testing::Test {
};

//## bytes(UTF16) -> UTF16 conversion
TEST_F(Test_Text_UTF16Encoding, BytesToUTF16Conversion) {
    std::unique_ptr<TextDecoder> decoder(TextEncoding::utf16Encoding()->createDecoder());
    UTF16 utf16Buf[3] = { 0, 0, 0 };
    auto* bytes = (const byte_t*)g_utf16Buf;
    TextDecodeResult result;

    //### - [ ] input every 1bytes
    {
        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[0], 1, &utf16Buf[0], 1, &result));
        ASSERT_EQ(0x0000, utf16Buf[0]);       // まだ文字は出ない
        ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
        ASSERT_EQ(0, result.outputByteCount); // まだバイトも書き込まれない
        ASSERT_EQ(0, result.outputCharCount); // まだ文字は出ない

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[1], 1, &utf16Buf[0], 1, &result));
        ASSERT_EQ(0x65E5, utf16Buf[0]);       // "日"
        ASSERT_EQ(1, result.usedByteCount);   // 1byte消費した
        ASSERT_EQ(2, result.outputByteCount); // 2byte書き込まれた
        ASSERT_EQ(1, result.outputCharCount); // 1文字出てきた
    }

    //### - [ ] input every 1bytes (contains surrogate pair)
    {
        auto* bytes = (const byte_t*)g_utf16Hokke;

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[0], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[1], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[2], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[3], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(0, memcmp(utf16Buf, g_utf16Hokke, 4));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }
}

//## UTF16 -> bytes(UTF16) conversion
TEST_F(Test_Text_UTF16Encoding, UTF16ToBytesConversion) {
    std::unique_ptr<TextEncoder> encoder(TextEncoding::utf16Encoding()->createEncoder());
    UTF16 utf16Buf[3] = { 0, 0, 0 };
    TextEncodeResult result;

    //### - [ ] input every 1bytes
    {
        auto* bytes = (byte_t*)utf16Buf;

        // 1文字変換
        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Buf[0], 1, bytes, 6, &result));
        ASSERT_EQ(0x65E5, utf16Buf[0]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(2, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }

    //### - [ ] input every 1bytes (contains surrogate pair)
    {
        auto* bytes = (byte_t*)utf16Buf;

        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Hokke[0], 1, bytes, 6, &result));
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Hokke[1], 1, bytes, 6, &result));
        ASSERT_EQ(0, memcmp(utf16Buf, g_utf16Hokke, 4));
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(2, result.outputCharCount);
    }
}

//==============================================================================
//# UTF32 TextEncoding
class Test_Text_UTF32Encoding : public ::testing::Test {
};

//## bytes(UTF32) -> UTF16 conversion
TEST_F(Test_Text_UTF32Encoding, BytesToUTF16Conversion) {
    std::unique_ptr<TextDecoder> decoder(TextEncoding::utf32Encoding()->createDecoder());
    UTF16 utf16Buf[3] = { 0, 0, 0 };
    TextDecodeResult result;

    //### - [ ] all at once
    {
        auto* bytes = (const byte_t*)g_utf32Buf;
        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[0], sizeof(g_utf32Buf), &utf16Buf[0], 3, &result));
        ASSERT_EQ(0, memcmp(utf16Buf, g_utf16Buf, sizeof(utf16Buf)));
        ASSERT_EQ(12, result.usedByteCount);
        ASSERT_EQ(6, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }

    //### - [ ] input every 1bytes
    {
        auto* bytes = (const byte_t*)g_utf32Buf;

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[0], 1, &utf16Buf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[1], 1, &utf16Buf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[2], 1, &utf16Buf[0], 1, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[3], 1, &utf16Buf[0], 1, &result));
        ASSERT_EQ(0x65E5, utf16Buf[0]); // "日"
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(2, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }

    //### - [ ] input every 1bytes (contains surrogate pair)
    {
        auto* bytes = (const byte_t*)g_utf32Hokke;

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[0], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[1], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[2], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, decoder->convertToUTF16(&bytes[3], 1, &utf16Buf[0], 2, &result));
        ASSERT_EQ(0, memcmp(utf16Buf, g_utf16Hokke, 4));
        ASSERT_EQ(1, result.usedByteCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }
}

//## UTF16 -> bytes(UTF32) conversion
TEST_F(Test_Text_UTF32Encoding, BytesToUTF32Conversion) {
    std::unique_ptr<TextEncoder> encoder(TextEncoding::utf32Encoding()->createEncoder());
    UTF32 utf32Buf[3] = { 0, 0, 0 };
    TextEncodeResult result;

    //### - [ ] all at once
    {
        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Buf[0], 3, (byte_t*)&utf32Buf[0], 12, &result));
        ASSERT_EQ(0, memcmp(utf32Buf, g_utf32Buf, sizeof(utf32Buf)));
        ASSERT_EQ(3, result.usedElementCount);
        ASSERT_EQ(12, result.outputByteCount);
        ASSERT_EQ(3, result.outputCharCount);
    }

    //### - [ ] input every 1 elements
    {
        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Buf[0], 1, (byte_t*)&utf32Buf[0], 4, &result));
        ASSERT_EQ(g_utf32Buf[0], utf32Buf[0]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }

    //### - [ ] input every 1 elements
    {
        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Hokke[0], 1, (byte_t*)&utf32Buf[0], 4, &result));
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(0, result.outputByteCount);
        ASSERT_EQ(0, result.outputCharCount);

        ASSERT_EQ(true, encoder->convertFromUTF16(&g_utf16Hokke[1], 1, (byte_t*)&utf32Buf[0], 4, &result));
        ASSERT_EQ(g_utf32Hokke[0], utf32Buf[0]);
        ASSERT_EQ(1, result.usedElementCount);
        ASSERT_EQ(4, result.outputByteCount);
        ASSERT_EQ(1, result.outputCharCount);
    }
}
#endif
