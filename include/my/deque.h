#pragma once
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <memory>
#include <algorithm> //swap
#include <iterator> // std::random_access_iterator_tag
//Всегда есть хотя бы 1 блок, иначе очищается всё вплоть до map_ 
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

			const deque<T, Alloc>* container_ = nullptr;

		public:
			Iterator() = default;
			Iterator(PtrT ptr, map_pointer map_ptr, const deque* container);
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

			bool operator<(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ < other.cur_) : (map_ptr_ < other.map_ptr_); }
			bool operator<=(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ <= other.cur_) : (map_ptr_ < other.map_ptr_); }
			bool operator>(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ > other.cur_) : (map_ptr_ > other.map_ptr_); }
			bool operator>=(const Iterator& other) const { return (map_ptr_ == other.map_ptr_) ? (cur_ >= other.cur_) : (map_ptr_ > other.map_ptr_); }
			bool operator==(const Iterator& other) const { return cur_ == other.cur_; }
			bool operator!=(const Iterator& other) const { return cur_ != other.cur_; }
		};
#pragma endregion

	private: // methods
		void create_map_and_blocks(size_type num_elements);
		void deallocate_memory() noexcept;
		void shrink_to_size(size_type new_size) noexcept;
		void reallocate_map(size_type num_blocks) noexcept;
		void clean_and_steal(deque&& other) noexcept;

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

		reference operator[](size_type index) { return *(begin_ + index); }
		const_reference operator[](size_type index) const { return *(begin_ + index); }
		reference at(size_type index) {
			if (index >= size_) throw std::out_of_range("deque::at() index out of range");
			return *(begin_ + index);
		}
		const_reference at(size_type index) const {
			if (index >= size_) throw std::out_of_range("deque::at() index out of range");
			return *(begin_ + index);
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
		const_iterator begin() const noexcept { return { begin_.cur_, begin_.map_ptr_, this }; } //всё же надо чтоб кастовался
		const_iterator end() const noexcept { return { end_.cur_, end_.map_ptr_, this }; }
		const_iterator cbegin() const noexcept { return { begin_.cur_, begin_.map_ptr_, this }; }
		const_iterator cend() const noexcept { return { end_.cur_, end_.map_ptr_, this }; }

		size_type size() const noexcept { return size_; }
		bool empty() const noexcept { return size_ == 0; }
		Alloc get_allocator() const noexcept { return alloc_; }
		size_type max_size() const noexcept { return Traits::max_size(alloc_); }
		
		void assign(size_type count, const T& val);
		template<class It>
		requires std::forward_iterator<It>
		void assign(It first, It last);
		template<class It>
		requires std::input_iterator<It> && (!std::forward_iterator<It>)
		void assign(It first, It last);
		void assign(std::initializer_list<T> list);

		deque& operator=(const deque& other);
		deque& operator=(deque&& other) noexcept(
			Traits::propagate_on_container_move_assignment::value || Traits::is_always_equal::value
			);
		deque& operator=(std::initializer_list<T> list);

		void swap(deque& other) noexcept;
		void shrink_to_fit();
		void clear() noexcept;
		void resize(size_type new_size);
		void resize(size_type new_size, const T& val);
		
		template<typename... Args>
		reference emplace_back(Args&&... args);
		void push_back(const T& val);
		void push_back(T&& val);
		void pop_back();

		void push_front(const T& val);
		void push_front(T&& val);
		template<typename... Args>
		reference emplace_front(Args&&... args);
		void pop_front();
		//
		iterator insert(const_iterator pos, const T& val);
		iterator insert(const_iterator pos, T&& val);
		iterator insert(const_iterator pos, size_type count, const T& val);
		template<class It>
		requires std::forward_iterator<It>
		iterator insert(const_iterator pos, It first, It last);
		template<class It>
		requires std::input_iterator<It> && (!std::forward_iterator<It>)
		iterator insert(const_iterator pos, It first, It last);
		iterator insert(const_iterator pos, std::initializer_list<T> list);

		template<typename... Args>
		iterator emplace(const_iterator pos, Args&&... args);

		iterator erase(const_iterator pos);
		iterator erase(const_iterator first, const_iterator last);
		//
		bool operator==(const deque& other) const;
		bool operator!=(const deque& other) const;
		bool operator<=(const deque& other) const;
		bool operator>=(const deque& other) const;
		bool operator<(const deque& other) const;
		bool operator>(const deque& other) const;
	};
