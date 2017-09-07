#include"utils.h"
#include "CStation.h"
#include<vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include <string>
#include "python_function.h"
#include <valarray> 
#include <boost/numeric/ublas/matrix.hpp>
#include <algorithm>
#include<fstream>
#include <ctime>
using namespace std;
using namespace boost;
using namespace PYTHON_FUNCTION;
namespace UTILS
{
	/*
		Returns locations of month starts(using hours as index)
		*/
	inline void month_starts(boost::gregorian::date start, boost::gregorian::date end, vector<int> &month_locs)
	{

		boost::gregorian::date Date = start;
		while (Date < end)
		{
			boost::gregorian::date_duration difference = Date - start;
			month_locs.push_back(difference.days() * 24);
			//increment counter
			if (Date.month() < 9)
			{
				/*cout << Date.month() << "";
				cout << to_string(Date.year()) + "0" + to_string((Date.month() + 1)) + "01" << endl;*/
				Date = boost::gregorian::date_from_iso_string(to_string(Date.year()) + "0" + to_string((Date.month() + 1)) + "01");
			}
			else if (Date.month() >= 9 && Date.month() < 12)
			{
				/*cout << Date.month() << "";
				cout << to_string(Date.year()) + to_string((Date.month() + 1)) + "01" << endl;*/
				Date = boost::gregorian::date_from_iso_string(to_string(Date.year()) + to_string((Date.month() + 1)) + "01");
			}
			else
			{
				/*cout << Date.month() << "";
				cout << to_string(Date.year()+1) + "0101" << endl;*/
				Date = boost::gregorian::date_from_iso_string(to_string((Date.year() + 1)) + "0101");
			}

		}

	}
	/*
		Create array of month start / end pairs
		: param datetime start : start of data
		: param datetime end : end of data
		: returns : month_ranges : Nx2 array
		*/
	inline map<int, int> month_starts_in_pairs(boost::gregorian::date start, boost::gregorian::date end)
	{
		//set up the arrays of month start locations
		vector<int> m_starts;
		month_starts(start, end, m_starts);

		map<int, int > month_ranges;

		for (int i = 0; i < m_starts.size() - 1; i++)
		{
			month_ranges[m_starts[i]] = m_starts[i + 1];
		}
		boost::gregorian::date_duration difference = end - start;

		month_ranges[m_starts[m_starts.size() - 1]] = difference.days() * 24;

		return month_ranges;
	}

	valarray<bool> create_fulltimes(CStation& station, vector<string> var_list, boost::gregorian::date start, boost::gregorian::date end, vector<string> opt_var_list, bool do_input_station_id, bool do_qc_flags, bool do_flagged_obs)
	{
		//expand the time axis of the variables
		boost::gregorian::date_duration DaysBetween = end - start;
		valarray<float> fulltimes;

		//adjust if input netCDF file has different start date to desired
		string time_units = station.getTime_units();
		//Extraire la date de la chaîne
		time_units = time_units.substr(time_units.find("since "));
		time_units = time_units.substr(time_units.find(" "));
		time_units.erase(std::remove(time_units.begin(), time_units.end(), ' '), time_units.end());
		boost::gregorian::date  netcdf_start = boost::gregorian::date_from_iso_string(time_units);
		boost::gregorian::date_duration offset = start - netcdf_start;
		fulltimes = PYTHON_FUNCTION::arange<float>(DaysBetween.days() * 24, offset.days() * 24);

		valarray<bool> match, match_reverse;

		match = PYTHON_FUNCTION::in1D<float, float>(fulltimes, station.getMetvar("time")->getData());
		match_reverse = PYTHON_FUNCTION::in1D<float, float>(station.getMetvar("time")->getData(), fulltimes);

		//if optional/carry through variables given, then set to extract these too
		vector<string> full_var_list;
		copy(var_list.begin(), var_list.end(), std::back_inserter(full_var_list));

		if (opt_var_list.size() != 0)
		{
			copy(opt_var_list.begin(), opt_var_list.end(), std::back_inserter(full_var_list));
		}
		vector<string> final_var_list;
		copy(full_var_list.begin(), full_var_list.end(), std::back_inserter(final_var_list));
		//if (do_input_station_id)  final_var_list.push_back("input_station_id");

		for (vector<string>::iterator variable = final_var_list.begin(); variable != final_var_list.end(); variable++)
		{
			CMetVar* st_var = station.getMetvar(*variable);
			//use masked arrays for ease of filtering later

			std::valarray<float> valmask(static_cast<float>(atof(st_var->getMdi().c_str())), fulltimes.size());
			if (match_reverse.size() != 0)
			{
				valmask = st_var->getData()[match_reverse];

			}
			//but re-mask those filled timestamps which have missing data
			std::valarray<bool> mask_where(false, valmask.size());
			for (int i = 0; i < valmask.size(); i++)
			{
				if (valmask[i] != static_cast<float>(atof(st_var->getMdi().c_str())))
				{
					mask_where[i] = true;
				}
			}

			station.getMetvar(*variable)->setMaskedData(valmask[mask_where]);

			if (find(var_list.begin(), var_list.end(), *variable) != var_list.end() && do_flagged_obs == true)
			{
				//flagged values
				valmask = static_cast<float>(atof(st_var->getMdi().c_str()));
				std::valarray<float> v1;
				if (st_var->getFlagged_obs().size() != 0)
				{
					v1 = st_var->getFlagged_obs()[match_reverse];
					valmask = v1;
				}
				st_var->setFlagged_obs(valmask);
				// flags - for filtering

				valmask = static_cast<float>(atof(st_var->getMdi().c_str()));
				if (st_var->getFlags().size() != 0)
				{
					v1 = st_var->getFlags()[match_reverse];
					valmask = v1;
					station.getMetvar(*variable)->setFlags(valmask);
				}
			}

		}
		//do the QC flags, using try / except
		if (do_qc_flags == true)
		{
			/*try
			{


			}*/
		}
		//working in fulltimes throughout and filter by missing
		if (offset.days() != 0)	fulltimes = PYTHON_FUNCTION::arange<float>(DaysBetween.days() * 24, offset.days() * 24);
		station.getMetvar("time")->setData(fulltimes);
		return match;
	}

