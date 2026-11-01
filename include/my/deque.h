#pragma once
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
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
		using Traits = std::allocator_traits<Alloc>;

		iterator begin_{};
		iterator end_{};
		size_type size_ = 0;
		Alloc alloc_;

		using map_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<pointer>;
		using map_pointer = typename std::allocator_traits<map_alloc>::pointer;
		using map_Traits = std::allocator_traits<map_alloc>;

		map_pointer map_ = nullptr; //T**
		size_type map_size_ = 0;
		map_alloc map_alloc_;

		static const size_type block_size_ = (sizeof(T) < 512) ? (512 / sizeof(T)) : 1;
		static const size_type factor_ = 2;

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
			Iterator() = default;
			Iterator(PtrT ptr, map_pointer map_ptr);
			Iterator(const Iterator& other) = default;

			RefT operator*() const { return *cur_; }
			PtrT operator->() const { return *cur_; }
			RefT operator[](difference_type off) const { return *(*this + off); }

			Iterator& operator=(const Iterator& other) = default;

			Iterator& operator++();
			Iterator operator++(int);
			Iterator& operator+=(difference_type off);
			Iterator operator+(difference_type off) const;

			Iterator& operator--();
			Iterator operator--(int);
			Iterator& operator-=(difference_type off);
			Iterator operator-(difference_type off) const;

			difference_type operator-(const Iterator& other) const;

			bool operator<(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ < other.cur_) : (map_ptr_ < other.map_ptr_) }
			bool operator<=(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ <= other.cur_) : (map_ptr_ < other.map_ptr_) }
			bool operator>(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ > other.cur_) : (map_ptr_ > other.map_ptr_) }
			bool operator>=(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ >= other.cur_) : (map_ptr_ > other.map_ptr_) }
			bool operator==(const Iterator& other) const { return cur_ == other.cur_; }
			bool operator!=(const Iterator& other) const { return cur_ != other.cur_; }
		};
#pragma endregion

	private: // methods
		void create_map_and_blocks(size_type num_elements);
		void deallocate_memory() noexcept;

	public: // methods
		deque() noexcept(noexcept(Alloc()) && std::is_nothrow_copy_constructible_v<Alloc>) : alloc_(), map_alloc_(alloc_) {}
		deque(const Alloc& alloc) noexcept(std::is_nothrow_copy_constructible_v<Alloc>) : alloc_(alloc), map_alloc_(alloc_) {}
		deque(size_type size, const Alloc& alloc = Alloc());
		deque(size_type size, const T& val, const Alloc& alloc = Alloc());
		template<class It>
		requires std::forward_iterator<It>
		deque(It first, It last, const Alloc& alloc = Alloc());
		template<class It>
		requires std::input_iterator<It> && (!std::forward_iterator<It>)
		deque(It first, It last, const Alloc& alloc = Alloc());
		deque(std::initializer_list<T> list, const Alloc& alloc = Alloc());
		deque(const deque& other);
		deque(const deque& other, const Alloc& alloc);
		deque(deque&& other) noexcept(std::is_nothrow_move_constructible_v<Alloc>);
		deque(deque&& other, const Alloc& alloc) noexcept(Traits::is_always_equal::value);
		~deque() noexcept;

		reference operator[](size_type index) { return *(begin_ + pos); }
		const_reference operator[](size_type index) const { return *(begin_ + pos); }
		reference at(size_type index) {
			if (index >= size_) throw std::out_of_range("deque::at() index out of range");
			return *(begin_ + pos);
		}
		const_reference at(size_type index) const {
			if (index >= size_) throw std::out_of_range("deque::at() index out of range");
			return *(begin_ + pos);
		}

		reference front() {
			if (size_ == 0) throw std::out_of_range("vector::front() on empty deque");
			return *begin_;
		}
		const_reference front() const {
			if (size_ == 0) throw std::out_of_range("vector::front() on empty deque");
			return *begin_;
		}
		reference back() {
			if (size_ == 0) throw std::out_of_range("vector::back() on empty deque");
			return *(end_ - 1);
		}
		const_reference back() const {
			if (size_ == 0) throw std::out_of_range("vector::back() on empty deque");
			return *(end_ - 1);
		}

		iterator begin() noexcept { return begin_; }
		iterator end() noexcept { return end_; }
		const_iterator begin() const noexcept { return { begin_.cur_, begin_.map_ptr_ }; }
		const_iterator end() const noexcept { return { end_.cur_, end_.map_ptr_ }; }
		const_iterator cbegin() const noexcept { return { begin_.cur_, begin_.map_ptr_ }; }
		const_iterator cend() const noexcept { return { end_.cur_, end_.map_ptr_ }; }

		size_type size() const noexcept { return size_; }
		bool empty() const noexcept { return size_ == 0; }
		Alloc get_allocator() const noexcept { return alloc_; }
		size_type max_size() const noexcept { return Traits::max_size(alloc_); }


	};
