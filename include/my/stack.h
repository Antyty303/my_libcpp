#include "my/vector.h"
#include <algorithm>
#include <utility>
#include <memory>


namespace my {
    template<typename T, typename Container = ::vector<T>>
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
        stack() = default;
        explicit stack(const Container& cont);
        explicit stack(Container&& cont);
        stack(const stack&) = default;
        stack(stack&&) = default;
        template< class InputIt >
        stack(InputIt first, InputIt last);

        template<class Alloc>
        explicit stack(const Alloc& alloc);
        template<class Alloc>
        stack(const Container& cont, const Alloc& alloc);
        template<class Alloc>
        stack(Container&& cont, const Alloc& alloc);
        template<class Alloc>
        stack(const stack& other, const Alloc& alloc);
        template<class Alloc>
        stack(stack&& other, const Alloc& alloc);
        template< class InputIt, class Alloc >
        stack(InputIt first, InputIt last, const Alloc& alloc);

        ~stack() = default;

        stack& operator=(const stack&) = default;
        stack& operator=(stack&&) = default;

        reference top();
        const_reference top() const;
        bool empty() const;
        size_type size() const;

        void push(const value_type& value);
        void push(value_type&& value);
        template<class... Args>
        decltype(auto) emplace(Args&&... args);
        void pop();

        void swap(stack& other) noexcept(noexcept(std::swap(c, other.c)));

        bool operator<(const stack& other) const;
        bool operator<=(const stack& other) const;
        bool operator>(const stack& other) const;
        bool operator>=(const stack& other) const;
        bool operator==(const stack& other) const;
        bool operator!=(const stack& other) const;
    };