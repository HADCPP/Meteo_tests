#include "spike.h"

using namespace std;
using namespace UTILS;
using namespace boost;
using namespace PYTHON_FUNCTION;
using namespace boost::numeric::ublas;


namespace INTERNAL_CHECKS
{
	void sc(CStation &station, std::vector<std::string> variable_list, std::vector<int> flag_col, boost::gregorian::date  start,
		boost::gregorian::date end, std::ofstream&  logfile, bool second)
	{

		cout << "refactor" << endl;

		int v = 0;
		for (string variable : variable_list)
		{

			
			varrayfloat flags = station.getQc_flags(flag_col[v]);
			int prev_flag_number = 0;
			if (second)
			{
				// count currently existing flags :
				varrayfloat dummy(flags.size());
				dummy = flags[flags != float(0)];
				prev_flag_number = dummy.size();
			}
			CMetVar st_var = station.getMetvar(variable);
			CMaskedArray<float>  all_filtered = apply_filter_flags(st_var);
			float reporting_resolution = reporting_accuracy(apply_filter_flags(st_var).data());
			//to match IDL system - should never be called as would mean no data
			if (reporting_resolution == -1)  reporting_resolution = 1;

			map<int, int> month_ranges = month_starts_in_pairs(start, end);
			// array de taille 12*86*2 où chaque ligne correspond à un mois
			std::vector<std::valarray<pair<int, int>>> month_ranges_years;
			std::valarray<pair<int, int>> month(86);
			int index = 0;
			int compteur = 0;
			int iteration = 1;
			map<int, int>::iterator month_it = month_ranges.begin();
			for (int i = 0; i < month_ranges.size(); i += 12)
			{
				if (iteration <= 86 && i < month_ranges.size() && compteur<12)
				{
					month[index++] = make_pair(month_it->first, month_it->second);
					iteration++;
					if (i + 12 < month_ranges.size()) std::advance(month_it, 12);
				}
				else
				{
					if (compteur == 12) break;
					month_ranges_years.push_back(month);
					compteur++;
					month.resize(86);
					index = 0;
					i = month_ranges_years.size();
					month_it = month_ranges.begin();
					std::advance(month_it, i);
					month[index++] = make_pair(month_it->first, month_it->second);
					std::advance(month_it, 12);
					iteration = 2;
				}
				if (i + 12 >= month_ranges.size() && compteur<12)
				{
					i = month_ranges_years.size();
					month_it = month_ranges.begin();
					std::advance(month_it, i + 1);
				}

			}

			varraysize good = npwhere<bool>(all_filtered.mask(), false, "=");

			CMaskedArray<int> full_time_diffs = CMaskedArray<int>::CMaskedArray(float(0), all_filtered.size());
			full_time_diffs.Setmask(all_filtered.mask());
			valarray<int> time_data(station.getTime_data().size());
			std::copy(station.getTime_data().begin(), station.getTime_data().end(), std::begin(time_data));
			std::valarray<int> dummy(good.size());
			time_data = time_data[good];
			dummy[good] = time_data[std::slice(1, good.size() - 1, 1)];
			dummy[good] -= time_data[std::slice(0, good.size() - 1, 1)];
			dummy[good[good.size() - 1]] = dummy[good[0]];
			full_time_diffs.setData(good,dummy);
			
			//develop critical values using clean values
			cout << "sort the differencing if values were flagged rather than missing" << endl;

			CMaskedArray<float> full_filtered_diffs = CMaskedArray<float>::CMaskedArray(float(0), all_filtered.size());
			full_filtered_diffs.Setmask(all_filtered.mask());
			varrayfloat dumm(good.size());
			dumm[good] = all_filtered.compressed()[std::slice(1, all_filtered.compressed().size() - 1, 1)];
			dumm[good] -= all_filtered.compressed()[std::slice(0, all_filtered.compressed().size() - 1, 1)];
			if (all_filtered.compressed().size() != good.size()) dumm[good.size()- 1]= dumm[good[0]];
			full_filtered_diffs.setData(good, dumm);

			//test all values
			varraysize good_to_uncompress = npwhere<bool>(st_var.getAllData().mask(), false, "=");
			CMaskedArray<float> full_value_diffs = CMaskedArray<float>::CMaskedArray(float(0),st_var.getData().size());
			full_value_diffs.Setmask(st_var.getAllData().mask());
			size_t taille = st_var.getAllData().compressed().size();
			dumm.resize(taille);
			dumm[good_to_uncompress] = st_var.getAllData().compressed()[std::slice(1, taille - 1, 1)];
			dumm[good_to_uncompress] -= st_var.getAllData().compressed()[std::slice(0, taille -1, 1)];

			// convert to compressed time to match IDL
			varrayfloat value_diffs = full_value_diffs.compressed();
			valarray<int> time_diffs = full_time_diffs.compressed();
			varrayfloat filtered_diffs = full_filtered_diffs.compressed();
			flags = flags[good_to_uncompress];
			matrix<float> critical_values(9, 12);
			for (size_t i = 0; i < critical_values.size1(); ++i)
				for (size_t j = 0; j < critical_values.size2(); ++j)
					critical_values(i, j) = Cast<float>(st_var.getMdi());
			//link observation to calendar month

			varrayfloat month_locs(float(0), full_time_diffs.size());

			valarray<int> this_month_time_diff(full_time_diffs.size());
			varrayfloat this_month_filtered_diff(full_filtered_diffs.size());
			for (size_t month = 0; month < 12; month++)
			{
				for (size_t year = 0; year < int(month_ranges.size() / 12); month++)
				{
					if (year == 0)
					{
						this_month_time_diff = full_time_diffs.data()[std::slice(month_ranges_years[month][year].first,
							month_ranges_years[month][year].second - month_ranges_years[month][year].first, 1)];
						this_month_filtered_diff = full_filtered_diffs.data()[std::slice(month_ranges_years[month][year].first,
							month_ranges_years[month][year].second - month_ranges_years[month][year].first, 1)];
					}
					else
					{
						this_month_time_diff += full_time_diffs.data()[std::slice(month_ranges_years[month][year].first,
							month_ranges_years[month][year].second - month_ranges_years[month][year].first, 1)];
						this_month_filtered_diff += full_filtered_diffs.data()[std::slice(month_ranges_years[month][year].first,
							month_ranges_years[month][year].second - month_ranges_years[month][year].first, 1)];
					}
					month_locs[std::slice(month_ranges_years[month][year].first,
						month_ranges_years[month][year].second - month_ranges_years[month][year].first, 1)] = month;

				}
				for (size_t delta = 1; month < 9; delta++)
				{
					float iqr;
					varraysize locs = npwhere<int>(this_month_time_diff, delta, "=");
					if (locs.size() >= 100)
					{
						dumm.resize(locs.size());
						dumm = this_month_filtered_diff[locs];
						iqr = IQR(dumm);
						if (iqr == float(0) && delta == 1)
							critical_values(delta - 1, month) = float(6);
						else if (iqr == float(0))
							critical_values(delta - 1, month) = Cast<float>(st_var.getMdi());
						else
							critical_values(delta - 1, month) = 6 * iqr;

						float threshold = get_critical_values<float>(dumm, float(0), 0.5, critical_values(delta - 1, month));
						if (threshold < critical_values(delta - 1, month)) critical_values(delta - 1, month) = threshold;
						cout << critical_values(delta - 1, month) << "" << iqr << "" << 6 * iqr << endl;
					}
				}
			}
			month_locs = month_locs[good_to_uncompress];
			//not less than 5x reporting accuracy
			for (size_t i = 0; i < critical_values.size1(); ++i)
				for (size_t j = 0; j < critical_values.size2(); ++j)
				{
					if (critical_values(i, j) != Cast<float>(st_var.getMdi()))
					{

						if (critical_values(i, j) <= 5 * reporting_resolution)
							critical_values(i, j) = 5 * reporting_resolution;

					}
				}

			//check hourly against 2 hourly, if <2 / 3 the increase to avoid crazy rejection rate
			for (int month = 0; month < 12; month++)
			{
				if (critical_values(0, month) != Cast<float>(st_var.getMdi()) && critical_values(1, month) != Cast<float>(st_var.getMdi()))
					if (critical_values(0, month) / critical_values(1, month) <= 0.66)
						critical_values(0, month) = 0.66*critical_values(1, month);

			}
			//get time differences for unfiltered data
			full_time_diffs.resize(st_var.getData().size());

			full_time_diffs.Setmask(st_var.getAllData().mask());
			time_data = time_data[good_to_uncompress];
			dummy[good_to_uncompress] = time_data[std::slice(1, good.size() - 1, 1)];
			dummy[good_to_uncompress] -= time_data[std::slice(0, good.size() - 1, 1)];
			dummy[good_to_uncompress[good_to_uncompress.size() - 1]] = dummy[good_to_uncompress[0]];
			full_time_diffs.setData(good_to_uncompress, dummy);
			dummy.free();
			time_data.free();
			time_diffs.resize(full_time_diffs.size());
			time_diffs = full_time_diffs.compressed();

			// go through each difference, identify which month it is in if passes spike thresholds 

			// spikes at the beginning or ends of sections

			for (size_t t = 0; t < Arange<int>(time_diffs.size(), 0, 1).size(); ++t)
			{
				float median_diff;
				if (std::abs(time_diffs[t - 1]) > 240 && std::abs(time_diffs[t]) < 3)
				{
					//10 days before but short gap thereafter

					taille = good_to_uncompress.size() - (t);
					varraysize indice = good_to_uncompress[std::slice(t + 1, taille - 1, 1)];
					dumm.resize(taille);
					dumm = st_var.getData()[indice];
					CMaskedArray<float> next_values = CMaskedArray<float>::CMaskedArray(dumm);
					valarray<bool> mask = st_var.getAllData().mask()[indice];
					next_values.Setmask(mask);
					
					mask.free();
					
					good = npwhere(next_values.mask(), false, "=");
					indice = good[std::slice(0, 10, 1)];
					dumm = next_values.data()[indice];
					float next_median = idl_median(dumm);
					float next_diff = std::abs(value_diffs[t]);  //out of spike
					median_diff = std::abs(next_median - st_var.getAllData().data()[good_to_uncompress[t]]);

					if (critical_values(time_diffs[t] - 1, month_locs[t]) != Cast<float>(st_var.getMdi()))
					{
						//jump from spike > critical but average after < critical / 2
						if (std::abs(median_diff) < critical_values(time_diffs[t] - 1, month_locs[t]) / 2. &&
							std::abs(next_diff) > critical_values(time_diffs[t] - 1, month_locs[t]))
						{
							flags[t] = 1;

						}
					}
				}
				else if (std::abs(time_diffs[t - 1]) < 3 && std::abs(time_diffs[t]) > 240)
				{
					//10 days after but short gap before
					taille = good_to_uncompress.size() - (t+1);
					varraysize indice = good_to_uncompress[std::slice(t - 1, taille - 1, 1)];
					dumm.resize(taille);
					dumm = st_var.getData()[indice];
					CMaskedArray<float> prev_values = CMaskedArray<float>::CMaskedArray(dumm);
					valarray<bool> mask = st_var.getAllData().mask()[indice];
					prev_values.Setmask(mask);

					mask.free();

					good = npwhere(prev_values.mask(), false, "=");

					indice = good[std::slice(good.size()-11, 10, 1)];
					dumm = prev_values.data()[indice];
					float prev_median = idl_median(dumm);
					float prev_diff = std::abs(value_diffs[t]);  //out of spike
					median_diff = std::abs(prev_median - st_var.getAllData().data()[good_to_uncompress[t]]);

					if (critical_values(time_diffs[t-1] - 1, month_locs[t]) != Cast<float>(st_var.getMdi()))
					{
						//jump from spike > critical but average before< critical / 2
						if (std::abs(median_diff) < critical_values(time_diffs[t-1] - 1, month_locs[t]) / 2. &&
							std::abs(prev_diff) > critical_values(time_diffs[t] - 1, month_locs[t]))
						{
							flags[t] = 1;

						}
					}
				}
			}

			for (size_t t = 0; t < Arange<int>(time_diffs.size(), 0, 1).size(); ++t)
			{
				for (int spk_len : {1, 2, 3})
				{
					if (t >= spk_len && t < time_diffs.size() - spk_len)
					{
						//check if time differences are appropriate, for multi-point spikes
						if ((std::abs(time_diffs[t - spk_len]) <= spk_len * 3) &&
							(std::abs(time_diffs[t]) <= spk_len * 3) &&
							(time_diffs[t - spk_len - 1] - 1 < spk_len * 3) &&
							(time_diffs[t + 1] - 1 < spk_len * 3) &&
							((spk_len == 1) ||
							((spk_len == 2) && (std::abs(time_diffs[t - spk_len + 1]) <= spk_len * 3)) ||
							((spk_len == 3) && (std::abs(time_diffs[t - spk_len + 1]) <= spk_len * 3) && (std::abs(time_diffs[t - spk_len + 2]) <= spk_len * 3))))
						{
							//check if differences are valid   
							if ((value_diffs[t - spk_len] != Cast<float>(st_var.getMdi())) &&
								(value_diffs[t - spk_len] != st_var.getFdi()) &&
								(critical_values(time_diffs[t - spk_len] - 1, month_locs[t]) != Cast<float>(st_var.getMdi())))
							{
								//if exceed critical values
								if (std::abs(value_diffs[t - spk_len]) >= critical_values(time_diffs[t - spk_len] - 1, month_locs[t]))
								{
									//are signs of two differences different
									if (std::copysignf(1, value_diffs[t]) != copysignf(1, value_diffs[t - spk_len]))
									{
										//are within spike differences small
										if ((spk_len == 1) ||
											((spk_len == 2) &&(std::abs(value_diffs[t - spk_len + 1]) < critical_values(time_diffs[t - spk_len + 1] - 1, month_locs[t]) / 2.)) ||
											((spk_len == 3) && (std::abs(value_diffs[t - spk_len + 1]) < critical_values(time_diffs[t - spk_len + 1] - 1,month_locs[t]) / 2.) &&
											(std::abs(value_diffs[t - spk_len + 2]) < critical_values(time_diffs[t - spk_len + 2] - 1, month_locs[t] / 2.))))
										{
											//check if following value is valid
											if (std::abs(value_diffs[t]) >= critical_values(time_diffs[t] - 1, month_locs[t]))
											{
												//test if surrounding differences below 1/2 critical value
												if (std::abs(value_diffs[t - spk_len - 1]) <= critical_values(time_diffs[t - spk_len - 1] - 1, month_locs[t]) / 2.) 
													if (std::abs(value_diffs[t + 1]) <= critical_values(time_diffs[t + 1] - 1, month_locs[t]) / 2.)
													{
														//set the flags
														flags[std::slice(t - spk_len + 1, t + 1 - t - spk_len + 1, 1)] = 1;

													}
											}
										}
									}
								}
							}
						}
					}
				}
			}
			station.setQc_flags(flags, good_to_uncompress, flag_col[v]);
			varraysize flag_locs = npwhere(station.getQc_flags()[flag_col[v]], float(0), "!");
			print_flagged_obs_number(logfile, "Spike", variable, flag_locs.size() - prev_flag_number);
			//copy flags into attribute
			st_var.setFlags(flag_locs, float(1));

			v++;
		}
		append_history(station, "Spike check");
	}
}