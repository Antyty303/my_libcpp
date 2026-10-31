#include "my/vector.h"
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ====================== Helper classes ===========================
// Allocator that tracks copies, moves, and propagation traits
template <typename T>
struct TrackingAllocator {
    using value_type = T;
    TrackingAllocator() = default;
    TrackingAllocator(int id) : id(id) {}
    int id = 0;

    T* allocate(std::size_t n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
    void deallocate(T* p, std::size_t) { ::operator delete(p); }

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    template <typename U> bool operator==(const TrackingAllocator<U>& other) const { return id == other.id; }
    template <typename U> bool operator!=(const TrackingAllocator<U>& other) const { return id != other.id; }
};

template<typename T>
struct NoPropMoveAlloc {
    using value_type = T;
    NoPropMoveAlloc() = default;
    NoPropMoveAlloc(int id) : id(id) {}
    int id = 0;

    T* allocate(std::size_t n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
    void deallocate(T* p, std::size_t) { ::operator delete(p); }

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::true_type;
    using is_always_equal = std::false_type;

    template<typename U> bool operator==(const NoPropMoveAlloc<U>& other) const { return id == other.id; }
    template<typename U> bool operator!=(const NoPropMoveAlloc<U>& other) const { return id != other.id; }
};

// Type that throws on copy/move after a certain count
static int throw_counter = 0;
static int throw_limit = -1;
struct ThrowOnCopy {
    int value;
    ThrowOnCopy(int v = 0) : value(v) {}
    ThrowOnCopy(const ThrowOnCopy& other) {
        if (throw_limit >= 0 && throw_counter++ >= throw_limit)
            throw std::runtime_error("ThrowOnCopy copy exception");
        value = other.value;
    }
    ThrowOnCopy(ThrowOnCopy&& other) noexcept(false) {
        if (throw_limit >= 0 && throw_counter++ >= throw_limit)
            throw std::runtime_error("ThrowOnCopy move exception");
        value = other.value;
        other.value = -1;
    }
    ThrowOnCopy& operator=(const ThrowOnCopy& other) {
        if (throw_limit >= 0 && throw_counter++ >= throw_limit)
            throw std::runtime_error("ThrowOnCopy copy assignment exception");
        value = other.value;
        return *this;
    }
    ThrowOnCopy& operator=(ThrowOnCopy&& other) noexcept(false) {
        if (throw_limit >= 0 && throw_counter++ >= throw_limit)
            throw std::runtime_error("ThrowOnCopy move assignment exception");
        value = other.value;
        other.value = -1;
        return *this;
    }
    ~ThrowOnCopy() = default;
};
bool operator==(const ThrowOnCopy& a, const ThrowOnCopy& b) { return a.value == b.value; }
bool operator!=(const ThrowOnCopy& a, const ThrowOnCopy& b) { return !(a == b); }

// Input iterator wrapper (not forward) that throws after some steps
class InputIt {
    int val_;
    int max_;
    int step_ = 0;
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int*;
    using reference = const int&;