#pragma endregion

#pragma region iterator functions
	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>::Iterator(PtrT ptr, map_pointer map_ptr, const deque* container) {
		if (map_ptr != nullptr && *map_ptr_ != nullptr) {
			cur_ = ptr;
			map_ptr_ = map_ptr;
			block_start_ = *map_ptr;
			block_end_ = *map_ptr + block_size_;
			container_ = container;
		}
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline typename deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator++() {
		++cur_;
		if (cur_ == block_end_) {
			++map_ptr_;
			if (map_ptr_ == container_->map_ + container_->map_size_) {
				cur_ = block_start_ = block_end_ = nullptr;
			}
			else {
				block_start_ = *map_ptr_;
				block_end_ = block_start_ + block_size_;
				cur_ = block_start_;
			}
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
		
		if (map_ptr_ == container_->map_ + container_->map_size_) {
			cur_ = block_start_ = block_end_ = nullptr;
		}
		else {
			block_start_ = *map_ptr_;
			block_end_ = block_start_ + block_size_;
			cur_ = block_start_ + (offset - (map_ptr_ - tmp_map_ptr) * block_size_);
		}

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
			cur_ = block_end_;
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
		if (num_elements > max_size()) throw std::length_error("deque length exceeds max_size");

		if (num_elements == 0) {
			begin_ = end_ = iterator();
			size_ = 0;
			map_ = nullptr;
			map_size_ = 0;
			return;
		}

		size_type num_blocks = num_elements / block_size_ + 1;

		map_size_ = ((4ull > num_blocks) ? 8 : (num_blocks + 4));
		map_ = map_Traits::allocate(map_alloc_, map_size_);
		for (size_type i = 0; i < map_size_; ++i) {
			map_[i] = nullptr;
		}

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
			map_Traits::deallocate(map_alloc_, map_, map_size_);
			throw;
		}
		size_ = num_elements;
		begin_ = iterator(*new_begin, new_begin, this);
		end_ = begin_ + num_elements;
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::deallocate_memory() noexcept {
		if (map_ != nullptr) { //This ensures that begin_.map_ptr_ != nullptr
			for (map_pointer cur = map_; cur < map_ + map_size_; ++cur) {
				if (*cur != nullptr) {
					Traits::deallocate(alloc_, *cur, block_size_);
					*cur = nullptr;
				}
			}
			map_Traits::deallocate(map_alloc_, map_, map_size_);
			map_ = nullptr;
			map_size_ = 0;
			size_ = 0;
			begin_ = end_ = iterator();
		}
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::shrink_to_size(size_type new_size) noexcept {
		for (iterator it = begin_ + new_size; it != end_; ++it) Traits::destroy(alloc_, std::addressof(*it));
		end_ = begin_ + new_size;

		if (end_.cur_ == end_.block_start_ && end_.map_ptr_ != begin_.map_ptr_) {
			Traits::deallocate(alloc_, *end_.map_ptr_, block_size_);
			*end_.map_ptr_ = nullptr;
			end_.block_start_ = end_.cur_ = nullptr;
			end_.block_end_ = end_.block_start_ + block_size_;
		}

		for (map_pointer cur = end_.map_ptr_ + 1; cur < map_ + map_size_; ++cur) {
			if (*cur != nullptr) {
				Traits::deallocate(alloc_, *cur, block_size_);
				*cur = nullptr;
			}
		}

		size_ = new_size;
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::reallocate_map(size_type num_blocks) noexcept {
		if (num_blocks <= map_size_) return;

		size_type new_map_size = num_blocks + 4;
		map_pointer new_map = map_Traits::allocate(map_alloc_, num_blocks);
		
		new_map[0] = new_map[new_map_size - 1] = nullptr;
		for (size_type i = 0; i < map_size_; ++i) {
			new_map[i + 2] = map_[i];
		}
		new_map[1] = Traits::allocate(alloc_, block_size_);
		new_map[new_map_size - 2] = Traits::allocate(alloc_, block_size_);

		size_type begin_off = begin_.map_ptr_ - map_ + 2;
		size_type end_off = end_.map_ptr_ - map_ + 2;
		begin_.map_ptr_ = new_map + begin_off;
		end_.map_ptr_ = new_map + end_off;

		map_Traits::deallocate(map_, map_size_);
		map_ = new_map;
		map_size_ = new_map_size;
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::clean_and_steal(deque&& other) noexcept {
		for (iterator it = begin_; it != end_; ++it) {
			Traits::destroy(alloc_, std::addressof(*it));
		}
		if (map_ != nullptr) {
			for (map_pointer cur = map_; cur < map_ + map_size_; ++cur) {
				if (*cur != nullptr) {
					Traits::deallocate(alloc_, *cur, block_size_);
					*cur = nullptr;
				}
			}
			map_Traits::deallocate(map_alloc_, map_, map_size_);
		}

		begin_ = other.begin_;
		end_ = other.end_;
		size_ = other.size_;
		map_ = other.map_;
		map_size_ = other.map_size_;

		other.begin_ = other.end_ = iterator{};
		other.size_ = 0;
		other.map_ = nullptr;
		other.map_size_ = 0;
	}
#pragma endregion

#pragma region constructors && destructor
	template<typename T, typename Alloc>
	inline deque<T, Alloc>::deque(size_type size, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
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
	inline deque<T, Alloc>::deque(size_type size, const T& val, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
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
	inline deque<T, Alloc>::deque(It first, It last, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_)  {
		size_type size = std::distance(first, last);
		create_map_and_blocks(size);
		size_type i = 0;
		try {
			iterator it = begin_;
			for (; first != last; ++i, ++first, ++it) {
				Traits::construct(alloc_, std::addressof(*it), *first);
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
	inline deque<T, Alloc>::deque(It first, It last, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
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
	inline deque<T, Alloc>::deque(std::initializer_list<T> list, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
		create_map_and_blocks(list.size());
		size_type i = 0;
		try {
			iterator it = begin_;
			for (auto src = list.begin(); src != list.end(); ++i, ++src, ++it) {
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
	inline deque<T, Alloc>::deque(const deque& other) : alloc_(Traits::select_on_container_copy_construction(other.alloc_)), map_alloc_(alloc_) {
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
	inline deque<T, Alloc>::deque(const deque& other, const Alloc& alloc) : alloc_(alloc), map_alloc_(alloc_) {
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
	inline deque<T, Alloc>::deque(deque&& other) noexcept(std::is_nothrow_move_constructible_v<Alloc>) 
		: map_(other.map_), map_size_(other.map_size_), begin_(other.begin_), end_(other.end_), size_(other.size_), alloc_(std::move(other.alloc_)), map_alloc_(std::move(other.map_alloc_)) {
		other.begin_ = iterator{};
		other.end_ = iterator{};
		other.size_ = 0;
		other.map_ = nullptr;
		other.map_size_ = 0;
	}
	
	template<typename T, typename Alloc>
	inline deque<T, Alloc>::deque(deque&& other, const Alloc& alloc) noexcept(Traits::is_always_equal::value) : alloc_(alloc), map_alloc_(alloc_) {
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
			if (alloc_ == other.alloc_) {
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
	}

	template<typename T, typename Alloc>
	inline deque<T, Alloc>::~deque() noexcept {
		if (map_ != nullptr) {
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}

			for (map_pointer cur = map_; cur < map_ + map_size_; ++cur) {
				if (*cur != nullptr) {
					Traits::deallocate(alloc_, *cur, block_size_);
				}
			}

			map_Traits::deallocate(map_alloc_, map_, map_size_);
		}
	}
#pragma endregion

#pragma region assign && operators
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::assign(size_type count, const T& val) {
		if (count == 0) {
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}
			deallocate_memory();
			return;
		}

		if constexpr (std::is_nothrow_copy_constructible_v<T>) {
			size_t assign_count = std::min(count, size_);

			for (iterator it = begin_; assign_count > 0; --assign_count, ++it) {
				*it = val;
			}

			if (count < size_) { shrink_to_size(count); return; }
			if (count == size_) return;

			size_type extra = count - size_;
			const size_type remaining_in_end = (end_.map_ptr_ != map_ + map_size_) ? (end_.block_start_ + block_size_ - end_.cur_) : 0;

			size_type blocks_needed = 0;
			if (extra > remaining_in_end) {
				blocks_needed = (extra - remaining_in_end + block_size_ - 1) / block_size_;
			}

			const size_type remaining_in_map = map_size_ - (end_.map_ptr_ - map_);
			if (remaining_in_map != 0) --remaining_in_map;

			if (blocks_needed > remaining_in_map) reallocate_map(blocks_needed);
			
			while (extra > 0) {
				if (end_.cur_ == end_.block_start_ + block_size_) {
					++end_.map_ptr_;
					if (*end_.map_ptr_ == nullptr) {
						*end_.map_ptr_ = Traits::allocate(alloc_, block_size_);
					}
					end_.block_start_ = *end_.map_ptr_;
					end_.block_end_ = *end_.map_ptr + block_size_;
					end_.cur_ = end_.block_start_;
				}
				Traits::construct(alloc_, std::addressof(*end_), val);
				++end_.cur_;
				++size_;
				--extra;
			}
			return;
		}
		else {
			deque tmp(count, val, alloc_);
			
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}
			deallocate_memory();

			begin_ = tmp.begin_;
			end_ = tmp.end_;
			size_ = tmp.size_;
			map_ = tmp.map_;
			map_size_ = tmp.map_size_;

			tmp.begin_ = tmp.end_ = iterator{};
			tmp.size_ = 0;
			tmp.map_ = nullptr;
			tmp.map_size_ = 0;
		}
	}

	template<typename T, typename Alloc>
	template<class It>
	requires std::forward_iterator<It>
	inline void deque<T, Alloc>::assign(It first, It last) {
		size_type count = static_cast<size_type>(std::distance(first, last));
		if (count == 0) {
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}
			deallocate_memory();
			return;
		}

		if constexpr (std::is_nothrow_copy_constructible_v<T>) {
			size_t assign_count = std::min(count, size_);

			for (iterator it = begin_; assign_count > 0; --assign_count, ++it, ++first) {
				*it = *first;
			}

			if (count < size_) { shrink_to_size(count); return; }
			if (count == size_) return;

			size_type extra = count - size_;
			const size_type remaining_in_end = (end_.map_ptr_ != map_ + map_size_) ? (end_.block_start_ + block_size_ - end_.cur_) : 0;
			
			size_type blocks_needed = 0;
			if (extra > remaining_in_end) {
				blocks_needed = (extra - remaining_in_end + block_size_ - 1) / block_size_;
			}

			const size_type remaining_in_map = map_size_ - (end_.map_ptr_ - map_);
			if (remaining_in_map != 0) --remaining_in_map; 
			
			if (blocks_needed > remaining_in_map) reallocate_map(blocks_needed);
			
			while (extra > 0) {
				if (end_.cur_ == end_.block_start_ + block_size_) {
					++end_.map_ptr_;
					if (*end_.map_ptr_ == nullptr) {
						*end_.map_ptr_ = Traits::allocate(alloc_, block_size_);
					}
					end_.block_start_ = *end_.map_ptr_;
					end_.block_end_ = *end_.map_ptr + block_size_;
					end_.cur_ = end_.block_start_;
				}
				Traits::construct(alloc_, std::addressof(*end_), *(first));
				++first;
				++end_.cur_;
				++size_;
				--extra;
			}
			return;
		}
		else {
			deque tmp(first, last, alloc_);
			
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}
			deallocate_memory();

			begin_ = tmp.begin_;
			end_ = tmp.end_;
			size_ = tmp.size_;
			map_ = tmp.map_;
			map_size_ = tmp.map_size_;

			tmp.begin_ = tmp.end_ = iterator{};
			tmp.size_ = 0;
			tmp.map_ = nullptr;
			tmp.map_size_ = 0;
		}
	}

	template<typename T, typename Alloc>
	template<class It>
	requires std::input_iterator<It> && (!std::forward_iterator<It>)
	inline void deque<T, Alloc>::assign(It first, It last) {
		deque tmp(alloc_);
		for (; first != last; ++first) {
			tmp.push_back(*first);
		}

		for (iterator it = begin_; it != end_; ++it) {
			Traits::destroy(alloc_, std::addressof(*it));
		}
		deallocate_memory();

		begin_ = tmp.begin_;
		end_ = tmp.end_;
		size_ = tmp.size_;
		map_ = tmp.map_;
		map_size_ = tmp.map_size_;

		tmp.begin_ = tmp.end_ = iterator{};
		tmp.size_ = 0;
		tmp.map_ = nullptr;
		tmp.map_size_ = 0;
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::assign(std::initializer_list<T> list) {
		assign(list.begin(), list.end());
	}

	template<typename T, typename Alloc>
	inline deque<T, Alloc>& deque<T, Alloc>::operator=(const deque& other) {
		if (this == &other) return *this;

		if constexpr (Traits::propagate_on_container_copy_assignment::value) {
			if (alloc_ != other.alloc_) {
				for (iterator it = begin_; it != end_; ++it) {
					Traits::destroy(alloc_, std::addressof(*it));
				}
				deallocate_memory();
				alloc_ = other.alloc_;
				map_alloc_ = other.map_alloc_;
			}
		}

		assign(other.begin_, other.end_);
		return *this;
	}
	
	template<typename T, typename Alloc>
	inline deque<T, Alloc>& deque<T, Alloc>::operator=(deque&& other) noexcept(
		Traits::propagate_on_container_move_assignment::value || Traits::is_always_equal::value)
	{
		if (this == &other) return *this;

		if (other.size_ == 0) {
			for (iterator it = begin_; it != end_; ++it) {
				Traits::destroy(alloc_, std::addressof(*it));
			}
			deallocate_memory();

			if constexpr (Traits::propagate_on_container_move_assignment::value) {
				alloc_ = std::move(other.alloc_);
				map_alloc_ = std::move(other.map_alloc_);
			}

			other.deallocate_memory();
			return *this;
		}

		if constexpr (Traits::is_always_equal::value || Traits::propagate_on_container_move_assignment::value) {

			clean_and_steal(std::move(other));
			if constexpr (Traits::propagate_on_container_move_assignment::value) {
				alloc_ = std::move(other.alloc_);
				map_alloc_ = std::move(other.map_alloc_);
			}

		}
		else {
			if (this->alloc_ == other.alloc_) {
				clean_and_steal(std::move(other));
			}
			else {
				deque tmp(std::move(other), alloc_);
				clean_and_steal(std::move(tmp));

				for (iterator it = other.begin_; it != other.end_; ++it) {
					Traits::destroy(other.alloc_, std::addressof(*it));
				}
				other.deallocate_memory();
			}
		}
		return *this;
	}
	
	template<typename T, typename Alloc>
	inline deque<T, Alloc>& deque<T, Alloc>::operator=(std::initializer_list<T> list) {
		assign(list.begin(), list.end());
		return *this;
	}
#pragma endregion

#pragma region swap && size
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::swap(deque& other) noexcept {
		if (this == &other) return;

		std::swap(map_, other.map_);
		std::swap(map_size_, other.map_size_);
		std::swap(begin_, other.begin_);
		std::swap(end_, other.end_);
		std::swap(size_, other.size_);

		if constexpr (Traits::propagate_on_container_swap::value) {
			std::swap(alloc_, other.alloc_);
			std::swap(map_alloc_, other.map_alloc_);
		}
	}
	
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::shrink_to_fit() {
		if (size_ == 0) {
			deallocate_memory();
			return;
		}

		deque tmp(alloc_);
		tmp.create_map_and_blocks(size_);
		tmp.size_ = 0;
		tmp.end_ = tmp.begin_;

		for (iterator it = begin_; it != end_; ++it) {
			Traits::construct(tmp.alloc_, std::addressof(*tmp.end_), std::move_if_noexcept(*it));
			++tmp.end_;
			++tmp.size_;
		}

		for (iterator it = begin_; it != end_; ++it) {
			Traits::destroy(alloc_, std::addressof(*it));
		}
		deallocate_memory();
		map_ = tmp.map_;
		map_size_ = tmp.map_size_;
		begin_ = tmp.begin_;
		end_ = tmp.end_;
		size_ = tmp.size_;

		tmp.map_ = nullptr;
		tmp.map_size_ = 0;
		tmp.size_ = 0;
		tmp.begin_ = tmp.end_ = iterator();
	}
	
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::clear() noexcept {
		for (iterator it = begin_; it != end_; ++it) {
			Traits::destroy(alloc_, std::addressof(*it));
		}

		if (map_ != nullptr) {
			pointer block_to_keep = *begin_.map_ptr_;
			*begin_.map_ptr_ = nullptr;

			for (map_pointer cur = map_; cur < map_ + map_size_; ++cur) {
				if (*cur != nullptr) {
					Traits::deallocate(alloc_, *cur, block_size_);
					*cur = nullptr;
				}
			}

			map_pointer central_ptr = map_ + map_size_ / 2;
			*central_ptr = block_to_keep;

			begin_ = iterator(block_to_keep, central_ptr, this);
			end_ = begin_;
		}

		size_ = 0;
	}
	
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::resize(size_type new_size) {
		if (new_size > max_size()) throw std::length_error("deque length exceeds max_size");
	
		if (new_size < size_) {
			if (new_size == 0) { clear(); return; }
			shrink_to_size(new_size); return;
		}
		if (new_size == size_) return;

		deque tmp(alloc_);
		tmp.create_map_and_blocks(new_size);
		tmp.size_ = 0;
		tmp.end_ = tmp.begin_;

		for (iterator it = begin_; it != end_; ++it) {
			Traits::construct(tmp.alloc_, std::addressof(*tmp.end_), std::move_if_noexcept(*it));
			++tmp.end_;
			++tmp.size_;
		}
		for (size_type i = size_; i < new_size; ++i) {
			Traits::construct(tmp.alloc_, std::addressof(*tmp.end_));
			++tmp.end_;
			++tmp.size_;
		}

		for (iterator it = begin_; it != end_; ++it) {
			Traits::destroy(alloc_, std::addressof(*it));
		}
		deallocate_memory();
		map_ = tmp.map_;
		map_size_ = tmp.map_size_;
		begin_ = tmp.begin_;
		end_ = tmp.end_;
		size_ = tmp.size_;

		tmp.map_ = nullptr;
		tmp.map_size_ = 0;
		tmp.size_ = 0;
		tmp.begin_ = tmp.end_ = iterator();
	}
	
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::resize(size_type new_size, const T& val) {
		if (new_size > max_size()) throw std::length_error("deque length exceeds max_size");

		if (new_size < size_) {
			if (new_size == 0) { clear(); return; }
			shrink_to_size(new_size); return;
		}
		if (new_size == size_) return;

		deque tmp(alloc_);
		tmp.create_map_and_blocks(new_size);
		tmp.size_ = 0;
		tmp.end_ = tmp.begin_;

		for (iterator it = begin_; it != end_; ++it) {
			Traits::construct(tmp.alloc_, std::addressof(*tmp.end_), std::move_if_noexcept(*it));
			++tmp.end_;
			++tmp.size_;
		}
		for (size_type i = size_; i < new_size; ++i) {
			Traits::construct(tmp.alloc_, std::addressof(*tmp.end_), val);
			++tmp.end_;
			++tmp.size_;
		}

		for (iterator it = begin_; it != end_; ++it) {
			Traits::destroy(alloc_, std::addressof(*it));
		}
		deallocate_memory();
		map_ = tmp.map_;
		map_size_ = tmp.map_size_;
		begin_ = tmp.begin_;
		end_ = tmp.end_;
		size_ = tmp.size_;

		tmp.map_ = nullptr;
		tmp.map_size_ = 0;
		tmp.size_ = 0;
		tmp.begin_ = tmp.end_ = iterator();
	}
#pragma endregion

#pragma region comparison
	template<typename T, typename Alloc>
	inline bool deque<T, Alloc>::operator==(const deque& other) const {
		if (this == &other) return true;
		if (size_ != other.size_) return false;
		iterator a = begin_, b = other.begin_;
		for (; a != end_; ++a, ++b) {
			if (!(*a == *b)) return false;
		}
		return true;
	}

	template<typename T, typename Alloc>
	inline bool deque<T, Alloc>::operator!=(const deque& other) const {
		if (this == &other) return false;
		if (size_ != other.size_) return true;
		iterator a = begin_, b = other.begin_;
		for (; a != end_; ++a, ++b) {
			if (!(*a == *b)) return true;
		}
		return false;
	}
	
	template<typename T, typename Alloc>
	inline bool deque<T, Alloc>::operator<=(const deque& other) const {
		iterator a = begin_, b = other.begin_;
		for (; a != end_ && b != other.end_; ++a, ++b) {
			if (*a < *b) return true;
			if (*b < *a) return false;
		}
		return a == end_;
	}
	
	template<typename T, typename Alloc>
	inline bool deque<T, Alloc>::operator>=(const deque& other) const {
		iterator a = begin_, b = other.begin_;
		for (; a != end_ && b != other.end_; ++a, ++b) {
			if (*a < *b) return false;
			if (*b < *a) return true;
		}
		return b == other.end_;
	}
	
	template<typename T, typename Alloc>
	inline bool deque<T, Alloc>::operator<(const deque& other) const {
		iterator a = begin_, b = other.begin_;
		for (; a != end_ && b != other.end_; ++a, ++b) {
			if (*a < *b) return true;
			if (*b < *a) return false;
		}
		return (a == end_) && (b != other.end_);
	}
	
	template<typename T, typename Alloc>
	inline bool deque<T, Alloc>::operator>(const deque& other) const {
		iterator a = begin_, b = other.begin_;
		for (; a != end_ && b != other.end_; ++a, ++b) {
			if (*a < *b) return false;
			if (*b < *a) return true;
		}
		return (a != end_) && (b == other.end_);
	}
#pragma endregion

#pragma region back
	template<typename T, typename Alloc>
	template<typename... Args>
	inline deque<T, Alloc>::reference deque<T, Alloc>::emplace_back(Args&&... args) {
		if (size_ == max_size()) throw std::length_error("deque length exceeds max_size");

		if (map_ == nullptr) {
			create_map_and_blocks(1);
			size_ = 0;
			end_ = begin_;
		}
		else if (end_.cur_ == end_.block_start_ && end_.map_ptr_ != begin_.map_ptr_) {
			if (end_.map_ptr_ == map_ + map_size_) {
				reallocate_map(map_size_);
			}

			if (*end_.map_ptr_ == nullptr) {
				*end_.map_ptr_ = Traits::allocate(alloc_, block_size_);
				end_.block_start_ = *end_.map_ptr_;
				end_.block_end_ = *end_.map_ptr + block_size_;
				end_.cur_ = end_.block_start_;
			}
		}
		Traits::construct(alloc_, std::addressof(*end_), std::forward<Args>(args)...);
		++end_;
		++size_;

		return *(end_ - 1);
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::push_back(const T& val) {
		emplace_back(val);
	}
	
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::push_back(T&& val) {
		emplace_back(std::move(val));
	}
	
	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::pop_back() {
		if (size_ == 0) throw std::out_of_range("Error: deque::pop_back() called on empty deque");

		if (end_.cur_ == end_.block_start_ && end_.map_ptr_ != begin_.map_ptr_) {
			if ((end_.map_ptr_ + 1) < map_ + map_size_ && *(end_.map_ptr_ + 1) != nullptr) {
				Traits::deallocate(alloc_, *(end_.map_ptr_ + 1), block_size_);
				*(end_.map_ptr_ + 1) = nullptr;
			}
		}
		--end_;
		Traits::destroy(alloc_, std::addressof(*end_));
		--size_;
	}
#pragma endregion

#pragma region front
	template<typename T, typename Alloc>
	template<typename... Args>
	inline deque<T, Alloc>::reference deque<T, Alloc>::emplace_front(Args&&... args) {
		if (size_ == max_size()) throw std::length_error("deque length exceeds max_size");

		if (map_ == nullptr) {
			create_map_and_blocks(1);
			size_ = 0;
			end_ = begin_;
		}
		
		if (begin_.cur_ == begin_.block_start_) {
			if (begin_.map_ptr_ == map_) {
				reallocate_map(map_size_);
			}

			if (*(begin_.map_ptr_ - 1) == nullptr) {
				*(begin_.map_ptr_ - 1) = Traits::allocate(alloc_, block_size_);
			}
		}
		Traits::construct(alloc_, std::addressof(*(begin_ - 1)), std::forward<Args>(args)...);
		--begin_;
		++size_;

		return *begin_;
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::push_front(const T& val) {
		emplace_front(val);
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::push_front(T&& val) {
		emplace_front(std::move(val));
	}

	template<typename T, typename Alloc>
	inline void deque<T, Alloc>::pop_front() {
		if (size_ == 0) throw std::out_of_range("Error: deque::pop_front() called on empty deque");

		if (begin_.cur_ == begin_.block_end_ - 1) {
			if (begin_.map_ptr_ > map_ && *(begin_.map_ptr_ - 1) != nullptr) {
				Traits::deallocate(alloc_, *(begin_.map_ptr_ - 1), block_size_);
				*(begin_.map_ptr_ - 1) = nullptr;
			}
		}
		Traits::destroy(alloc_, std::addressof(*begin_));
		++begin_;
		--size_;
	}
#pragma endregion

#pragma region middle

#pragma endregion

}