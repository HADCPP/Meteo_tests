#include "climatological.h"


using namespace std;
using namespace UTILS;
using namespace boost;
using namespace PYTHON_FUNCTION;

namespace INTERNAL_CHECKS
{
	void coc(CStation& station, std::vector<std::string> variable_list, std::vector<int> flag_col, boost::gregorian::date  start, boost::gregorian::date end, std::ofstream&  logfile, bool idl )
	{
		int v = 0;
		for (string variable : variable_list)
		{
			CMetVar st_var = station.getMetvar(variable);
			CMaskedArray<float> all_filtered = apply_filter_flags(st_var);

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
				if (iteration <= taille && i < month_ranges.size() && compteur<12)
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
				if (i + 12 >= month_ranges.size() && compteur<12)
				{
					i = month_ranges_years.size();
					month_it = month_ranges.begin();
					std::advance(month_it, i + 1);
				}

			}

			for (int month = 0; month < 12; ++month)
			{
				varrayfloat  hourly_climatologies(Cast<float>(st_var.getMdi()), 24);
				//append all e.g.Januaries together
				vector<int> year_ids;
				varrayfloat datacount( month_ranges.size());
				vector<CMaskedArray<float>> this_month;
				vector<CMaskedArray<float>> this_month_filtered;
				concatenate_months(month_ranges_years[v], all_filtered.m_data, this_month, year_ids, datacount, Cast<float>(st_var.getMdi()), true);
				concatenate_months(month_ranges_years[v], st_var.getData(), this_month, year_ids, datacount, Cast<float>(st_var.getMdi()), true);
				
				//if fixed climatology period, sort this here

				//this_month = this_month.reshape(-1,24)
				//this_month_filtered = this_month_filtered.reshape(-1, 24)

				//get hourly climatology for each month
				for (int hour = 0; hour < 24; ++hour)
				{
					CMaskedArray<float> this_hour = this_month[hour];
					//need to have data if this is going to work!
					if (this_hour.compressed().size()>0)
					{
						//winsorize & climatologies - done to match IDL
						if (idl)
						{
							varrayfloat dummy(this_hour.compressed().size() + 1);

							for (size_t i = 0; i < dummy.size() - 1; ++i)
								dummy[i] = this_hour.compressed()[i];
							dummy[dummy.size() - 1] = float(-999999);
							winsorize(dummy, 0.05, idl);
							CMaskedArray<float> this_hour = CMaskedArray<float>(dummy);
							hourly_climatologies[hour] = this_hour.ma_sum() / (this_hour.size() - 1);

						}
						else
						{
							varrayfloat dummy = this_hour.compressed();
							winsorize(dummy, 0.05, idl);
							CMaskedArray<float> this_hour = CMaskedArray<float>(dummy);
							hourly_climatologies[hour] = this_hour.ma_mean();
						}
					}
				}
				//Verifier si this_month contient des valeurs non masquées
				valarray<bool> v_mask(this_month.size());
				for (size_t i = 0; i < this_month.size(); ++i)
					v_mask[i] = this_month[i].m_mask.max();
				if (v_mask.max()) //equivalent de len(this_month.compressed()) > 0
				{
					//can get stations with few obs in a particular variable.

					//anomalise each hour over month appropriately
					valarray<varrayfloat> anomalies(this_month.size()), anomalies_filtered(this_month_filtered.size());

					for (size_t i = 0; i < this_month.size(); ++i)
					{
						anomalies[i] = this_month[i].m_data - hourly_climatologies;
					}
					for (size_t i = 0; i < this_month_filtered.size(); ++i)
					{
						anomalies_filtered[i] = this_month_filtered[i].m_data - hourly_climatologies;
					}

					varraysize taille(anomalies.size());
					for (size_t i = 0; i < taille.size(); i++)
					{
						taille[i] = CompressedSize(anomalies[i], Cast<float>(st_var.getMdi()));
					}
					//float iqr;
					if (taille.sum() >= 10)
					{

					}
						
				}
			}

			v++;
		}
	}
}