    InputIt() : val_(0), max_(0) {}
    InputIt(int start, int count) : val_(start), max_(start + count) {}
    int operator*() const { return val_; }
    InputIt& operator++() {
        ++val_; ++step_; return *this;
    }
    InputIt operator++(int) { InputIt tmp = *this; ++(*this); return tmp; }
    bool operator==(const InputIt& other) const { return val_ == other.val_; }
    bool operator!=(const InputIt& other) const { return !(*this == other); }
};

// ====================== Tests ================================

TEST(VectorTests, DefaultConstructor) {
    my::vector<int> v;
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
    EXPECT_TRUE(v.empty());
    EXPECT_TRUE(v.begin() == v.end());
}

TEST(VectorTests, ConstructorWithAllocator) {
    TrackingAllocator<int> alloc(42);
    my::vector<int, TrackingAllocator<int>> v(alloc);
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
    EXPECT_EQ(v.get_allocator().id, 42);
}

TEST(VectorTests, ConstructorSize) {
    my::vector<int> v(5);
    EXPECT_EQ(v.size(), 5u);
    EXPECT_GE(v.capacity(), 5u);
    for (size_t i = 0; i < 5; ++i) EXPECT_EQ(v[i], 0);
}

TEST(VectorTests, ConstructorSizeAndValue) {
    my::vector<std::string> v(3, "hello");
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "hello");
    EXPECT_EQ(v[1], "hello");
    EXPECT_EQ(v[2], "hello");
}

TEST(VectorTests, ConstructorSizeAlloc) {
    TrackingAllocator<int> alloc(100);
    my::vector<int, TrackingAllocator<int>> v(5, alloc);
    EXPECT_EQ(v.size(), 5u);
    EXPECT_GE(v.capacity(), 5u);
    EXPECT_EQ(v.get_allocator().id, 100);
}

TEST(VectorTests, ConstructorForwardIterators) {
    std::vector<int> src = { 1, 2, 3, 4, 5 };
    my::vector<int> v(src.begin(), src.end());
    EXPECT_EQ(v.size(), 5u);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(v[i], i + 1);
}

TEST(VectorTests, ConstructorInputIterators) {
    InputIt begin(10, 4);
    InputIt end(14, 0);
    my::vector<int> v(begin, end);
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 11);
    EXPECT_EQ(v[2], 12);
    EXPECT_EQ(v[3], 13);
}

TEST(VectorTests, ConstructorInitializerList) {
    my::vector<double> v = { 1.1, 2.2, 3.3 };
    EXPECT_EQ(v.size(), 3u);
    EXPECT_DOUBLE_EQ(v[0], 1.1);
    EXPECT_DOUBLE_EQ(v[2], 3.3);
}

TEST(VectorTests, CopyConstructor) {
    my::vector<int> original = { 7, 8, 9 };
    my::vector<int> copy(original);
    EXPECT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy[0], 7);
    EXPECT_EQ(copy[1], 8);
    EXPECT_EQ(copy[2], 9);
    copy[0] = 100;
    EXPECT_EQ(original[0], 7);
}

TEST(VectorTests, MoveConstructor) {
    my::vector<int> original = { 1, 2, 3 };
    int* old_data = original.data();
    size_t old_size = original.size();
    my::vector<int> moved(std::move(original));
    EXPECT_EQ(moved.data(), old_data);
    EXPECT_EQ(moved.size(), old_size);
    EXPECT_TRUE(original.empty());
    EXPECT_EQ(original.data(), nullptr);
    EXPECT_EQ(original.size(), 0u);
}

TEST(VectorTests, CopyWithAllocator) {
    TrackingAllocator<int> alloc(77);
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec original = { 10, 20 };
    Vec copy(original, alloc);
    EXPECT_EQ(copy.size(), 2u);
    EXPECT_EQ(copy.get_allocator().id, 77);
    EXPECT_NE(original.get_allocator().id, 77);
}

TEST(VectorTests, MoveWithAllocator) {
    TrackingAllocator<int> alloc1(1), alloc2(2);
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec original({ 1, 2, 3 }, alloc1);

    int* old_data = original.data();
    Vec moved(std::move(original), alloc2);
    EXPECT_EQ(moved.size(), 3u);
    EXPECT_EQ(moved.get_allocator().id, 2);
    EXPECT_NE(moved.data(), old_data);
    EXPECT_TRUE(original.empty());
}

TEST(VectorTests, MoveWithSameAllocator) {
    TrackingAllocator<int> alloc1(1);
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec original({ 1, 2, 3 }, alloc1);

    int* old_data = original.data();
    Vec moved(std::move(original), alloc1);

    EXPECT_EQ(moved.size(), 3u);
    EXPECT_EQ(moved.get_allocator().id, 1);
    EXPECT_EQ(moved.data(), old_data);
    EXPECT_TRUE(original.empty());
}

TEST(VectorTests, AssignCount) {
    my::vector<int> v;
    v.assign(4, 42);
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 42);
    EXPECT_EQ(v[3], 42);
    v.assign(0, 99);
    EXPECT_TRUE(v.empty());
}

TEST(VectorTests, AssignForwardIterators) {
    my::vector<int> v;
    std::vector<int> src = { 10, 20, 30 };
    v.assign(src.begin(), src.end());
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[2], 30);
}

