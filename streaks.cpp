#include "streaks.h"
#include  "utils.h"


using namespace std;
using namespace PYTHON_FUNCTION;
using namespace UTILS;

namespace INTERNAL_CHECKS
{


	inline float rsc_get_straight_string_threshold(CMetVar& st_var, boost::gregorian::date   start, boost::gregorian::date   end, float reporting , float old_threshold)
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
		float threshold = get_critical_values(string_length, 1, 1, old_threshold);
		return  threshold;
		 
	}
	
	void rsc_annual_string_expectance( CMaskedArray& all_filtered, const std::vector<int>& value_starts, const std::vector<int>& value_lengths, std::valarray<float>& flags, boost::gregorian::date  start, boost::gregorian::date end, CMetVar& st_var, std::vector<int> times)
	{
		vector<int> month_starts;
		UTILS::month_starts(start, end,month_starts);  //  FONCTION RESHAPE
		vector<valarray<int>> m_month_starts;
		int iteration = 1;
		valarray<int> month(12);
		for (int i : month_starts)
		{
			if (iteration <= 12)
			{
				month[iteration-1]=i;
				iteration++;
			}
			else
			{
				m_month_starts.push_back(month);
				month.resize(12);
				month[0] = i;
				iteration = 2;
			}
		}
		m_month_starts.push_back(month);

		valarray < float > year_proportions(Cast<float>(st_var.getMdi()), m_month_starts.size());
		valarray<float> year(all_filtered.size());
		// churn through each year in turn
		for (int y = 0; y < m_month_starts.size(); y++)
		{
			if (y != m_month_starts.size() - 1)
				year = all_filtered.data()[std::slice(m_month_starts[y][0], m_month_starts[y + 1][0], 1)];
			else
				year = all_filtered.data()[std::slice(m_month_starts[y][0], m_month_starts.size() - m_month_starts[y][0], 1)];

			

		}

	}

	void rsc_straight_strings(CMetVar& st_var, std::vector<int> times, int n_obs, int n_days, boost::gregorian::date  start, boost::gregorian::date end, map<float, float> WIND_MIN_VALUE, bool wind, float reporting, bool dynamic)
	{
		float threshold;
		CMaskedArray all_filtered = apply_filter_flags(st_var);
		if (st_var.getName() == "winddirs")
		{
			//remove calm periods for this check
			CMetVar wd_st_var = st_var;
			valarray<size_t> calms = npwhere(st_var.getData(), float(0), '=');  //True calms have direction set to 0, northerlies to 360
			wd_st_var.setData(calms,Cast<float>( wd_st_var.getMdi()));

			if (dynamic)
			{
				threshold = rsc_get_straight_string_threshold(wd_st_var, start, end, reporting, float(n_obs));
				if (threshold < n_obs)  n_obs = int(threshold);
			}
			all_filtered = apply_filter_flags(wd_st_var); // calms have been removed
		}
		else
		{
			if (dynamic)
			{
				threshold = rsc_get_straight_string_threshold(st_var, start, end, reporting, float(n_obs));
				if (threshold < n_obs)  n_obs = int(threshold);
			}
		}
		valarray<float> flags(0., all_filtered.size());
		//Look for continuous straight strings

		float prev_value = Cast<float>(st_var.getMdi());
		vector<int> string_points;
		//storage for excess over years
		vector<int> values_starts;
		vector<int> values_lengths;

		int o = 0;
		for (float obs : all_filtered.data())
		{
			if (all_filtered.mask()[o] == false)
			{
				if (obs != prev_value)
				{
					if (st_var.getName() == "winddirs" && prev_value == 0);

					else
					{
						//if different value to before, which is long enough (and large enough for Wind)
						if (string_points.size() >= 10)
						{
							if (wind == false || (wind == true && prev_value > WIND_MIN_VALUE[reporting]))
							{
								//note start and length for the annual excess test
								values_starts.push_back(string_points[0]);
								values_lengths.push_back(string_points.size());

								int time_diff = times[string_points[string_points.size() - 1]] - times[string_points[0]];
								
								//if length above threshold and spread over sufficient time frame, flag

								if (string_points.size() >= n_obs || time_diff >= n_days * 24) //measuring time in hours
								{
									valarray<bool> dummy(false, flags.size());
									for (int val: string_points)
									{
										dummy[val] = true;
									}
									flags[dummy] = 1;
								}
							}
						}
					}
					string_points.clear();
					string_points.push_back(o);
				}
				else
				{
					//if same value as before, note and continue
					string_points.push_back(o);
				}
				prev_value = obs;
			}
			o++;
		}


	}

	void rsc(CStation& station, std::vector<std::string> var_list, std::vector<vector<int>>  flag_col, boost::gregorian::date  start,
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

