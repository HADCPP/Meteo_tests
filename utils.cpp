#include"utils.h"
#include "station.h"
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
using namespace dlib;
using namespace PYTHON_FUNCTION;
namespace UTILS
{
	/*
		Returns locations of month starts(using hours as index)
	*/
	typedef matrix<double, 1, 1> input_vector;
	typedef matrix<double, 2, 1> parameter_vector;


	void month_starts(boost::gregorian::date start, boost::gregorian::date end,std::vector<int>& month_locs)
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
	inline map<int,int> month_starts_in_pairs(boost::gregorian::date start, boost::gregorian::date end)
	{
		//set up the arrays of month start locations
		std::vector<int> m_starts;
		month_starts(start, end,m_starts);

		map<int,int > month_ranges; 
			
		for (int i =0 ; i < m_starts.size()-1; i++)
		{
			month_ranges[m_starts[i]] = m_starts[i + 1];
		}
		boost::gregorian::date_duration difference = end - start;
			
		month_ranges[m_starts[m_starts.size()-1]] = difference.days() * 24;

		return month_ranges;
	}

	valarray<bool> create_fulltimes(CStation& station, std::vector<string> var_list, boost::gregorian::date start, boost::gregorian::date end, std::vector<string> opt_var_list, bool do_input_station_id, bool do_qc_flags, bool do_flagged_obs)
	{
		//expand the time axis of the variables
		boost::gregorian::date_duration DaysBetween = end - start;
		valarray<float> fulltimes;
		
		//adjust if input netCDF file has different start date to desired
		string time_units = station.getTime_units();
		//Extraire la date de la cha�ne
		time_units = time_units.substr(time_units.find("since "));  
		time_units = time_units.substr(time_units.find(" "));
		time_units.erase(std::remove(time_units.begin(), time_units.end(), ' '), time_units.end());
		boost::gregorian::date  netcdf_start = boost::gregorian::date_from_iso_string(time_units);
		boost::gregorian::date_duration offset = start- netcdf_start;
		fulltimes = PYTHON_FUNCTION::arange<float>(DaysBetween.days() * 24, offset.days() * 24);

		valarray<bool> match, match_reverse;

		match = PYTHON_FUNCTION::in1D<float, float>(fulltimes, station.getMetvar("time").getData());
		match_reverse = PYTHON_FUNCTION::in1D<float, float>(station.getMetvar("time").getData(), fulltimes);

		//if optional/carry through variables given, then set to extract these too
		std::vector<string> full_var_list;
		copy(var_list.begin(), var_list.end(), std::back_inserter(full_var_list));

		if (opt_var_list.size() != 0)
		{
			copy(opt_var_list.begin(), opt_var_list.end(), std::back_inserter(full_var_list));
		}
		std::vector<string> final_var_list;
		copy(full_var_list.begin(), full_var_list.end(), std::back_inserter(final_var_list));
		//if (do_input_station_id)  final_var_list.push_back("input_station_id");

		for (std::vector<string>::iterator variable = final_var_list.begin(); variable != final_var_list.end(); variable++)
		{
			CMetVar& st_var = station.getMetvar(*variable);
			//use masked arrays for ease of filtering later
			
			std::valarray<float> valmask(static_cast<float>(atof(st_var.getMdi().c_str())), fulltimes.size());
			if (match_reverse.size()!= 0)	
			{
				valmask = st_var.getData()[match_reverse];
				 
			}
			//but re-mask those filled timestamps which have missing data
			std::valarray<bool> mask_where(false,valmask.size());
			for (int i = 0; i < valmask.size();i++)
			{
				if (valmask[i] != static_cast<float>(atof(st_var.getMdi().c_str())))
				{
					mask_where[i] = true;
				}	
			}
			
			station.getMetvar(*variable).setMaskedData(valmask[mask_where]);
						
			if (find(var_list.begin(), var_list.end(), *variable) != var_list.end() && do_flagged_obs == true)
			{
				//flagged values
				valmask = static_cast<float>(atof(st_var.getMdi().c_str()));
				std::valarray<float> v1;
				if (st_var.getFlagged_obs().size() != 0)
				{
					v1 = st_var.getFlagged_obs()[match_reverse];
					valmask = v1;
				}
				st_var.setFlagged_obs(valmask);
				// flags - for filtering

				valmask = static_cast<float>(atof(st_var.getMdi().c_str()));
				if (st_var.getFlags().size() != 0)
				{
					v1 = st_var.getFlags()[match_reverse];
					valmask = v1;
					station.getMetvar(*variable).setFlags(valmask);
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
		if (offset.days() != 0)	fulltimes = PYTHON_FUNCTION::arange<float>( DaysBetween.days() * 24, offset.days() * 24);
		station.getMetvar("time").setData(fulltimes);
		return match;
	}

		/*
		Return reporting accuracy & reporting frequency for variable

		: param obj st_var :	CStation variable object
		: param datetime start : start of data series
		: param datatime end : e	nd of data series

		: returns :
		reporting_stats - Nx2 array, one pair for each month
		'''
		*/

	void monthly_reporting_statistics(CMetVar st_var, boost::gregorian::date start, boost::gregorian::date end)
	{
		map<int, int> monthly_ranges;
		monthly_ranges= month_starts_in_pairs(start, end);
	}
	 void print_flagged_obs_number(std::ofstream& logfile, string test, string variable, int nflags, bool noWrite)
	{
		if (!noWrite)
		logfile << test<< " Check Flags :"<< variable  <<" :"<< nflags;
	}
	/*
	Apply these flags to all variables
		: param object CStation : the CStation object to be processed
		: param list all_variables : the variables where the flags are to be applied
		: param file logfile : logfile to store outputs
		: returns :
		*/
	 void apply_flags_all_variables(CStation& station, std::vector<string> full_variable_list, int flag_col, ofstream& logfile, string test)
	{

	}
	/*
	Applying windspeed flags to wind directions synergistically
    Called after every test which assess windspeeds

    :param object station: the station object to be processed
	*/
	void apply_windspeed_flags_to_winddir(CStation& station)
	{
		CMetVar & windspeeds = station.getMetvar("windspeeds");
		CMetVar & winddirs = station.getMetvar("winddirs");

		winddirs.setFlags(windspeeds.getFlags());
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
		Retourner un array o� les valeurs qui correspondent aux indices 
		o� st_var.flags==1 sont masqu�es
	*/

	CMaskedArray apply_filter_flags(CMetVar& st_var)
	{
		return  PYTHON_FUNCTION::ma_masked_where<float,float>(st_var.getFlags(),1,st_var.getData());
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
				bincenters[i] = 0.5*(bins[i] + bins[i+1]);


	}
	double model(const input_vector&  input, const parameter_vector& params)
	{
		const double p0 = params(0);
		const double p1 = params(1);

		const double i0 = input(0);
		const double temp = p0*i0 + p1 ;

		return temp*temp;
	}
	// ----------------------------------------------------------------------------------------

	// This function is the "residual" for a least squares problem.   It takes an input/output
	// pair and compares it to the output of our model and returns the amount of error.  The idea
	// is to find the set of parameters which makes the residual small on all the data pairs.
	// Source : dlib documentation
	double residual(const std::pair<input_vector, double>& data, const parameter_vector& params)
	{
		return model(data.first, params) - data.second;
	}
	// ----------------------------------------------------------------------------------------
	// This function is the derivative of the residual() function with respect to the parameters.
	parameter_vector residual_derivative(const std::pair<input_vector, double>& data, const parameter_vector& params)
	{
		parameter_vector der(2);

		const double p0 = params(0);
		const double p1 = params(1);
		

		const double i0 = data.first(0);
		
		const double temp = p0*i0 + p1 ;

		der(0) = i0 * 2 * temp;
		der(1) = 2 * temp;
		
		return der;
	}
	/*
		decay function for line fitting
		p[0] = intercept
		p[1] = slope
	*/
	valarray<float> linear(const valarray<float>& X,  parameter_vector& p)
	{
		return float(p(1)) * X + float(p(0));
	}

	float get_critical_values(std::vector<int> indata, int binmin , int binwidth , float old_threshold )
	{

		float threshold;
		if (indata.size() > 0)  //sort indata ?
		{
			std::vector<int> dummy = np_abs(indata);
			dummy = np_ceil<int>(dummy);
			valarray<float> full_edges = arange<float>(float(3* (*max(dummy.begin(), dummy.end()))), float(binmin), float(binwidth));
			dummy.clear();
			valarray<float> full_hist = histogram<float,int>(np_abs(indata), full_edges);
			if (full_hist.size() > 1)
			{
				// use only the central section (as long as it's not just 2 bins)
				int i = 0;
				int limit = 0;
				while (limit < 2)
				{
					if (npwhere<float>(full_hist, float(0.), "=").size()>0)
					{
						limit = npwhere<float>(full_hist,0.,"=")[i];  //where instead of argwhere??
						i++;
					}
					else
					{
						limit = full_hist.size();
						break;
					}
				}
				valarray<float> edges = full_edges[std::slice(0, limit, 1)];
				valarray<float> hist = full_hist[std::slice(0, limit, 1)];
				hist.apply(Log10Func);
				//Initialiser les parametres
				
				// DATA (X,Y)
				

				std::vector<pair<input_vector, double>> data;
				input_vector input;
				for (size_t i = 0; i < edges.size(); ++i)
				{
					input(0) = edges[i];
					data.push_back(make_pair(input, hist[i]));
				}
				// Now let's use the solve_least_squares_lm() routine to figure out what the
				// parameters are based on just the data.
				//Use the Levenberg-Marquardt method to determine the parameters which
				// minimize the sum of all squared residuals
				parameter_vector  fit;
				fit(0) = hist[np_argmax(hist)];
				fit(1) = 1;
			
				dlib::solve_least_squares_lm(objective_delta_stop_strategy(1e-7).be_verbose(),
					residual,
					derivative(residual),
					data,
					fit);
				valarray<float> fit_curve = linear(full_edges, fit);
				if (fit(1) < 0)
				{
					//in case the fit has a positive slope
					//where does fit fall below log10(-0.1)
					if (npwhere<float>(fit_curve, float(-1), "<").size()>0)
					{
						size_t fit_below_point1 = npwhere<float>(fit_curve, float(-1), "<")[0];
						valarray<float> dummy = full_hist[slice(fit_below_point1, full_hist.size() - fit_below_point1, 1)];
						size_t first_zero_bin = npwhere<float>(dummy, 0, "=")[0] + 1;
						threshold = binwidth*(fit_below_point1 + first_zero_bin);
					}
					else
					{
						//too shallow a decay - use default maximum
						threshold = full_hist.size();
					}
					//find first empty bin after that
				}
				else threshold = full_hist.size();
				
			}
			else 
				threshold = *std::max_element(indata.begin(),indata.end()) + binwidth;
		}
		else 
			threshold = *std::max_element(indata.begin(), indata.end()) + binwidth;

		return threshold;
	}

	//Sum up a single month across all years(e.g.all Januaries)
	//return this_month, year_ids, datacount

	void  concatenate_months(std::valarray<std::pair<int, int>>& month_ranges, std::valarray<float>& data, std::vector<CMaskedArray>& this_month,
		std::vector<int>& year_ids, std::valarray<float>& datacount, float missing_value, bool hours)
	{

		
		for (size_t y = 0; y < month_ranges.size();y++)
		{

			valarray<float> dummy = data[slice(month_ranges[y].first, month_ranges[y].second - month_ranges[y].first + 1, 1)];
			CMaskedArray this_year = CMaskedArray::CMaskedArray(missing_value, dummy);
			datacount[y] = this_year.compressed().size();

			if (y == 0)
			{
				//store so can access each hour of day separately
				if (hours)
					this_month = L_reshape(this_year, 24);
				else
					this_month.push_back(this_year);
				for (int i = 0; i < this_month.size(); ++i)
					year_ids.push_back(y);
			}
			else
			{
				if (hours)
				{
					std::vector<CMaskedArray> t_years = C_reshape(this_year, 24);
					std::copy(t_years.begin(), t_years.end(), std::back_inserter(this_month));
				}
				else
					this_month.push_back(this_year);
				for (int i = 0; i < this_month.size(); ++i)
					year_ids.push_back(y);
			}
			
		}
	}

	//''' Calculate the percentile of data '''
	inline float percentiles(std::valarray<float>& data, float percent, bool idl)
	{
		valarray<float> sorted_data(data);
		std::sort(std::begin(sorted_data), std::end(sorted_data));
		int n_data;
		float percentile;
		if (idl)
		{
			n_data = data.size() - 1;
			percentile = sorted_data[int(ceil(n_data * percent))]; // matches IDL formulation
		}
		else
		{
			n_data = data.size();
			percentile = sorted_data[int(n_data * percent)];
		}

		return percentile;
	}
		


	void winsorize(std::valarray<float>& data, float percent, bool idl )
	{
		for (float pct : { percent, 1 - percent })
		{
			float percentile;
			valarray<size_t> locs(data.size());
			if (pct < 0.5)
			{
				percentile = percentiles(data, pct, idl );
				locs = npwhere(data, percentile, "<");
			}
			else
			{
				percentile = percentiles(data, pct, idl);
				locs = npwhere(data, percentile, ">");
			}
			data[locs] = percentile;

		}
	}


		


}