TEST(VectorTests, AssignInputIterators) {
    my::vector<int> v;
    InputIt begin(5, 3);
    InputIt end(8, 0);
    v.assign(begin, end);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 5);
    EXPECT_EQ(v[1], 6);
    EXPECT_EQ(v[2], 7);
}

TEST(VectorTests, AssignInitializerList) {
    my::vector<int> v;
    v.assign({ 100, 200, 300 });
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 100);
    EXPECT_EQ(v[2], 300);
}

TEST(VectorTests, CopyAssignment) {
    my::vector<int> v1 = { 1, 2, 3 };
    my::vector<int> v2 = { 4, 5 };
    v2 = v1;
    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v1.size(), 3u);
    v1 = v1;
    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1[0], 1);
}

TEST(VectorTests, CopyMethod) {
    my::vector<int> v1 = { 1, 2, 3 };
    my::vector<int> v2 = { 4, 5 };
    v2.copy(v1);
    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v1.size(), 3u);

    v1.copy(v1);
    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1[0], 1);
}

TEST(VectorTests, MoveAssignment) {
    my::vector<int> v1 = { 10, 20, 30 };
    my::vector<int> v2;
    v2 = std::move(v1);
    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[0], 10);
    EXPECT_TRUE(v1.empty());

    v2 = std::move(v2);
    EXPECT_EQ(v2.size(), 3u);
}

TEST(VectorTests, MoveAssignmentNoPropagateSameAlloc) {
    using Vec = my::vector<int, NoPropMoveAlloc<int>>;
    NoPropMoveAlloc<int> alloc(10);
    Vec v1({ 1, 2, 3 }, alloc);
    Vec v2({ 4, 5 }, alloc);

    int* old_v1_data = v1.data();
    v2 = std::move(v1);

    EXPECT_EQ(v2.get_allocator().id, 10);
    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2.data(), old_v1_data);
    EXPECT_TRUE(v1.empty());
}

TEST(VectorTests, MoveAssignmentNoPropagateDifferentAlloc) {
    using Vec = my::vector<int, NoPropMoveAlloc<int>>;
    Vec v1({ 1, 2, 3 }, NoPropMoveAlloc<int>(10));
    Vec v2({ 4, 5 }, NoPropMoveAlloc<int>(20));

    int* old_v1_data = v1.data();
    v2 = std::move(v1);

    EXPECT_EQ(v2.get_allocator().id, 20);
    EXPECT_EQ(v2.size(), 3u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_NE(v2.data(), old_v1_data);
    EXPECT_TRUE(v1.empty());
}

TEST(VectorTests, AssignmentInitializerList) {
    my::vector<int> v;
    v = { 7, 8, 9 };
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 7);
    v = {};
    EXPECT_TRUE(v.empty());
}


TEST(VectorTests, ElementAccess) {
    my::vector<int> v = { 10, 20, 30 };
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v.at(1), 20);
    EXPECT_EQ(v.front(), 10);
    EXPECT_EQ(v.back(), 30);
    const auto& cv = v;
    EXPECT_EQ(cv[2], 30);
    EXPECT_EQ(cv.at(0), 10);
    EXPECT_EQ(cv.front(), 10);
    EXPECT_EQ(cv.back(), 30);

    EXPECT_THROW(v.at(100), std::out_of_range);

    my::vector<int> empty_vec;
    EXPECT_THROW(empty_vec.front(), std::out_of_range);
    EXPECT_THROW(empty_vec.back(), std::out_of_range);
}

TEST(VectorTests, DataAccess) {
    my::vector<int> v = { 1, 2, 3 };
    int* p = v.data();
    EXPECT_EQ(p[0], 1);
    EXPECT_EQ(p[1], 2);
    const auto& cv = v;
    const int* cp = cv.data();
    EXPECT_EQ(cp[2], 3);
}

TEST(VectorTests, ResizeDefault) {
    my::vector<int> v = { 1, 2, 3 };
    v.resize(5);
    EXPECT_EQ(v.size(), 5u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[3], 0);
    EXPECT_EQ(v[4], 0);
    v.resize(2);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    v.resize(0);
    EXPECT_TRUE(v.empty());
}