#pragma endregion

#pragma region iterator functions
	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>::Iterator(PtrT ptr, map_pointer map_ptr) {
		if (map_ptr != nullptr) {
			cur_ = ptr;
			map_ptr_ = map_ptr;
			block_start_ = *map_ptr;
			block_end_ = *map_ptr + block_size_;
		}
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator++() {
		++cur_;
		if (cur_ == block_end_) {
			++map_ptr_;
			block_start_ = *map_ptr_;
			block_end_ = block_start_ + block_size_;
			cur_ = block_start_;
		}
		return *this;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator++(int) {
		Iterator<PtrT, RefT> tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator+=(difference_type off) {
		difference_type offset = off + (cur_ - block_start_);

		map_pointer tmp_map_ptr = map_ptr_;
		if (offset >= 0) {
			map_ptr_ += offset / block_size_;
		}
		else {
			map_ptr_ += (offset - block_size_ + 1) / block_size_;
		}

		block_start_ = *map_ptr_;
		block_end_ = block_start_ + block_size_;
		cur_ = block_start_ + (offset - (map_ptr_ - tmp_map_ptr) * block_size_);

		return *this;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator+(difference_type off) const {
		Iterator<PtrT, RefT> tmp = *this;
		tmp += off;
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator--() {
		if (cur_ == block_start_) {
			--map_ptr_;
			block_start_ = *map_ptr_;
			block_end_ = block_start_ + block_size_;
			cur_ = block_start_;
		}
		--cur_;
		return *this;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator--(int) {
		Iterator<PtrT, RefT> tmp = *this;
		--(*this);
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator-=(difference_type off) {
		return *this += -off;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator-(difference_type off) const {
		Iterator<PtrT, RefT> tmp = *this;
		tmp += -off;
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT>::difference_type deque<T, Alloc>::Iterator<PtrT, RefT>::operator-(const Iterator<PtrT, RefT>& other) const {
		return (map_ptr_ - other.map_ptr_ - 1) * block_size_ + (cur_ - block_start_) + (other.block_end_ - other.cur_);
	}

#pragma endregion

#pragma region private
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::create_map_and_blocks(size_type num_elements) {
		size_type num_blocks = num_elements / block_size_ + 1;

		map_size_ = ((8ull > num_block) ? static_cast<size_type>(8) : (num_blocks + 2));
		map_ = map_Traits::allocate(map_alloc_, map_size_);

		map_pointer new_begin = map_ + (map_size_ - num_blocks) / 2;
		map_pointer new_end = new_begin + num_blocks - 1;

		map_pointer cur;
		try {
			for (cur = new_begin; cur <= new_end; ++cur) {
				*cur = Traits::allocate(alloc_, block_size_);
			}
		}
		catch (...) {
			for (map_pointer ptr = new_begin; ptr < cur; ++ptr) {
				Traits::deallocate(alloc_, *ptr, block_size_);
			}
			map_Traits::deallocate(map_allocator, map_, map_size_);
			throw;
		}
		size_ = num_elements;
		begin_ = iterator(*new_begin, new_begin);
		end_ = begin_ + num_elements;
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::deallocate_memory() noexcept {
		if (map_ != nullptr) {
			for (map_pointer cur = begin_.map_ptr_; cur <= end_.map_ptr_; ++cur) {
				if (*cur != nullptr) Traits::deallocate(alloc_, *cur, block_size_);
			}
			map_Traits::deallocate(map_alloc_, map_, map_size_);
			map_ = nullptr;
			map_size_ = 0;
			size_ = 0;
		}
	}
#pragma endregion

#pragma region constructors && destructor
	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(size_type size, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
		create_map_and_blocks(size);
		size_type i = 0;
		try {
			iterator it = begin_;
			for (; i < size; ++i, ++it) {
				Traits::construct(alloc_, std::addressof(*it));
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin_ + j)));
			}
			deallocate_memory();
			throw;
		}
	}

	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(size_type size, const T& val, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
		create_map_and_blocks(size);
		size_type i = 0;
		try {
			iterator it = begin_;
			for (; i < size; ++i, ++it) {
				Traits::construct(alloc_, std::addressof(*it), val);
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin_ + j)));
			}
			deallocate_memory();
			throw;
		}
	}

	template<typename T, typename Alloc>
	template<class It>
	requires  std::forward_iterator<It>
	deque<T, Alloc>::deque(It first, It last, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_)  {
		size_type size = std::distance(first, last);
		create_map_and_blocks(size);
		size_type i = 0;
		try {
			iterator it = begin_;
			for (; first != last; ++i, ++first, ++it) {
				Traits::construct(alloc_, std::addressof(*dst), *first);
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin_ + j)));
			}
			deallocate_memory();
			throw;
		}
	}

	template<typename T, typename Alloc>
	template<class It>
	requires std::input_iterator<It> && (!std::forward_iterator<It>)
	deque<T, Alloc>::deque(It first, It last, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
		size_type i = 0;
		try {
			for (; first != last; ++i, ++first) {
				push_back(*first);
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin + j)));
			}
			deallocate_memory();
			throw;
		}
	}

	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(std::initializer_list<T> list, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
		create_map_and_blocks(list.size());
		size_type i = 0;
		try {
			iterator it = begin_;
			for (auto src = init.begin(); src != init.end(); ++i, ++src, ++it) {
				Traits::construct(alloc_, std::addressof(*it), *src);
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin_ + j)));
			}
			deallocate_memory();
		}
	}
	
	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(const deque& other) : alloc_(Traits::select_on_container_copy_construction(other.alloc_)), map_alloc_(alloc_) {
		create_map_and_blocks(other.size_);
		size_type i = 0;
		try {
			iterator it = begin_;
			for (const_iterator src = other.begin(); src != other.end(); ++i, ++src, ++it) {
				Traits::construct(alloc_, std::addressof(*it), *src);
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin_ + j)));
			}
			deallocate_memory();
			throw;
		}
	}
	
	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(const deque& other, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
		create_map_and_blocks(other.size_);
		size_type i = 0;
		try {
			iterator it = begin_;
			for (const_iterator src = other.begin(); src != other.end(); ++i, ++src, ++it) {
				Traits::construct(alloc_, std::addressof(*it), *src);
			}
		}
		catch (...) {
			for (size_type j = 0; j < i; ++j) {
				Traits::destroy(alloc_, std::addressof(*(begin_ + j)));
			}
			deallocate_memory();
			throw;
		}
	}
	
	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(deque&& other) noexcept(std::is_nothrow_move_constructible_v<Alloc>) 
		: map_(other.map_), map_size_(other.map_size_), begin_(other.begin_), end_(other.end_), size_(other.size_), alloc_(std::move(other.alloc_)), map_alloc_(std::move(other.map_alloc_)) {
		other.begin_ = iterator{};
		other.end_ = iterator{};
		other.size_ = 0;
		other.map_ = nullptr;
		other.map_size_ = 0;
	}
	
	template<typename T, typename Alloc>
	deque<T, Alloc>::deque(deque&& other, const Alloc& alloc) noexcept(Traits::is_always_equal::value) : alloc_(alloc), map_alloc_(alloc_) {
		if constexpr (Traits::is_always_equal::value) {
			begin_ = other.begin_;
			end_ = other.end_;
			size_ = other.size_;
			map_ = other.map_;
			map_size_ = other.map_size_;

			other.begin_ = iterator{};
			other.end_ = iterator{};
			other.size_ = 0;
			other.map_ = nullptr;
			other.map_size_ = 0;
		}
		else {
			create_map_and_blocks(other.size_);
			iterator it = begin_;
			for (iterator src = other.begin_; src != other.end_; ++src, ++it) {
				Traits::construct(alloc_, std::addressof(*it), std::move(*src));
			}
		}
	}

	template<typename T, typename Alloc>
	deque<T, Alloc>::~deque() noexcept {
		if (map_ != nullptr) {
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}

			for (map_pointer cur = begin_.map_ptr_; cur <= end_.map_ptr_; ++cur) {
				Traits::deallocate(alloc_, *cur, block_size_);
			}

			map_Traits::deallocate(map_alloc_, map_, map_size_);
		}
	}
#pragma endregion

}