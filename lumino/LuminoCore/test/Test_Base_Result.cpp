﻿#include "Common.hpp"
#include <LuminoCore/Base/Result.hpp>
#include <LuminoCore/IO/FileSystem.hpp>

//==============================================================================

class Test_Base_Result : public ::testing::Test {};

struct TestErr1 {
    enum class Kind {
        Invalid,
        TooLong,
    };

    Kind kind;
    std::string text;
};

namespace ln {
template<>
static String toString<TestErr1>(const TestErr1& e) {
    return ln::format(U"kind:{},text:{}", static_cast<int>(e.kind), String::fromStdString(e.text));
}
} // namespace ln 

static BasicResult<std::string, TestErr1> testFunc1(int len) {
    if (len > 10) return err(TestErr1{TestErr1::Kind::TooLong, "error."});
    if (len == 2) return "aaa"; 
    return ok(std::string("123"));
}

static BasicResult<void, int> testFunc2(int len) {
    if (len > 10) return err(1);
    return ok();
}

static BasicResult<void> testFunc3(int len) {
    if (len > 10) return err();
    return ok();
}

static std::string g_str1 = "v1";
static std::string g_str12 = "aaa";
static BasicResult<std::string&, TestErr1> testFunc4(int len) {
    if (len > 10) return err(TestErr1{TestErr1::Kind::TooLong, "error."});
    if (len == 2) return g_str12; 
    return ok<std::string&>(g_str1);
}

static std::string g_str2 = "v1";
static std::string g_str22 = "aaa";
static BasicResult<const std::string&, TestErr1> testFunc5(int len) {
    if (len > 10) return err(TestErr1{TestErr1::Kind::TooLong, "error."});
    if (len == 2) return g_str22; 
    return ok<const std::string&>(g_str2);
}

static BasicResult<int, TestErr1> testFunc6(int len) {
    if (len > 10) return err();
    return 1;
}

void func1() {
    BasicResult a(ok());
}
//
//static auto testFunc7(int len) -> BasicResult  {
//    if (len > 10) return err();
//    return ok();
//}
//
//template<typename T = int>
//class Foo {};
//
//int maxxxin() {
//    Foo f;
//}
//
//Foo maxxxin2() {
//    return Foo();
//}

TEST_F(Test_Base_Result, Basic) {
    auto r1 = testFunc1(1);
    ASSERT_EQ(true, r1.isOk());
    ASSERT_EQ(false, r1.isErr());
    ASSERT_EQ("123", r1.unwrap());
    bool r1r = !r1;
    ASSERT_EQ(false, r1r);

    auto r2 = testFunc1(100);
    ASSERT_EQ(false, r2.isOk());
    ASSERT_EQ(true, r2.isErr());
    ASSERT_EQ("error.", r2.unwrapErr().text);
    bool r2r = !r2;
    ASSERT_EQ(true, r2r);

    auto r3 = testFunc1(2);
    ASSERT_EQ(true, r3.isOk());
    ASSERT_EQ(false, r3.isErr());
    ASSERT_EQ("aaa", r3.unwrap());

	LN_LOG_WARNING(r3);
}

TEST_F(Test_Base_Result, VoidOk) {
    auto r1 = testFunc2(1);
    ASSERT_EQ(true, r1.isOk());
    ASSERT_EQ(false, r1.isErr());

    auto r2 = testFunc2(100);
    ASSERT_EQ(false, r2.isOk());
    ASSERT_EQ(true, r2.isErr());
}

TEST_F(Test_Base_Result, Void2) {
    auto r1 = testFunc3(1);
    ASSERT_EQ(true, r1.isOk());
    ASSERT_EQ(false, r1.isErr());

    auto r2 = testFunc3(100);
    ASSERT_EQ(false, r2.isOk());
    ASSERT_EQ(true, r2.isErr());
}

TEST_F(Test_Base_Result, Reference1) {
    auto r1 = testFunc4(1);
    ASSERT_EQ(true, r1.isOk());
    ASSERT_EQ(false, r1.isErr());
    ASSERT_EQ("v1", r1.unwrap());
    r1.unwrap() = "v2";

    auto r4 = testFunc4(1);
    ASSERT_EQ("v2", r4.unwrap());

    auto r2 = testFunc4(100);
    ASSERT_EQ(false, r2.isOk());
    ASSERT_EQ(true, r2.isErr());

    auto r3 = testFunc4(2);
    ASSERT_EQ(true, r3.isOk());
    ASSERT_EQ(false, r3.isErr());
    ASSERT_EQ("aaa", r3.unwrap());
}

TEST_F(Test_Base_Result, Reference2) {
    auto r1 = testFunc5(1);
    ASSERT_EQ(true, r1.isOk());
    ASSERT_EQ(false, r1.isErr());
    ASSERT_EQ("v1", r1.unwrap());

    auto r2 = testFunc5(100);
    ASSERT_EQ(false, r2.isOk());
    ASSERT_EQ(true, r2.isErr());

    auto r3 = testFunc5(2);
    ASSERT_EQ(true, r3.isOk());
    ASSERT_EQ(false, r3.isErr());
    ASSERT_EQ("aaa", r3.unwrap());
}

TEST_F(Test_Base_Result, DefaultError) {
    auto r2 = testFunc6(100);
    ASSERT_EQ(false, r2.isOk());
    ASSERT_EQ(true, r2.isErr());
}

using TestResultA = BasicResult<void, String>;
TestResultA testFunc7() {
    return err(U"abc");
}

Result<> testFunc8(int a) {
    if (a == 1) {
        // Value -> void
        auto r1 = FileSystem::readAllText(U"aaa");
        if (!r1) return r1;
    }
    if (a == 2) {
        // void -> void
        TestResultA r2 = testFunc7();
        if (!r2) return r2;
    }

    //Result<> r3 = link(r2.unwrap());
    //if (!r3) return err("link error.", r3);

    return ok();
}

TEST_F(Test_Base_Result, Box1) {
    auto r1 = testFunc8(1);
    auto s1 = r1.toString();

    //auto r1 = FileSystem::read
    auto r2 = testFunc7();
}

// ok型の無い Result に unwrap() を呼び出してもコンパイルエラーにならないこと。
TEST_F(Test_Base_Result, VoidUnwrap) {
    struct Test {
        static ln::IOResult<> func() { return ok(); }
    };
    Test::func().unwrap();
}

// RefObject を Box してもコンパイルエラーにならないこと。
TEST_F(Test_Base_Result, BoxRefObject) {
    class TestObject1 : public RefObject {

    };

    struct Local {
        static IOResult<Ref<TestObject1>> open1() {
            return ok(makeRef<TestObject1>());
        }
        static BasicResult<Ref<TestObject1>> open2() {
            return open1();
        }
    };
}

// 右辺値 operator* で値を戻したときにコンパイルエラーにならないこと。
TEST_F(Test_Base_Result, MoveReference) {
    struct Test {
        static IOResult<String> func() { return U"abc"; }
    };
	auto str = *Test::func();
	ASSERT_EQ(U"abc", str);
}
