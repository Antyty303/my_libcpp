#pragma once
#include <initializer_list>
#include <stdexcept>
#include <memory>
#include <iterator> // std::random_access_iterator_tag

namespace my {
#pragma region Class
	template<typename T, typename Alloc = ::std::allocator<T>>
	class deque {
	private:
		template <typename PtrT, typename RefT>
		class Iterator; //forward declaration

	public:
		using value_type = T;
		using allocator_type = Alloc;
		using pointer = typename std::allocator_traits<Alloc>::pointer;
		using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;
		using reference = T&;
		using const_reference = const T&;
		using size_type = typename std::allocator_traits<Alloc>::size_type;
		using difference_type = typename std::allocator_traits<Alloc>::difference_type;
		using iterator = Iterator<T*, T&>;
		using const_iterator = Iterator<const T*, const T&>;

	private:
		using map_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<pointer>;
		using map_pointer = typename std::allocator_traits<map_alloc>::pointer;
		using map_Traits = std::allocator_traits<map_alloc>;
		
		map_pointer map_ = nullptr; //T**
		size_type map_size_ = 0;

		using Traits = std::allocator_traits<Alloc>;

		iterator begin_;
		iterator end_;
		size_type size_ = 0;
		Alloc alloc_;

		static const size_type block_size = (sizeof(T) < 512) ? (512 / sizeof(T)) : 1;
		static const size_type factor = 2;

#pragma region iterator
		template <typename PtrT, typename RefT>
		class Iterator {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = std::remove_const_t<std::remove_pointer_t<PtrT>>;
			using difference_type = std::ptrdiff_t;
			using pointer = PtrT;
			using reference = RefT;

		private:
			PtrT cur_ = nullptr;
			map_pointer map_ptr_ = nullptr;
			PtrT block_start_ = nullptr;
			PtrT block_end_ = nullptr; //start + block_size

		public:

		};
#pragma endregion

	public:
		
	};
#pragma endregion