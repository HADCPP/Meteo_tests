#pragma once
#include "station.h"
#include "python_function.h"
#include "utils.h"
#include "Utilities.h"



#include <boost/date_time/gregorian/gregorian.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <map>


namespace INTERNAL_CHECKS
{
	void coc(CStation& station, std::vector<std::string> variable_list, std::vector<int> flag_col, boost::gregorian::date  start, boost::gregorian::date end, std::ofstream&  logfile, bool idl = false);
}