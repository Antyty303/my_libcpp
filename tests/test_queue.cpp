#include "my/queue.h"
#include <gtest/gtest.h>

#include "my/vector.h"
#include <vector>
#include <deque>
#include <list>
#include <string>

#include <type_traits>
#include <utility>

template <typename T>
T MakeValue(int i) {
    if constexpr (std::is_same_v<T, std::string>) {
        return std::to_string(i);
    }
    else {
        return T(i);
    }
}

using QueueTypes = ::testing::Types<
    my::queue<int, std::deque<int>>,
    my::queue<int, std::list<int>>,

    my::queue<double, std::deque<double>>,
    my::queue<double, std::list<double>>,

    my::queue<std::string, std::deque<std::string>>,
    my::queue<std::string, std::list<std::string>>
>;

template <typename T>
class QueueTests : public ::testing::Test {
protected:
    T queue;
};

TYPED_TEST_SUITE(QueueTests, QueueTypes);

// реярш
TYPED_TEST(QueueTests, DefaultConstructor) {
    TypeParam q{};
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TYPED_TEST(QueueTests, ContainerCopyConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam q(c);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(q.back(), MakeValue<ValueType>(2));
}

TYPED_TEST(QueueTests, ContainerMoveConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam q(std::move(c));
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(q.back(), MakeValue<ValueType>(2));
}

TYPED_TEST(QueueTests, IteratorConstructor) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam q(v.begin(), v.end());
    EXPECT_EQ(q.size(), 3);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(q.back(), MakeValue<ValueType>(3));
}

TYPED_TEST(QueueTests, AllocatorOnlyConstructor) {
    using ValueType = typename TypeParam::value_type;
    std::allocator<ValueType> alloc;

    TypeParam q(alloc);
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(QueueTests, ContainerAndAllocatorCopyConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;
    std::allocator<ValueType> alloc;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam q(c, alloc);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(q.back(), MakeValue<ValueType>(2));
}

TYPED_TEST(QueueTests, ContainerAndAllocatorMoveConstructor) {
    using Container = typename TypeParam::container_type;
    using ValueType = typename TypeParam::value_type;
    std::allocator<ValueType> alloc;

    Container c;
    c.push_back(MakeValue<ValueType>(1));
    c.push_back(MakeValue<ValueType>(2));

    TypeParam q(std::move(c), alloc);
    EXPECT_EQ(q.size(), 2);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(q.back(), MakeValue<ValueType>(2));
}

TYPED_TEST(QueueTests, CopyConstructor) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam q(v.begin(), v.end());

    TypeParam copy(q);
    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(copy.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(copy.back(), MakeValue<ValueType>(3));
    EXPECT_EQ(q.size(), 3);
}

TYPED_TEST(QueueTests, MoveConstructor) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam q(v.begin(), v.end());

    TypeParam move(std::move(q));
    EXPECT_EQ(move.size(), 3);
    EXPECT_EQ(move.front(), MakeValue<ValueType>(1));
    EXPECT_EQ(move.back(), MakeValue<ValueType>(3));
}

TYPED_TEST(QueueTests, Front) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam q(v.begin(), v.end());

    EXPECT_EQ(q.front(), MakeValue<ValueType>(1));
    q.front() = MakeValue<ValueType>(13);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(13));

    const TypeParam& cq = q;

    EXPECT_EQ(cq.front(), MakeValue<ValueType>(13));
}

TYPED_TEST(QueueTests, Back) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
        MakeValue<ValueType>(1),
        MakeValue<ValueType>(2),
        MakeValue<ValueType>(3)
    };

    TypeParam q(v.begin(), v.end());

    EXPECT_EQ(q.back(), MakeValue<ValueType>(3));
    q.back() = MakeValue<ValueType>(13);
    EXPECT_EQ(q.back(), MakeValue<ValueType>(13));

    const TypeParam& cq = q;

    EXPECT_EQ(cq.back(), MakeValue<ValueType>(13));
}


TYPED_TEST(QueueTests, PushLValue) {
    using ValueType = typename TypeParam::value_type;

    ValueType val = MakeValue<ValueType>(10);
    TypeParam q{};
    EXPECT_EQ(q.size(), 0);

    q.push(val);
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.back(), val);
}

TYPED_TEST(QueueTests, PushRValue) {
    using ValueType = typename TypeParam::value_type;

    TypeParam q{};
    EXPECT_EQ(q.size(), 0);

    q.push(MakeValue<ValueType>(10));
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.back(), MakeValue<ValueType>(10));
}

TYPED_TEST(QueueTests, Emplace) {
    using ValueType = typename TypeParam::value_type;

    TypeParam q{};

    if constexpr (std::is_same_v<ValueType, std::string>) {
        q.emplace(3, 'x');
        EXPECT_EQ(q.back(), "xxx");
    }
    else {
        auto& ref = q.emplace(1);
        EXPECT_EQ(ref, MakeValue<ValueType>(1));

        ref = MakeValue<ValueType>(2);
        EXPECT_EQ(q.back(), MakeValue<ValueType>(2));
    }
}

TYPED_TEST(QueueTests, Pop) {
    using ValueType = typename TypeParam::value_type;

    std::vector<ValueType> v = {
         MakeValue<ValueType>(1),
         MakeValue<ValueType>(2)
    };

    TypeParam q(v.begin(), v.end());
    EXPECT_EQ(q.size(), 2);

    q.pop();
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.front(), MakeValue<ValueType>(2));

    q.pop();
    EXPECT_TRUE(q.empty());
}

TYPED_TEST(QueueTests, Swap) {
    using ValueType = typename TypeParam::value_type;

    TypeParam q1, q2;
    q1.push(MakeValue<ValueType>(1));
    q2.push(MakeValue<ValueType>(2));
    q2.push(MakeValue<ValueType>(3));

    std::swap(q1, q2);

    EXPECT_EQ(q1.size(), 2);
    EXPECT_EQ(q1.front(), MakeValue<ValueType>(2));
    EXPECT_EQ(q1.back(), MakeValue<ValueType>(3));

    EXPECT_EQ(q2.size(), 1);
    EXPECT_EQ(q2.front(), MakeValue<ValueType>(1));
}

TYPED_TEST(QueueTests, Comparisons) {
    using ValueType = typename TypeParam::value_type;

    TypeParam q1, q2, q3, q4;

    q1.push(MakeValue<ValueType>(1));
    q2.push(MakeValue<ValueType>(1));
    q3.push(MakeValue<ValueType>(2));

    q4.push(MakeValue<ValueType>(1));
    q4.push(MakeValue<ValueType>(2));

    EXPECT_TRUE(q1 == q2);
    EXPECT_FALSE(q1 != q2);
    EXPECT_TRUE(q1 != q3);
    EXPECT_FALSE(q1 == q3);

    EXPECT_TRUE(q1 < q3); // [1] < [2]
    EXPECT_TRUE(q1 < q4); // [1] < [1, 2]
    EXPECT_TRUE(q1 <= q3);
    EXPECT_FALSE(q1 > q3);
    EXPECT_FALSE(q1 >= q3);

    EXPECT_TRUE(q3 > q1);
    EXPECT_TRUE(q3 >= q1);
    EXPECT_FALSE(q3 < q1);
    EXPECT_FALSE(q3 <= q1);
}