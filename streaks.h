/*
Repeated Streaks Check (RSC)
*
*   Checks for replication of
*     1) checks for consecutive repeating values
*     2) checks if one year has more repeating strings than expected
*     3) checks for repeats at a given hour across a number of days
*     4) checks for repeats for whole days - all 24 hourly values
*
*
*   Some thresholds now determined dynamically
*/

#pragma once
#include "utils.h"
#include "station.h"
#include "python_function.h"


#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <array>


namespace INTERNAL_CHECKS
{
	/*Derive threshold number for strings / streaks of repeating values

		: param object st_var : station variable object
		: param datetime start : start of data
		: param datetime end : end of data
		: param float reporting : reporting accuracy
		: param bool diagnostics : do diagnostic output
		: param bool plots : do plots
		: param float old_threshold : old threshold to use as comparison
	*/
	inline float rsc_get_straight_string_threshold(CMetVar& st_var, boost::gregorian::date   start, boost::gregorian::date  end, float reporting = 0., float old_threshold = 0.);

	/*
	Check for strings/streaks of repeating values
    :param object st_var: station variable object
    :param int n_days: number of days to exceed
    :param int n_obs: number of observations to exceed
    :param datetime start: start of data
    :param datetime end: end of data    
    :param float reporting: reporting accuracy
    :param bool wind: whether there is wind data to account for - extra minimum value
    :param bool dynamic: calculate threshold of number of observations dynamically rather than using n_obs
	*/
	void rsc_straight_strings(CMetVar& st_var, std::vector<int> times, int n_obs, int n_days, boost::gregorian::date  start, boost::gregorian::date end, std::map<float, float> WIND_MIN_VALUE, bool wind = false, float reporting = 0., bool dynamic = true);

	void rsc(CStation& station, std::vector<std::string> var_list, std::vector<int>  flag_col, boost::gregorian::date  start,
		boost::gregorian::date end, std::ofstream& logfile);
}