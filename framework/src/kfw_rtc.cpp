
#include <kfw_rtc.hpp>
#include <itron.h>


#define EPOCH_ADJUSTMENT_DAYS	719468
#define ADJUSTED_EPOCH_YEAR     0
#define ADJUSTED_EPOCH_WDAY     3
#define DAYS_PER_ERA            146097
#define DAYS_PER_CENTURY        36524
#define DAYS_PER_4_YEARS        (3 * 365 + 366)
#define DAYS_PER_YEAR           365
#define DAYS_IN_JANUARY         31
#define DAYS_IN_FEBRUARY        28
#define YEARS_PER_ERA           400

namespace kfw {

	DateTime::DateTime(utc_time_t ticks)
	{
		set_ticks(ticks);
	}

	void DateTime::get_utc_now(DateTime &dt)
	{
		auto utc_now = Rtc::get_time();
		dt.set_ticks(utc_now);
	}

	// @TODO
	void DateTime::get_now(DateTime &dt)
	{
		auto utc_now = Rtc::get_time();
		dt.set_ticks(utc_now);
	}

	void DateTime::set_ticks(utc_time_t ticks)
	{
		uint64_t total_sec = ticks / 1000;
		int32_t days = total_sec / (24 * 3600) + EPOCH_ADJUSTMENT_DAYS;
		int32_t rem = total_sec % (24 * 3600);
		if(rem < 0) {
			rem += 24 * 3600;
			--days;
		}
		m_hour = rem / 3600;
		rem %= 3600;
		m_min = rem / 60;
		m_sec = rem % 60;
		m_msec = ticks % 1000;
		m_kind = DateTimeKind::kUtc;

		/* compute year, month, day & day of year */
		/* for description of this algorithm see
		 * http://howardhinnant.github.io/date_algorithms.html#civil_from_days */

		int32_t era = (days >= 0 ? days : days - (DAYS_PER_ERA - 1)) / DAYS_PER_ERA;
		int32_t eraday = days - era * DAYS_PER_ERA;
		int32_t erayear = (eraday - eraday / (DAYS_PER_4_YEARS - 1) + eraday / DAYS_PER_CENTURY -
				eraday / (DAYS_PER_ERA - 1)) / 365;
		int32_t yearday = eraday - (DAYS_PER_YEAR * erayear + erayear / 4 - erayear / 100);
		int32_t month = (5 * yearday + 2) / 153;
		int32_t day = yearday - (153 * month + 2) / 5 + 1;
		month += month < 10 ? 2 : -10;
		int32_t year = ADJUSTED_EPOCH_YEAR + erayear + era * YEARS_PER_ERA + (month <= 1);
		month = month + 1;
		m_year = year;
		m_month = month;
		m_day = day;
	}

	static utc_time_t s_time_counter;

	static utc_time_t get_time_counter() {
		utc_time_t r;
		loc_cpu();
		r = s_time_counter;
		unl_cpu();
		return r;
	}

	static void set_time_counter(utc_time_t v)
	{
		loc_cpu();
		s_time_counter = v;
		unl_cpu();
	}

	bool Rtc::m_is_synced = false;

	void Rtc::sync(utc_time_t time)
	{
		set_time_counter(time);
		m_is_synced = true;
	}

	void Rtc::reset()
	{
		set_time_counter(0);
		m_is_synced = false;
	}

	utc_time_t Rtc::get_time()
	{
		return get_time_counter();
	}

	extern "C" {
		static void kfw_cyc_handler();
	};

	static void kfw_cyc_handler()
	{
		s_time_counter += 10;
	}

	void kfw_rtc_static_init()
	{
		T_CCYC ccyc = {0};

		ccyc.cycatr = KOS_TA_STA;
		ccyc.cychdr = (kos_fp_t)kfw_cyc_handler;
		ccyc.cyctim = 10;

		kos_cre_cyc(&ccyc);
	}

}
