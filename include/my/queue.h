#include "deque"
#include <algorithm>
#include <utility>
#include <memory>
#include <type_traits>
#include <iterator>

namespace my {
    template<typename C, typename T>
    concept QueueContainer = requires(C c, const C cc, T & val, T && rval) {
        typename C::value_type;
        typename C::reference;
        typename C::const_reference;
        typename C::size_type;
        requires std::same_as<typename C::value_type, T>;

        { cc.empty() } -> std::convertible_to<bool>;
        { cc.size() } -> std::convertible_to<typename C::size_type>;

        { c.front() } -> std::convertible_to<typename C::reference>;
        { cc.front() } -> std::convertible_to<typename C::const_reference>;
        { c.back() } -> std::convertible_to<typename C::reference>;
        { cc.back() } -> std::convertible_to<typename C::const_reference>;

        { c.push_back(val) } -> std::same_as<void>;
        { c.push_back(std::move(rval)) } -> std::same_as<void>;
        { c.pop_back() } -> std::same_as<void>;
        c.emplace_back(std::move(rval));

        c.swap(c);
    };

    template<typename T, typename Container = ::std::deque<T>>
    requires QueueContainer<Container, T>
    class queue {
    public:
        using container_type = Container;
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;
        using reference = typename Container::reference;
        using const_reference = typename Container::const_reference;

    protected:
        Container c;

    public:
        queue() : c() {}
        explicit queue(const Container& cont) : c(cont) {}
        explicit queue(Container&& cont) : c(std::move(cont)) {}
        queue(const queue&) = default;
        queue(queue&&) = default;
        template<std::input_iterator InputIt>
        requires std::is_constructible_v<Container, InputIt, InputIt>
        queue(InputIt first, InputIt last) : c(first, last) {}

        template<typename Alloc>
        requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, const Alloc&>)
        explicit queue(const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc)) {}

        template<typename Alloc>
        requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, const Container&, const Alloc&>)
        queue(const Container& cont, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, cont)) {}

        template<typename Alloc>
        requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, Container&&, const Alloc&>)
        queue(Container&& cont, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, std::move(cont))) {}

        template<typename Alloc>
        requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, const Container&, const Alloc&>)
        queue(const queue& other, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, other.c)) {}

        template<typename Alloc>
        requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, Container&&, const Alloc&>)
        queue(queue&& other, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, std::move(other.c))) {}

        template<typename InputIt, typename Alloc>
        requires (std::input_iterator<InputIt>&& std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, InputIt, InputIt>)
        queue(InputIt first, InputIt last, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, first, last)) {}

        ~queue() = default;

        queue& operator=(const queue&) = default;
        queue& operator=(queue&&) = default;

        reference front() { return c.front(); }
        const_reference front() const { return c.front(); }
        reference back() { return c.back(); }
        const_reference back() const { return c.back(); }
        bool empty() const { return c.empty(); }
        size_type size() const { return c.size(); }

        void push(const value_type& value) { c.push_back(value); }
        void push(value_type&& value) { c.push_back(std::move(value)); }
        template<class... Args>
        decltype(auto) emplace(Args&&... args) { return c.emplace_back(std::forward<Args>(args)...); }
        void pop() { c.pop_front(); }

        void swap(queue& other) noexcept(noexcept(std::swap(c, other.c))) { std::swap(c, other.c); }
    };

    template<typename T, typename Container>
    bool operator==(const queue<T, Container>& lhs, const queue<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<typename T, typename Container>
    bool operator!=(const queue<T, Container>& lhs, const queue<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<typename T, typename Container>
    bool operator<(const queue<T, Container>& lhs, const queue<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<typename T, typename Container>
    bool operator<=(const queue<T, Container>& lhs, const queue<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<typename T, typename Container>
    bool operator>(const queue<T, Container>& lhs, const queue<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<typename T, typename Container>
    bool operator>=(const queue<T, Container>& lhs, const queue<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<typename T, typename Container>
    void swap(queue<T, Container>& lhs, queue<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }
}