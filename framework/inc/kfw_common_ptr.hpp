#pragma once

#include <kfw_common_base.hpp>

namespace kfw {

	template <typename T> class UniquePtr final : private NonCopyable
	{
	public:
		UniquePtr(T *p) : m_p(p) {}
		~UniquePtr() {
			dispose();
		}

		void dispose() {
			if(m_p != nullptr) {
				delete m_p;
				m_p = nullptr;
			}
		}

		T *operator -> () {
			return m_p;
		}

		T &operator *() {
			return *m_p;
		}

		operator T *() {
			return m_p;
		}

		operator T &() {
			return *m_p;
		}

	private:
		T *m_p;
	};

}
