
#include <kfw_common_string.hpp>

namespace kfw {

	dse_string_t dse_string_create(uint32_t len) {
		uint8_t *p = new uint8_t[len + 8];
		if(p == nullptr) {
			return nullptr;
		}
		((uint32_t *)p)[0] = 1;
		((uint32_t *)p)[1] = len;
		memset(p + 8, 0, len);
		return (dse_string_t)(p + 8);
	}

	dse_string_t dse_string_create(const ConstStringRef &s) {
		uint8_t *p = new uint8_t[s.get_length() + 8];
		if(p == nullptr) {
			return nullptr;
		}
		((uint32_t *)p)[0] = 1;
		((uint32_t *)p)[1] = s.get_length();
		uint8_t *str_top = p + 8;
		memcpy(str_top, s.get_data(), s.get_length());
		return (dse_string_t)str_top;
	}

	void dse_string_addref(dse_string_t s) {
		volatile uint32_t *p_cnt = (volatile uint32_t *)(s - 8);
		atomic_inc(p_cnt);
	}

	void dse_string_release(dse_string_t s) {
		volatile uint32_t *p_cnt = (volatile uint32_t *)(s - 8);
		uint32_t new_cnt = atomic_dec(p_cnt);
		if(new_cnt == 0) {
			delete p_cnt;
		}
	}
}