	/*
	Return reporting accuracy & reporting frequency for variable

	: param obj st_var :	station variable object
	: param datetime start : start of data series
	: param datatime end : e	nd of data series

	: returns :
	reporting_stats - Nx2 array, one pair for each month
	'''
	*/

	void monthly_reporting_statistics(CMetVar& st_var, boost::gregorian::date start, boost::gregorian::date end)
	{
		map<int, int> monthly_ranges;
		monthly_ranges = month_starts_in_pairs(start, end);
	}
	void print_flagged_obs_number(std::ofstream& logfile, string test, string variable, int nflags, bool noWrite)
	{
		if (!noWrite)
			logfile << test << " Check Flags :" << variable << " :" << nflags;
	}
	/*
	Apply these flags to all variables
	: param object station : the station object to be processed
	: param list all_variables : the variables where the flags are to be applied
	: param file logfile : logfile to store outputs
	: returns :
	*/
	void apply_flags_all_variables(CStation& station, vector<string> full_variable_list, int flag_col, ofstream& logfile, string test)
	{

	}
	void append_history(CStation& station, string text)
	{
		time_t _time;
		struct tm timeInfo;
		char format[32];
		time(&_time);
		localtime_s(&timeInfo, &_time);
		strftime(format, 32, "%Y-%m-%d %H-%M", &timeInfo);
		station.setHistory(station.getHistory() + text + format);
	}
	/*
		Return the data masked by the flags
		Retourner un array où les valeurs qui correspondent aux indices
		où st_var.flags==1 sont masquées
		*/

	valarray<float> apply_filter_flags(CMetVar *st_var)
	{

		return  PYTHON_FUNCTION::masked_values<float>(st_var->getData(), 1);
	}

	float reporting_accuracy(valarray<float> good_values, bool winddir)
	{
		float resolution = -1;
		if (winddir)
		{
			resolution = 1;
			if (good_values.size() > 0)
			{
				valarray<float> binEdges = PYTHON_FUNCTION::arange<float>(362, 0);
				valarray<float> hist = PYTHON_FUNCTION::histogram<float>(good_values, binEdges);
				//normalise
				hist = hist / hist.sum();

				valarray<float> hist1 = hist[PYTHON_FUNCTION::arange<size_t>(360 + 90, 90, 90)];
				if (hist1.sum() >= 0.6) resolution = 90;
				hist1 = hist[PYTHON_FUNCTION::arange<size_t>(360 + 45, 45, 45)];
				if (hist1.sum() >= 0.6) resolution = 45;
				hist1 = hist[PYTHON_FUNCTION::Arange(360 + 22.5, 22.5, 22.5)];
				if (hist1.sum() >= 0.6) resolution = 10;

				cout << "Wind dir resolution =" << resolution << " degrees" << endl;
			}
		}
		else
		{
			if (good_values.size() > 0)
			{
				valarray<float> remainders = std::abs(good_values) - std::abs(good_values.apply(UTILS::MyApplyRoundFunc));
				valarray<float> binEdges = PYTHON_FUNCTION::arange<float>(1.05, -0.05, 0.0);
				valarray<float> hist = PYTHON_FUNCTION::histogram<float>(good_values, binEdges);

				hist = hist / hist.sum();
				if (hist[0] >= 0.3)
					if (hist[5] >= 0.15)
						resolution = 0.5;
					else
						resolution = 1.0;
				else
					resolution = 0.1;

			}
		}
		return resolution;
	}

	/* create bins and bin centres from data
		given bin width covers entire range */

	void create_bins(valarray<float> indata, float binwidth, valarray<float> &bins, valarray<float>  &bincenters)
	{
		//set up the bins
		int bmins = int(floor(indata.min()));
		int bmax = int(ceil(indata.max()));
		bins = PYTHON_FUNCTION::arange<float>(bmax + (3. * binwidth), bmins - binwidth, binwidth);
		for (int i = 0; i < bins.size() - 1; i++)
			bincenters[i] = 0.5*(bins[i] + bins[i + 1]);
	}

	

}