TEST(VectorTests, ResizeWithValue) {
    my::vector<std::string> v;
    v.resize(3, "yes");
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "yes");
    v.resize(1, "no");
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], "yes");
}

TEST(VectorTests, ReserveAndShrink) {
    my::vector<int> v;
    v.reserve(100);
    EXPECT_GE(v.capacity(), 100u);
    size_t cap = v.capacity();
    v.push_back(1);
    EXPECT_EQ(v.capacity(), cap);
    v.shrink_to_fit();
    EXPECT_EQ(v.capacity(), v.size());
}

TEST(VectorTests, Clear) {
    my::vector<int> v = { 1, 2, 3 };
    v.clear();
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
    v.clear();
}

TEST(VectorTests, Swap) {
    my::vector<int> a = { 1, 2, 3 };
    my::vector<int> b = { 4, 5 };
    int* a_data = a.data();
    int* b_data = b.data();
    a.swap(b);
    EXPECT_EQ(a.size(), 2u);
    EXPECT_EQ(b.size(), 3u);
    EXPECT_EQ(a.data(), b_data);
    EXPECT_EQ(b.data(), a_data);
}

TEST(VectorTests, PushBack) {
    my::vector<int> v;
    v.push_back(10);
    EXPECT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 10);
    const int x = 20;
    v.push_back(x);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[1], 20);
    v.push_back(30);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[2], 30);
}

TEST(VectorTests, PopBack) {
    my::vector<int> v = { 1, 2, 3 };
    v.pop_back();
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    v.pop_back();
    v.pop_back();
    EXPECT_TRUE(v.empty());

    EXPECT_THROW(v.pop_back(), std::out_of_range);
}

TEST(VectorTests, InsertSingleElement) {
    my::vector<int> v = { 10, 20, 30 };
    auto it = v.insert(v.begin() + 1, 15);
    EXPECT_EQ(*it, 15);
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 15);
    EXPECT_EQ(v[2], 20);
    EXPECT_EQ(v[3], 30);

    it = v.insert(v.end(), 40);
    EXPECT_EQ(*it, 40);
    EXPECT_EQ(v.size(), 5u);
    EXPECT_EQ(v[4], 40);

    it = v.insert(v.begin(), 5);
    EXPECT_EQ(*it, 5);
    EXPECT_EQ(v[0], 5);
}

TEST(VectorTests, InsertMoveElement) {
    my::vector<std::string> v = { "a", "b" };
    std::string s = "c";
    v.insert(v.begin() + 1, std::move(s));
    EXPECT_EQ(v[1], "c");
    EXPECT_TRUE(s.empty());
}

TEST(VectorTests, InsertCount) {
    my::vector<int> v = { 1, 2, 3 };
    auto it = v.insert(v.begin() + 2, 3, 99);
    EXPECT_EQ(*it, 99);
    EXPECT_EQ(v.size(), 6u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 99);
    EXPECT_EQ(v[3], 99);
    EXPECT_EQ(v[4], 99);
    EXPECT_EQ(v[5], 3);
}

TEST(VectorTests, InsertForwardRange) {
    my::vector<int> v = { 1, 5 };
    std::vector<int> src = { 2, 3, 4 };
    auto it = v.insert(v.begin() + 1, src.begin(), src.end());
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(v.size(), 5u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 5);
}

TEST(VectorTests, InsertInputRange) {
    my::vector<int> v = { 10, 40 };
    InputIt begin(20, 2);
    InputIt end(22, 0);
    auto it = v.insert(v.begin() + 1, begin, end);
    EXPECT_EQ(*it, 20);
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 21);
    EXPECT_EQ(v[3], 40);
}

TEST(VectorTests, InsertInitializerList) {
    my::vector<int> v = { 100, 200 };
    v.insert(v.end(), { 300, 400 });
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[2], 300);
    EXPECT_EQ(v[3], 400);
}

TEST(VectorTests, EraseSingle) {
    my::vector<int> v = { 1, 2, 3, 4 };
    auto it = v.erase(v.begin() + 1);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
    EXPECT_EQ(v[2], 4);

    it = v.erase(v.end() - 1);
    EXPECT_TRUE(it == v.end());
    EXPECT_EQ(v.size(), 2u);
}

