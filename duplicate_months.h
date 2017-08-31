#pragma once
#include"station.h"
#include "utils.h"
#include <vector>
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include<iostream>

namespace DUPLICATE_MONTHS
{
	inline void duplication_test(std::valarray<std::string> source_data, std::valarray<std::string> target_data, std::vector<int>valid, int sm, int tm,
		std::map<int, int>::iterator source_month, std::map<int, int>::iterator  target_month, std::vector<int> &duplicated, station stat, int flag_col);

	void dmc(station stat, std::vector<std::string> variable_list, std::vector<std::string> full_variable_list, int flag_col,
		boost::gregorian::date start,boost::gregorian::date end, std::ofstream &logfile);

}