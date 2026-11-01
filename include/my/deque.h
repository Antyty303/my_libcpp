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
		using map_alloc = typename std::allocator_traits<Alloc>::rebind_alloc<pointer>;
		using map_pointer = typename std::allocator_traits<map_alloc>::pointer;
		using map_Traits = std::allocator_traits<map_alloc>;
		
		map_pointer map_ = nullptr; //T**
		size_type map_size_ = 0;

		using Traits = std::allocator_traits<Alloc>;

		iterator begin_;
		iterator end_;
		size_type size_ = 0;
		Alloc alloc_;

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
			Iterator(PtrT ptr, map_pointer map_ptr)
				: cur_(ptr), map_ptr_(map_ptr), block_start_(*map_ptr), block_end_(*map_ptr + block_size_) {}
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

	public:
		
	};
#pragma endregion

#pragma region iterator functions
	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator++() {
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
	inline deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator++(int) {
		Iterator<PtrT, RefT> tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator+=(difference_type off) {
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
	inline deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator+(difference_type off) const {
		Iterator<PtrT, RefT> tmp = *this;
		tmp += off;
		return tmp;
	}
	
	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator--() {
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
	inline deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator--(int) {
		Iterator<PtrT, RefT> tmp = *this;
		--(*this);
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>& deque<T, Alloc>::Iterator<PtrT, RefT>::operator-=(difference_type off) {
		return *this += -off;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT> deque<T, Alloc>::Iterator<PtrT, RefT>::operator-(difference_type off) const {
		Iterator<PtrT, RefT> tmp = *this;
		tmp += -off;
		return tmp;
	}

	template<typename T, typename Alloc>
	template<typename PtrT, typename RefT>
	inline deque<T, Alloc>::Iterator<PtrT, RefT>::difference_type deque<T, Alloc>::Iterator<PtrT, RefT>::operator-(const Iterator<PtrT, RefT>& other) const {
		return (map_ptr_ - other.map_ptr_ - 1) * block_size_ + (cur_ - block_start_) + (other.block_end_ - other.cur_);
	}

#pragma endregion