TEST(VectorTests, EraseCount) {
    my::vector<int> v = { 1, 2, 3, 4, 5 };
    auto it = v.erase(v.begin() + 1, 2);
    EXPECT_EQ(*it, 4);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 5);

    it = v.erase(v.end() - 2, 2);
    EXPECT_TRUE(it == v.end());
    EXPECT_EQ(v.size(), 1u);
}

TEST(VectorTests, EraseRange) {
    my::vector<int> v = { 1, 2, 3, 4, 5 };
    auto it = v.erase(v.begin() + 1, v.begin() + 4);
    EXPECT_EQ(*it, 5);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 5);

    it = v.erase(v.begin(), v.begin());
    EXPECT_TRUE(it == v.begin());
    EXPECT_EQ(v.size(), 2u);
}

TEST(VectorTests, EmplaceBack) {
    my::vector<std::pair<int, int>> v;
    auto& ref = v.emplace_back(1, 2);
    EXPECT_EQ(ref.first, 1);
    EXPECT_EQ(ref.second, 2);
    EXPECT_EQ(v.size(), 1u);
    v.emplace_back(3, 4);
    EXPECT_EQ(v[1].first, 3);
}

TEST(VectorTests, Emplace) {
    my::vector<std::string> v = { "hello", "world" };
    auto it = v.emplace(v.begin() + 1, "beautiful");
    EXPECT_EQ(*it, "beautiful");
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], "hello");
    EXPECT_EQ(v[1], "beautiful");
    EXPECT_EQ(v[2], "world");

    it = v.emplace(v.end(), "!");
    EXPECT_EQ(*it, "!");
    EXPECT_EQ(v.size(), 4u);
}

TEST(VectorTests, ComparisonOperators) {
    my::vector<int> a = { 1, 2, 3 };
    my::vector<int> b = { 1, 2, 3 };
    my::vector<int> c = { 1, 2, 4 };
    my::vector<int> d = { 1, 2 };
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a != c);
    EXPECT_TRUE(a < c);
    EXPECT_TRUE(c > a);
    EXPECT_TRUE(a > d);
    EXPECT_TRUE(d < a);
    EXPECT_TRUE(a >= b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(c >= a);
    EXPECT_TRUE(d <= a);
}

// Allocator propagation tests
TEST(VectorTests, AllocatorPropagateOnCopy) {
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec v1(5, TrackingAllocator<int>(1));
    v1 = { 1, 2, 3, 4, 5 };
    Vec v2(TrackingAllocator<int>(2));
    v2 = v1;
    EXPECT_EQ(v2.get_allocator().id, 1);
    EXPECT_EQ(v2.size(), 5u);
}

TEST(VectorTests, AllocatorMoveAssignPropagate) {
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec v1(5, TrackingAllocator<int>(1));
    v1 = { 1, 2, 3, 4, 5 };
    Vec v2(TrackingAllocator<int>(2));
    v2 = std::move(v1);
    EXPECT_EQ(v2.get_allocator().id, 1);
}

TEST(VectorTests, AllocatorSwap) {
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec v1(5, TrackingAllocator<int>(10));
    Vec v2(3, TrackingAllocator<int>(20));
    v1 = { 1, 2, 3, 4, 5 };
    v2 = { 6, 7, 8 };
    v1.swap(v2);
    EXPECT_EQ(v1.get_allocator().id, 20);
    EXPECT_EQ(v2.get_allocator().id, 10);
}

TEST(VectorTests, AllocatorSelectOnCopyConstruct) {
    using Vec = my::vector<int, TrackingAllocator<int>>;
    Vec v1(3, 7, TrackingAllocator<int>(99));
    Vec v2(v1);
    EXPECT_EQ(v2.get_allocator().id, 99);
}

TEST(VectorTests, ExceptionSafetyReserve) {
    throw_counter = 0;
    throw_limit = 3;
    my::vector<ThrowOnCopy> v;
    v.push_back(ThrowOnCopy(10));
    v.push_back(ThrowOnCopy(20));
    size_t old_cap = v.capacity();

    EXPECT_THROW(v.reserve(100), std::runtime_error);

    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v.capacity(), old_cap);
    EXPECT_EQ(v[0].value, 10);
    EXPECT_EQ(v[1].value, 20);
}

