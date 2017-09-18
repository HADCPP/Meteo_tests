#pragma once

#include "station.h"
#include "python_function.h"

#include<vector>
#include <algorithm>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <valarray>
#include <dlib\optimization.h>

namespace UTILS
{
	inline std::vector<int> month_starts(boost::gregorian::date start, boost::gregorian::date end);
	inline std::map<int, int> month_starts_in_pairs(boost::gregorian::date start, boost::gregorian::date end);
	std::valarray<bool> create_fulltimes(CStation& station, std::vector<std::string> var_list, boost::gregorian::date start,
		boost::gregorian::date end, std::vector<std::string> opt_var_list, bool do_input_station_id = true,
		bool do_qc_flags = true, bool do_flagged_obs = true);
	void monthly_reporting_statistics(const CMetVar& st_var, boost::gregorian::date start, boost::gregorian::date end);
	void print_flagged_obs_number(std::ofstream & logfile, std::string test, std::string variable, int nflags, bool noWrite=false);
	void apply_flags_all_variables(CStation& station, std::vector<std::string> full_variable_list, int flag_col, std::ofstream & logfile, std::string test);
	void apply_windspeed_flags_to_winddir(CStation& station);
	void append_history(CStation& station, std::string text);
	CMaskedArray  apply_filter_flags(CMetVar& st_var);
	void print_flagged_obs_number(std::ofstream& logfile, std::string test, std::string variable, int nflags, bool noWrite);
	inline float __cdecl MyApplyRoundFunc(float n)
	{
		return std::floor(n);
	}
	inline float __cdecl Log10Func(float n)
	{
		return std::log10f(n);
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
	inline T Cast(const S &data)
	{
		return static_cast<T>(data);
	}
	template<typename T>
	inline T Cast(const std::string &data)
	{
		string s = typeid(T).name();
		if (s == "int")
			return static_cast<T>(std::atoi(data.c_str()));

		else
			return static_cast<T>(std::atof(data.c_str()));

	}

	/*
		Plot histogram on log-y scale and fit 1/x decay curve to set threshold
		 :param array indata: input data to bin up
		:param int binmin: minimum bin value
		:param int binwidth: bin width
		:param str line_label: label for plotted histogram
		:param float old_threshold: (spike) plot the old threshold from IQR as well

		:returns:   threshold value
	   */
	float get_critical_values(std::vector<int> indata, int binmin = 0, int binwidth = 1, float old_threshold = 0.);
}