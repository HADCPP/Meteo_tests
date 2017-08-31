#include "Diurnal_cycle.h"
#include "utils.h"
#include <vector>
#include <string>
#include <iostream>
#include <valarray>
#include "python_function.h"


using namespace std;

namespace DIURNAL
{


	bool dcc_quartile_check(const valarray<float> & day)
	{
		valarray<int> quartile_has_data(0, 4);
		valarray<size_t> quart_start; //quart_end;
		quart_start = PYTHON_FUNCTION::Arange(HOURS, 0, HOURS / 4.);
		//quart_end = PYTHON_FUNCTION::Arange(HOURS + 6, 6, HOURS / 4.);
		for (int q : {1, 2, 3, 4})
		{
			if (day[std::slice(quart_start[q], 6, 1)].size() > 0)
				quartile_has_data[q] = 1;
		}
		if (quartile_has_data.sum() >= 3) return true;
		else return false;
	}

	


	void dcc(station& stat, vector<string>variable_list, vector<string> full_variable_list, vector<int>flag_col,
		ofstream &logfile)
	{
		//list of flags for each variable
		valarray<float> diurnal_flags;
		int v = 0;
		for (string variable : variable_list)
		{
			MetVar *st_var = stat.getMetvar(variable);
			valarray<float> filtered_data = UTILS::apply_filter_flags(st_var);
			vector<valarray<float>> v_filtered_data;
			int iteration = 1;
			valarray<float> h_filter_data;
			int val_index = 0;
			for (int i = 0; i < filtered_data.size();i++)
			{
				if (iteration <= 24)
				{
					h_filter_data[val_index] = filtered_data[i];
					iteration++;
				}
				else
				{
					v_filtered_data.push_back(h_filter_data);
					val_index=0;
					
					h_filter_data[val_index] = filtered_data[i];
					iteration = 2;
				}

			}
			v_filtered_data.push_back(h_filter_data);
			int number_of_days = v_filtered_data.size();

			valarray<int> diurnal_best_fits(INTMDI, number_of_days);
			valarray<int> diurnal_uncertainties(INTMDI, number_of_days);

			int d = 0;
			for (valarray<float> day : v_filtered_data)
			{
				//enough observations and have large enough diurnal range
				if (day.size() > OBS_PER_DAY)
				{
					int obs_daily_range = int(day.max() - day.min());
					if (obs_daily_range >= DAILY_RANGE)
					{
						valarray<float> scaled_sine,diffs;
						if (dcc_quartile_check(day))
						{
							scaled_sine = dcc_make_sine().apply([](float val){ return ((val + 1) / 2); });
							scaled_sine *= obs_daily_range;
							scaled_sine += day.min();
							diffs.resize(HOURS, 0.);
							//Find differences for each shifted sine --> cost function
							for (int h = 0; h < HOURS; h++)
							{
								valarray<float> data = day - scaled_sine;
								data = data.apply([](float val){ return std::abs(val); });
								diffs[h] = data.sum(); // check this array
								PYTHON_FUNCTION::np_roll(scaled_sine, 1);
							}
							diurnal_best_fits[d] = PYTHON_FUNCTION::np_argmin(diffs);
							//default uncertainty is the average time resolution of the data
							diurnal_uncertainties[d] = int(round(float(HOURS) / day.size()));
							if (DYNAMIC_DIURNAL)
							{
								float critical_value = diffs.min() + ((diffs.max() - diffs.min())*0.33);
								//centre so minimum in middle
								PYTHON_FUNCTION::np_roll(diffs, 11 - diurnal_best_fits[d]);
								int  uncertainty = 1;
								while (uncertainty < 11)
								{
									if (diffs[11 - uncertainty] > critical_value && diffs[11 + uncertainty] > critical_value)
									{
										//break if both sides greater than critical difference when counting outwards
										break;
									}
									uncertainty++;
								}
								//check if uncertainty greater than time resolution for day
								if (uncertainty > diurnal_uncertainties[d])
									diurnal_uncertainties[d] = uncertainty;
							}
						}

					}
				}
				d++;
			}
			//For each uncertainty range (1-6h) find median of cycle offset
			valarray<int> best_fits(-9, 6);
			for (int h = 0; h < 6; h++)
			{
				valarray<size_t> locs = PYTHON_FUNCTION::npwhere<int>(diurnal_uncertainties, h + 1,'=');
				if (locs.size() > 300)
				{

				}
			}
		}
	}
}