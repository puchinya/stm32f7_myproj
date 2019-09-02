#pragma once

#include <kfw_common_base.hpp>

namespace kfw {

	template <typename T, size_t L> class StaticArray;
	template <typename T> class ArrayRef;
	template <typename T> class ConstArrayRef;

	template <typename T, size_t L> class StaticArray final : private NonCopyable
	{
		friend class ArrayRef<T>;
		friend class ConstArrayRef<T>;
	public:
		StaticArray() = default;

		T &operator [](size_t index) {
			return m_items[index];
		}

		size_t get_length() const {
			return L;
		}

		size_t get_byte_size() const {
			return L * sizeof(T);
		}

		operator T *() {
			return m_items;
		}

	private:
		T m_items[L];
	};

	template <typename T> class ArrayRef final
	{
		friend class ConstArrayRef<T>;
	public:
		ArrayRef(T *data, size_t length)
		: m_data(data), m_length(length) {}
		ArrayRef(const ArrayRef &obj)
		: m_data(obj.m_data), m_length(obj.m_length) {}
		template <size_t L> ArrayRef(const StaticArray<T, L> &obj)
		: m_data(obj.m_items), m_length(L) {}

		T &operator [](size_t index) {
			return m_data[index];
		}

		operator const T *() const {
			return m_data;
		}

		operator T *() {
			return m_data;
		}

		size_t get_length() const {
			return m_length;
		}

		size_t get_byte_size() const {
			return m_length * sizeof(T);
		}

	private:
		T *m_data;
		size_t m_length;
	};

	template <typename T> class ConstArrayRef final
	{
	public:
		ConstArrayRef(const T *data, size_t length)
		: m_data(data), m_length(length) {}
		ConstArrayRef(const ConstArrayRef<T> &obj)
		: m_data(obj.m_data), m_length(obj.m_length) {}
		ConstArrayRef(const ArrayRef<T> &obj)
		: m_data(obj.m_data), m_length(obj.m_length) {}
		template <size_t L> ConstArrayRef(const StaticArray<T, L> &obj)
		: m_data(obj.m_items), m_length(L) {}

		const T &operator [](size_t index) const {
			return m_data[index];
		}

		operator const T *() const {
			return m_data;
		}

		size_t get_length() const {
			return m_length;
		}

		size_t get_byte_size() const {
			return m_length * sizeof(T);
		}

	private:
		const T *m_data;
		size_t m_length;
	};

}
