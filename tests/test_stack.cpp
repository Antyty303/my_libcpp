#include "my/stack.h"
#include <gtest/gtest.h>

#include "my/vector.h"
#include <vector>
#include <deque>
#include <list>
#include <string>

#include <type_traits>  //std::is_same_v
#include <utility>      //std::move

template <typename T>
T MakeValue(int i) {
    if constexpr (std::is_same_v<T, std::string>) {
        return std::to_string(i);
    }
    else {
        return T(i);
    }
}

using StackTypes = ::testing::Types<
    my::stack<int, std::deque<int>>,
    my::stack<int, std::vector<int>>,
    my::stack<int, my::vector<int>>,
    my::stack<int, std::list<int>>,

    my::stack<double, std::deque<double>>,
    my::stack<double, std::vector<double>>,
    my::stack<double, my::vector<double>>,
    my::stack<double, std::list<double>>,

    my::stack<std::string, std::deque<std::string>>,
    my::stack<std::string, std::vector<std::string>>,
    my::stack<std::string, my::vector<std::string>>,
    my::stack<std::string, std::list<std::string>>
>;

template <typename T>
class StackTests : public ::testing::Test {
protected:
    T stack;
};

TYPED_TEST_SUITE(StackTests, StackTypes);

// реярш
TYPED_TEST(StackTests, DefaultConstructor) {
    TypeParam s{};
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0);
}

TYPED_TEST(StackTests, ContainerCopyConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam s(c);
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(2));
}

TYPED_TEST(StackTests, ContainerMoveConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam s(std::move(c));
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(2));
}

TYPED_TEST(StackTests, IteratorConstructor) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam s(v.begin(), v.end());
    EXPECT_EQ(s.size(), 3);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(3));
}

TYPED_TEST(StackTests, AllocatorOnlyConstructor) {
    using ValueType = typename TypeParam::value_type;
    std::allocator<ValueType> alloc;

    TypeParam s(alloc);
    EXPECT_TRUE(s.empty());
}

TYPED_TEST(StackTests, ContainerAndAllocatorCopyConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;
    std::allocator<ValueType> alloc;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam s(c, alloc);
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(2));
}

TYPED_TEST(StackTests, ContainerAndAllocatorMoveConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;
    std::allocator<ValueType> alloc;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam s(std::move(c), alloc);
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(2));
}

TYPED_TEST(StackTests, CopyConstructor) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam s(v.begin(), v.end());

    TypeParam copy(s);
    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(copy.top(), MakeValue<ValueType>(3));
    EXPECT_EQ(s.size(), 3);
}

TYPED_TEST(StackTests, MoveConstructor) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam s(v.begin(), v.end());

    TypeParam move(std::move(s));
    EXPECT_EQ(move.size(), 3);
    EXPECT_EQ(move.top(), MakeValue<ValueType>(3));
}

TYPED_TEST(StackTests, Top) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam s(v.begin(), v.end());
   
    EXPECT_EQ(s.top(), MakeValue<ValueType>(3));
    s.top() = MakeValue<ValueType>(13);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(13));

    const TypeParam& cs = s;

    EXPECT_EQ(cs.top(), MakeValue<ValueType>(13));
}

TYPED_TEST(StackTests, PushLValue) {
    using ValueType = typename TypeParam::value_type;

    ValueType val = MakeValue<ValueType>(10);
    TypeParam s{};
    EXPECT_EQ(s.size(), 0);

    s.push(val);
    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s.top(), val);
}

TYPED_TEST(StackTests, PushRValue) {
    using ValueType = typename TypeParam::value_type;

    TypeParam s{};
    EXPECT_EQ(s.size(), 0);

    s.push(MakeValue<ValueType>(10));
    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(10));
}

TYPED_TEST(StackTests, Emplace) {
    using ValueType = typename TypeParam::value_type;

    TypeParam s{};

    if constexpr (std::is_same_v<ValueType, std::string>) {
        s.emplace(3, 'x');
        EXPECT_EQ(s.top(), "xxx");
    }
    else {
        auto& ref = s.emplace(1);
        EXPECT_EQ(ref, MakeValue<ValueType>(1));

        ref = MakeValue<ValueType>(2);
        EXPECT_EQ(s.top(), MakeValue<ValueType>(2));
    }
}

TYPED_TEST(StackTests, Pop) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
         MakeValue<ValueType>(1),
         MakeValue<ValueType>(2)
    };

    TypeParam s(v.begin(), v.end());
    EXPECT_EQ(s.size(), 2);

    s.pop();
    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s.top(), MakeValue<ValueType>(1));

    s.pop();
    EXPECT_TRUE(s.empty());
}

TYPED_TEST(StackTests, Swap) {
    using ValueType = typename TypeParam::value_type;

    TypeParam s1, s2;
    s1.push(MakeValue<ValueType>(1));
    s2.push(MakeValue<ValueType>(2));
    s2.push(MakeValue<ValueType>(3));

    std::swap(s1, s2);

    EXPECT_EQ(s1.size(), 2);
    EXPECT_EQ(s1.top(), MakeValue<ValueType>(3));

    EXPECT_EQ(s2.size(), 1);
    EXPECT_EQ(s2.top(), MakeValue<ValueType>(1));
}

TYPED_TEST(StackTests, Comparisons) {
    using ValueType = typename TypeParam::value_type;

    TypeParam s1, s2, s3, s4;

    s1.push(MakeValue<ValueType>(1));
    s2.push(MakeValue<ValueType>(1));
    s3.push(MakeValue<ValueType>(2));

    s4.push(MakeValue<ValueType>(1));
    s4.push(MakeValue<ValueType>(2));

    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 != s2);
    EXPECT_TRUE(s1 != s3);
    EXPECT_FALSE(s1 == s3);

    EXPECT_TRUE(s1 < s3); // [1] < [2]
    EXPECT_TRUE(s1 < s4); // [1] < [1, 2]
    EXPECT_TRUE(s1 <= s3);
    EXPECT_FALSE(s1 > s3);
    EXPECT_FALSE(s1 >= s3);

    EXPECT_TRUE(s3 > s1);
    EXPECT_TRUE(s3 >= s1);
    EXPECT_FALSE(s3 < s1);
    EXPECT_FALSE(s3 <= s1);
}