TEST(VectorTests, ExceptionSafetyOperatorCopy) {
    using Vec = my::vector<ThrowOnCopy>;
    Vec v1; v1.reserve(4);
    v1.emplace_back(1);
    v1.emplace_back(2);
    v1.emplace_back(3);

    Vec v2; v2.reserve(3);
    v2.emplace_back(4);
    v2.emplace_back(5);
    size_t old_size2 = v2.size();
    size_t old_cap2 = v2.capacity();

    throw_counter = 0;
    throw_limit = 2;

    EXPECT_THROW(v2 = v1, std::runtime_error);

    EXPECT_EQ(v2.size(), old_size2);
    EXPECT_EQ(v2.capacity(), old_cap2);
    EXPECT_EQ(v2[0].value, 4);
    EXPECT_EQ(v2[1].value, 5);

    EXPECT_EQ(v1.size(), 3u);
    EXPECT_EQ(v1[0].value, 1);
    EXPECT_EQ(v1[1].value, 2);
    EXPECT_EQ(v1[2].value, 3);
}

// Iterator tests in debug mode
#ifndef NDEBUG
TEST(VectorTests, DebugIteratorCreate) {
    my::vector<int> v = { 1, 2, 3 };
    auto it = v.begin();
    EXPECT_EQ(*it, 1);
    auto it2 = v.end();
    EXPECT_EQ(it2 - it, 3);
}

TEST(VectorTests, DebugIteratorCopyMove) {
    my::vector<int> v = { 10, 20 };
    auto it1 = v.begin();
    auto it2(it1);
    EXPECT_EQ(*it2, 10);
    auto it3(std::move(it1));
    EXPECT_EQ(*it3, 10);

    EXPECT_THROW(*it1, std::invalid_argument);
}

TEST(VectorTests, DebugIteratorInvalidationOnReserve) {
    my::vector<int> v = { 1, 2, 3 };
    auto it = v.begin() + 1;
    v.reserve(100);
    EXPECT_THROW(*it, std::invalid_argument);
}

TEST(VectorTests, DebugIteratorInvalidationOnPushBack) {
    my::vector<int> v;
    v.reserve(2);
    v.push_back(1);
    v.push_back(2);
    auto it = v.begin();
    v.push_back(3);
    EXPECT_THROW(*it, std::invalid_argument);

    my::vector<int> v2;
    v2.reserve(2);
    v2.push_back(1);
    auto it2 = v2.end();
    v2.push_back(2);
    EXPECT_THROW(*it2, std::invalid_argument);
}

TEST(VectorTests, DebugIteratorInvalidationOnSwap) {
    my::vector<int> a = { 1, 2 };
    my::vector<int> b = { 3, 4 };
    auto it_a = a.begin();
    auto it_b = b.begin();
    a.swap(b);

    EXPECT_EQ(*it_a, 1);
    EXPECT_EQ(it_a - b.begin(), 0);
    EXPECT_EQ(*it_b, 3);
    EXPECT_EQ(it_b - a.begin(), 0);
}

TEST(VectorTests, DebugIteratorSeek) {
    my::vector<int> v = { 0, 1, 2, 3 };
    auto it = v.begin();
    it += 2;
    EXPECT_EQ(*it, 2);
    it -= 1;
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    --it;
    EXPECT_EQ(*it, 1);
    auto it2 = it + 2;
    EXPECT_EQ(*it2, 3);
    auto diff = it2 - it;
    EXPECT_EQ(diff, 2);
}

TEST(VectorTests, DebugIteratorComparisons) {
    my::vector<int> v = { 5, 6, 7 };
    auto a = v.begin();
    auto b = v.begin() + 1;
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
    EXPECT_TRUE(a != b);
    EXPECT_TRUE(a == a);
}

TEST(VectorTests, DebugIteratorOutOfRange) {
    my::vector<int> v = { 1, 2 };
    auto it = v.end();
    EXPECT_THROW(*it, std::out_of_range);
    EXPECT_THROW(it[1], std::out_of_range);
}
#endif