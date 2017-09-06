#pragma once
#include<vector>
#include "CStation.h"
#include<string>


struct test
{
	bool duplicate = true;
	bool odd = true;
	bool frequent = true;
	bool diurnal = true;
	bool gap = true;
	bool records = true;
	bool streaks = true;
	bool climatological = true;
	bool spike = true;
	bool humidity = true;
	bool cloud = true;
	bool variance = true;
	bool winds = true;
};
typedef struct test test;
namespace
{
	std::vector<std::string> process_var{ "temperatures", "dewpoints", "windspeeds", "winddirs", "slp" };
	std::vector<std::string> carry_thru_vars;
	
}
namespace INTERNAL_CHECKS

{
	void internal_checks(std::vector<CStation> &station_info, test mytest , bool second,std::string *DATE);
}