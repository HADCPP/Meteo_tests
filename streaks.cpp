#include "streaks.h"

using namespace std;
using namespace PYTHON_FUNCTION;
using namespace UTILS;

namespace INTERNAL_CHECKS
{


	float rsc_get_straight_string_threshold(CMetVar& st_var, boost::gregorian::date   start, boost::gregorian::date   end, float reporting , float old_threshold)
	{
		
		CMaskedArray all_filtered = apply_filter_flags(st_var);
		//find and count the length of all repeating strings
		float prev_value = Cast<float>(st_var.getMdi());
		vector<int> this_string;
		vector<int> string_length;
		//run through all obs
		int o = 0;
		for (float obs : all_filtered.data())
		{
			if (all_filtered.mask()[0] == false)
			{
				if (obs != prev_value)
				{
					//if different values to before
					string_length.push_back(this_string.size());
					this_string.clear();
					this_string.push_back(o);
				}
				else
				{
					// if same value as before, note and continue
					this_string.push_back(o);
				}
				prev_value = obs;
			}
			o++;
		}
		return prev_value;

	}

	void rsc_straight_strings(  CMetVar& st_var , std::vector<int> times, int n_obs, int n_days, boost::gregorian::date  start, boost::gregorian::date end, bool wind , float reporting , bool dynamic)
	{
		float threshold;
		if (st_var.getName() == "winddirs")
		{
			//remove calm periods for this check
			CMetVar wd_st_var = st_var;
			valarray<size_t> calms = npwhere(st_var.getData(), float(0), '=');
			wd_st_var.setData(calms,Cast<float>( wd_st_var.getMdi()));

			if (dynamic)
			{

			}
		}
	}



	void rsc(CStation& station, std::vector<std::string> var_list, std::vector<int>  flag_col, boost::gregorian::date  start,
		boost::gregorian::date end, std::ofstream& logfile)
	{
		/*
		threshold values for low, mid and high resolution for each of the variables tested
		# consecutive repeats (ignoring missing data); spread over N days; same at each hour of day for N days; full day repeats
		*/

		array<int,4> tab1 {{40, 14, 25, 10 }};
		array<int,4> tab2 {{ 30, 10, 20, 7 }};
		array<int,4> tab3 {{ 24, 7, 15, 5 }};
		array<int,4> tab4 {{ 80, 14, 25, 10 }};
		array<int,4> tab5 {{ 60, 10, 20, 7 }};
		array<int,4> tab6 {{ 48, 7, 15, 2 }};
		array<int,4> tab7 {{ 120, 28, 25, 10 }};
		array<int,4> tab8 {{ 100, 21, 20, 7 }};
		array<int,4> tab9 {{ 72, 14, 15, 5 }};
		array<int,4> tab10{ { 120, 28, 28, 10 } };
		array<int,4> tab11 {{ 96, 28, 28, 10 }};
		array<int,4> tab12 {{ 72, 21, 21, 7 }};
		array<int,4> tab13	{{ 48, 14, 14, 7 }};
		array<int, 4> tab14{ { 24, 7, 14, 5 } };

		map<float ,array<int,4> > T, D, S, WS, WD;
		T[1] = tab1;
		T[0.5] = tab2;
		T[0.1] = tab3;
		D[1] =	tab4;
		D[0.5] = tab5;
		D[0.1] = tab6;
		S[1] = tab7;
		S[0.5] = tab8;
		S[0.1] = tab9;
		WS[1] = tab1;
		WS[0.5] = tab2;
		WS[0.1] = tab3;
		WD[90] = tab10;
		WD[45] = tab11;
		WD[22] = tab12;
		WD[10] = tab13;
		WD[1] =	tab14;

		map<string, map<float, array<int, 4> >> limits_dict;

		limits_dict["temperatures"] = T;
		limits_dict["dewpoints"] = D;
		limits_dict["slp"] = S;
		limits_dict["windspeeds"] = WS;
		limits_dict["winddirs"] = WD;

		map<float, float> WIND_MIN_VALUE;
		WIND_MIN_VALUE[1] = 0.5;
		WIND_MIN_VALUE[0.5] = 1.0;
		WIND_MIN_VALUE[0.1] = 0.5;

		vector<int> times = station.getTime_data();
		int v = 0;
		for (string variable : var_list)
		{
			CMetVar& st_var = station.getMetvar(variable);
			if (apply_filter_flags(st_var).compressed().size() > 0)
			{
				bool wind = false;
				if (variable == "windspeeds") wind = true;
				bool winddir = false;
				if (variable == "winddirs") wind = true;

				float reporting_resolution = reporting_accuracy(apply_filter_flags(st_var).data(), winddir);
				array<int,4> limits = limits_dict[variable][reporting_resolution];

				// need to apply flags to st_var.flags each time for filtering

			}
			v++;
		}
	}
}

