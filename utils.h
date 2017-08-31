#pragma once
#include<vector>
#include "station.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <valarray>



namespace UTILS
{
	inline std::vector<int> month_starts(boost::gregorian::date start, boost::gregorian::date end);
	inline std::map<int, int> month_starts_in_pairs(boost::gregorian::date start, boost::gregorian::date end);
	std::valarray<bool> create_fulltimes(CStation* stat, std::vector<std::string> var_list, boost::gregorian::date start,
		boost::gregorian::date end, std::vector<std::string> opt_var_list, bool do_input_station_id = true,
		bool do_qc_flags = true, bool do_flagged_obs = true);
	void monthly_reporting_statistics(CMetVar st_var, boost::gregorian::date start, boost::gregorian::date end);
	void print_flagged_obs_number(std::ofstream & logfile, std::string test, std::string variable, int nflags, bool noWrite=false);
	void apply_flags_all_variables(CStation* stat, std::vector<std::string> full_variable_list, int flag_col, std::ofstream & logfile, std::string test);
	void append_history(CStation* stat, std::string text);
	std::valarray<float> apply_filter_flags(CMetVar* st_var);
	void print_flagged_obs_number(std::ofstream& logfile, std::string test, std::string variable, int nflags, bool noWrite);
	inline float __cdecl MyApplyRoundFunc(float n)
	{
		return std::floor(n);
	}
	/*
	Uses histogram of remainders to look for special values

	: param array good_values :valarray (masked array.compressed())
	: param bool winddir : true if processing wind directions

	: output : resolution - reporting accuracy(resolution) of data
	*/
	float reporting_accuracy(std::valarray<float> good_values, bool winddir = false);
	void create_bins(std::valarray<float> indata, float binwidth, std::valarray<float> &bins, std::valarray<float>  &bincenters);
}