#include "variance.h"

using namespace std;
using namespace UTILS;
using namespace boost;
using namespace PYTHON_FUNCTION;
using namespace boost::numeric::ublas;


namespace INTERNAL_CHECKS
{
	void evc(CStation &station, std::vector<std::string> variable_list, std::vector<int> flag_col, boost::gregorian::date start, boost::gregorian::date  end, std::ofstream& logfile, bool idl )
	{
		//very similar to climatological check - ensure that not duplicating
		int v = 0;
		for (string variable : variable_list)
		{
			CMetVar st_var = station.getMetvar(variable);
			float reporting_resolution = reporting_accuracy(apply_filter_flags(st_var).compressed());

			int  reporting_freq = reporting_frequency(apply_filter_flags(st_var));
			map<int, int> month_ranges = month_starts_in_pairs(start, end);
			// array de taille 12*43*2 où chaque ligne correspond à un mois
			std::vector<std::valarray<pair<int, int>>> month_ranges_years;
			int taille = int(month_ranges.size() / 12);
			std::valarray<pair<int, int>> month(taille);
			int index = 0;
			int compteur = 0;
			int iteration = 1;
			map<int, int>::iterator month_it = month_ranges.begin();
			for (int i = 0; i < month_ranges.size(); i += 12)
			{
				if (iteration <= taille && i < month_ranges.size() && compteur < 12)
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
					month.resize(taille);
					index = 0;
					i = month_ranges_years.size();
					month_it = month_ranges.begin();
					std::advance(month_it, i);
					month[index++] = make_pair(month_it->first, month_it->second);
					std::advance(month_it, 12);
					iteration = 2;
				}
				if (i + 12 >= month_ranges.size() && compteur < 12)
				{
					i = month_ranges_years.size();
					month_it = month_ranges.begin();
					std::advance(month_it, i + 1);
				}
			}

			std::vector<varrayfloat> month_data_count;
			for (size_t i = 0; i < 12; ++i)
			{
				varrayfloat dum(st_var.getAllData().m_fill_value, taille);
				month_data_count.push_back(dum);
			}

			for (size_t month = 0; month < 12; ++month)
			{
				//set up hourly climatologies
				varrayfloat hourly_clims (st_var.getAllData().m_fill_value, 24);
				std::vector<int> year_ids;
				std::vector<CMaskedArray<float>> this_month;
				
				concatenate_months(month_ranges_years[v], st_var.getData(), this_month, year_ids, month_data_count[month], Cast<float>(st_var.getMdi()), true);

				//winsorize and get hourly climatology 
				for (size_t h = 0; h < 24; ++month)
				{
					CMaskedArray<float> this_hour = this_month[h];
					//need to have data if this is going to work!
					if (this_hour.compressed().size() > 0)
					{
						//winsorize & climatologies - done to match IDL
						if (idl)
						{
							varrayfloat dummy(this_hour.compressed().size() + 1);

							for (size_t i = 0; i < dummy.size() - 1; ++i)
								dummy[i] = this_hour.compressed()[i];
							dummy[dummy.size() - 1] = float(-999999);
							winsorize(dummy, 0.05, idl);
							CMaskedArray<float> this_hour_winsorized = CMaskedArray<float>(dummy);
							hourly_clims[h] = this_hour_winsorized.ma_sum() / (this_hour_winsorized.size() - 1);

						}
						else
						{
							varrayfloat dummy = this_hour.compressed();
							winsorize(dummy, 0.05, idl);
							CMaskedArray<float> this_hour_winsorized = CMaskedArray<float>(dummy);
							hourly_clims[h] = this_hour_winsorized.ma_mean();
						}
					}
				}
				//CMaskedArray<float> Hourly_Clims = ma_masked_where(hourly_clims,st_var.getAllData().fill_value(), hourly_clims);
				valarray<varrayfloat> anomalies(this_month.size());
				for (size_t i = 0; i < this_month.size(); ++i)
				{
					//anomalies[i] = this_month[i].m_data - Hourly_Clims.m_data;
				}
				//extract IQR of anomalies(using 1 / 2 value to match IDL)
				varraysize tailles(anomalies.size());
				for (size_t i = 0; i < tailles.size(); i++)
				{
					tailles[i] = CompressedSize(anomalies[i], Cast<float>(st_var.getMdi()));
				}
				//float iqr;
			}
			



			v++;
		}

	}
}