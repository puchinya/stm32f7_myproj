
#pragma once
#ifndef KFW_RTC_HPP
#define KFW_RTC_HPP

#include <kfw_common.hpp>

namespace kfw {

	// 1970年1月1日からの経過ミリ秒
	using utc_time_t = int64_t;

	enum class DateTimeKind
	{
		kUtc,
		kLocal
	};

	class DateTime final
	{
	public:
		DateTime()
		 : m_year(1970), m_month(0), m_day(0), m_hour(0), m_min(0), m_sec(0), m_msec(0),
		   m_kind(DateTimeKind::kUtc)
		{}
		explicit DateTime(utc_time_t ticks);

		static void get_utc_now(DateTime &dt);
		static void get_now(DateTime &dt);

		int32_t get_year() const {
			return m_year;
		}
		int32_t get_month() const {
			return m_month;
		}
		int32_t get_day() const {
			return m_day;
		}
		int32_t get_hour() const {
			return m_hour;
		}
		int32_t get_min() const {
			return m_min;
		}
		int32_t get_sec() const {
			return m_sec;
		}
		int32_t get_msec() const {
			return m_msec;
		}
		DateTimeKind get_kind() const {
			return m_kind;
		}
	private:
		void set_ticks(utc_time_t ticks);

		uint16_t m_year;
		uint8_t m_month;
		uint8_t m_day;
		uint8_t m_hour;
		uint8_t m_min;
		uint8_t m_sec;
		uint16_t m_msec;
		DateTimeKind m_kind;
	};

	class Rtc final : private NonCopyable
	{
	private:
		Rtc() {}
		static bool m_is_synced;
	public:
		static void sync(utc_time_t time);
		static void reset();
		static bool is_synced() {
			return m_is_synced;
		}

		static utc_time_t get_time();
	};

	void kfw_rtc_static_init();
};

#endif
