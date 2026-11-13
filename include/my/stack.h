#include "my/vector.h"
#include <algorithm>
#include <utility>
#include <memory>
#include <type_traits>
#include <iterator>

namespace my {
    template<typename T, typename Container = ::my::vector<T>>
    class stack {
    public:
        using container_type = Container;
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;
        using reference = typename Container::reference;
        using const_reference = typename Container::const_reference;

    protected:
        Container c;

    public:
        stack() : c() {}
        explicit stack(const Container& cont) : c(cont) {}
        explicit stack(Container&& cont) : c(std::move(cont)) {}
        stack(const stack&) = default;
        stack(stack&&) = default;
        template<std::input_iterator InputIt>
            requires std::is_constructible_v<Container, InputIt, InputIt>
        stack(InputIt first, InputIt last) : c(first, last) {}

        template<typename Alloc>
            requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, const Alloc&>)
        explicit stack(const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc)) {}

        template<typename Alloc>
            requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, const Container&, const Alloc&>)
        stack(const Container& cont, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, cont)) {}

        template<typename Alloc>
            requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, Container&&, const Alloc&>)
        stack(Container&& cont, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, std::move(cont))) {}

        template<typename Alloc>
            requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, const Container&, const Alloc&>)
        stack(const stack& other, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, other.c)) {}

        template<typename Alloc>
            requires (std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, Container&&, const Alloc&>)
        stack(stack&& other, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, std::move(other.c))) {}

        template<typename InputIt, typename Alloc>
            requires (std::input_iterator<InputIt>&& std::uses_allocator_v<Container, Alloc>&& std::is_constructible_v<Container, InputIt, InputIt>)
        stack(InputIt first, InputIt last, const Alloc& alloc) : c(std::make_obj_using_allocator<Container>(alloc, first, last)) {}

        ~stack() = default;

        stack& operator=(const stack&) = default;
        stack& operator=(stack&&) = default;

        reference top() { return c.back(); }
        const_reference top() const { return c.back(); }
        bool empty() const { return c.empty(); }
        size_type size() const { return c.size(); }

        void push(const value_type& value) { c.push_back(value); }
        void push(value_type&& value) { c.push_back(std::move(value)); }
        template<class... Args>
        decltype(auto) emplace(Args&&... args) { return c.emplace_back(std::forward<Args>(args)...); }
        void pop() { c.pop_back(); }

        void swap(stack& other) noexcept(noexcept(std::swap(c, other.c))) { std::swap(c, other.c); }
    };

    template<class T, class Container>
    bool operator==(const stack<T, Container>& lhs, const stack<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<class T, class Container>
    bool operator!=(const stack<T, Container>& lhs, const stack<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<class T, class Container>
    bool operator<(const stack<T, Container>& lhs, const stack<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<class T, class Container>
    bool operator<=(const stack<T, Container>& lhs, const stack<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<class T, class Container>
    bool operator>(const stack<T, Container>& lhs, const stack<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<class T, class Container>
    bool operator>=(const stack<T, Container>& lhs, const stack<T, Container>& rhs) {
        return lhs.c == rhs.c;
    }

    template<class T, class Container>
    void swap(stack<T, Container>& lhs, stack<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }
}