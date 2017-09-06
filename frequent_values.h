#pragma once
#include <string>
#include "CStation.h"
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
namespace
{
	const std::string SEASONS[5] = { "Ann", "MAM", "JJA", "SON", "D+JF" };
}
namespace FREQUENT_VALUES
{
	 /* Check for certain values occurring more frequently than would be expected
		: param object CStation : CStation object to process
		: param list variable_list : list of variables to process
		: param list flag_col : columns to fill in flag array
		: param datetime start : datetime object of start of data
		: param datetime end : datetime object of end of data
		: param file logfile : logfile to store outputs
	*/ 
	void  fvc(CStation &stat, std::vector<std::string> variable_list, std::vector<int> flag_col, boost::gregorian::date start, boost::gregorian::date end, std::ofstream &logfile);
	
}