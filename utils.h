#pragma once
#include<vector>
#include "CStation.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <valarray>



namespace UTILS
{
	inline std::vector<int> month_starts(boost::gregorian::date start, boost::gregorian::date end);

	inline std::map<int, int> month_starts_in_pairs(boost::gregorian::date start, boost::gregorian::date end);

	std::valarray<bool> create_fulltimes(CStation& station, std::vector<std::string> var_list, boost::gregorian::date start,
		boost::gregorian::date end, std::vector<std::string> opt_var_list, bool do_input_station_id = true,
		bool do_qc_flags = true, bool do_flagged_obs = true);

	void monthly_reporting_statistics(CMetVar& st_var, boost::gregorian::date start, boost::gregorian::date end);

	void print_flagged_obs_number(std::ofstream & logfile, std::string test, std::string variable, int nflags, bool noWrite=false);

	void apply_flags_all_variables(CStation& station, std::vector<std::string> full_variable_list, int flag_col, std::ofstream & logfile, std::string test);
	
	void append_history(CStation& station, std::string text);

	std::valarray<float> apply_filter_flags(CMetVar* st_var);

	void print_flagged_obs_number(std::ofstream& logfile, std::string test, std::string variable, int nflags, bool noWrite);

	inline float __cdecl MyApplyRoundFunc(float n)
	{
		return std::floor(n);
	}

	inline float __cdecl MyApplyAdd(float n,int add)
	{
		return n+add;
	}
	/*
	Uses histogram of remainders to look for special values

	: param array good_values :valarray (masked array.compressed())
	: param bool winddir : true if processing wind directions

	: output : resolution - reporting accuracy(resolution) of data
	*/
	float reporting_accuracy(std::valarray<float> good_values, bool winddir = false);
	void create_bins(std::valarray<float> indata, float binwidth, std::valarray<float> &bins, std::valarray<float>  &bincenters);

	template<typename T>
	inline T idl_median(std::valarray<T> indata)
	{
		std::nth_element(std::begin(indata), std::begin(indata) + indata.size() / 2, std::end(indata));
		return indata[indata.size() / 2];
	}

	template<typename T, typename S>
	inline T Cast(S data)
	{
		return static_cast<T>(data);
	}
	template<typename T>
	inline T Cast(std::string data)
	{
		string s = typeid(T).name();
		if (s == "int")
			return static_cast<T>(std::atoi(data.c_str()));

		if (s == "float")
			return static_cast<T>(std::atof(data.c_str()));
	}
}