
#include "station.h"
#include <vector>
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include "utils.h"
#include <iostream>
#include "python_function.h"
using namespace std;
using namespace UTILS;



namespace DUPLICATE_MONTHS
{
	/*
		Perform test whether month is duplicated
		: param array source_data : source data
		: param array target_data : target data
		: param array valid : valid values
		: param int sm : source month counter
		: param int tm : target month counter
		: param array duplicated : True / False array for duplicated months
	*/
	void is_month_duplicated(valarray<float> source_data, valarray<float> target_data, vector<int> valid, int sm,
		int tm, vector<int> &duplicated)
	{
		vector<int> match = PYTHON_FUNCTION::np_where(source_data[slice(valid[0], valid.size(), 1)], target_data[slice(valid[0], valid.size(), 1)]);
	
		if (match.size() == valid.size())
		{
			duplicated[sm] = 1;
			duplicated[sm + 1 + tm] = 1;
		}
	}

	/*
		Pass into test whether month is duplicated and set flags
		: param array source_data : source data
		: param array target_data : target data
		: param array valid : valid values
		: param int sm : source month counter
		: param int tm : target month counter
		: param array duplicated : True / False array for duplicated months
	*/

	inline void duplication_test(valarray<float> source_data, valarray<float> target_data, vector<int>valid, int sm, int tm,
		map<int, int>::iterator source_month, map<int, int>::iterator  target_month, vector<int> &duplicated,CStation stat,int flag_col)
	{
		is_month_duplicated(source_data, target_data, valid, sm, tm, duplicated);

		if (duplicated[sm] == 1) //make flags if this month is duplicated somewhere
		{
			//remplir le qc_flag
		}
	}
	/*
	:param obj CStation stat: CStation object with suitable attributes
	:param list variable_list: list of netcdf variables to process
	:param list full_variable_list: the variables for flags to be applied to
	:param list flag_col: which column to set in flag array
	:param datetime start: data start
	:param datetime end: data end
	:param file logfile: logfile to store outputs

	*/
	void dmc(CStation stat, std::vector<std::string> variable_list, std::vector<std::string> full_variable_list,
		int flag_col, boost::gregorian::date start, boost::gregorian::date end, ofstream &logfile)
	{
		const  int MIN_DATA_REQUIRED = 20;
		//get array of MAP<INT,INT> start / end pairs
		map<int, int > month_ranges;
		month_ranges = UTILS::month_starts_in_pairs(start, end);
		
		int v = 0;
		for (string variable: variable_list)
		{
			CMetVar & st_var = stat.getMetvar(variable); //Recuperer la variable meteo de la CStation
			vector<int> duplicated;
			int sm = 0;
			for (map<int, int>::iterator source_month = month_ranges.begin(); source_month != month_ranges.end(); ++source_month)
			{
				cout << "Month" << (sm + 1) << "of " << month_ranges.size() << endl;
				int taille_slice = source_month->second - source_month->first+1;
				valarray<float> source_data(taille_slice);
				
				source_data = st_var.getData()[slice(source_month->first, taille_slice, 1)];
				if (duplicated[sm] == 0)   // don't repeat if already a duplicated
				{
					int tm = 0;
					for (map<int, int>::iterator target_month = source_month; target_month != month_ranges.end(); ++target_month)
					{
						if (target_month == source_month) continue;
						valarray<float> target_data;
						int taille_slice = target_month->second - target_month->first + 1;
						target_data = st_var.getData()[slice(target_month->first, taille_slice, 1)];
						tm++;
						//match the data periods
						size_t overlap = std::min(source_data.size(), target_data.size());
						valarray<float> s_data, t_data;
						s_data = source_data[slice(0,overlap,1)];
						t_data = source_data[slice(0,overlap,1)];
						vector<int> s_valid, t_valid;
						s_valid = PYTHON_FUNCTION::np_where_vec<float>(s_data, st_var.getFdi());
						t_valid = PYTHON_FUNCTION::np_where_vec<float>(t_data, st_var.getFdi());
						//if enough of an overlap
						if (s_valid.size() >= MIN_DATA_REQUIRED && t_valid.size() >= MIN_DATA_REQUIRED)
						{
							if (s_valid.size() < t_valid.size())
								duplication_test(source_data, target_data, s_valid, sm, tm, source_month, target_month, duplicated, stat, flag_col);
							else //swap the list of valid points 
								duplication_test(source_data, target_data, t_valid, sm, tm, source_month, target_month, duplicated, stat, flag_col);
						}
						tm++;
					}//target month
				}
				sm++;
			}//souce month
			UTILS::print_flagged_obs_number(logfile, "Duplicate Month", variable, 12);

		}//variable list

		UTILS::apply_flags_all_variables(stat, full_variable_list, 0, logfile, "Duplicate Months check");

	}

}