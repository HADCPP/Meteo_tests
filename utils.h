#pragma once

#include "station.h"
#include "python_function.h"
#include "Utilities.h"

#include<vector>
#include <algorithm>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <valarray>
#include <dlib\optimization.h>

typedef dlib::matrix<double, 1, 1> input_vector;
typedef dlib::matrix<double, 2, 1> parameter_vector;


namespace UTILS
{
	void  month_starts(boost::gregorian::date start, boost::gregorian::date end, std::vector<int>& month_locs);
	inline std::map<int, int> month_starts_in_pairs(boost::gregorian::date start, boost::gregorian::date end);
	std::valarray<bool> create_fulltimes(CStation& station, std::vector<std::string> var_list, boost::gregorian::date start,
		boost::gregorian::date end, std::vector<std::string> opt_var_list, bool do_input_station_id = true,
		bool do_qc_flags = true, bool do_flagged_obs = true);
	void monthly_reporting_statistics(const CMetVar& st_var, boost::gregorian::date start, boost::gregorian::date end);
	void print_flagged_obs_number(std::ofstream & logfile, std::string test, std::string variable, int nflags, bool noWrite=false);
	void apply_flags_all_variables(CStation& station, std::vector<std::string> full_variable_list, int flag_col, std::ofstream & logfile, std::string test);
	void apply_windspeed_flags_to_winddir(CStation& station);
	void append_history(CStation& station, std::string text);
	CMaskedArray<float>  apply_filter_flags(CMetVar& st_var);
	void print_flagged_obs_number(std::ofstream& logfile, std::string test, std::string variable, int nflags, bool noWrite);
	inline float __cdecl MyApplyRoundFunc(float n)
	{
		return std::floor(n);
	}
	template<typename T>
	inline T __cdecl MyApplyAbsFunc(T n)
	{
		return std::abs(n);
	}
	inline float __cdecl Log10Func(float n)
	{
		return std::log10f(n);
	}
	/*
	Uses histogram of remainders to look for special values

	: param array good_values :valarray (masked array.compressed())
	: param bool winddir : true if processing wind directions

	: output : resolution - reporting accuracy(resolution) of data
	*/
	 float reporting_accuracy(varrayfloat& good_values, bool winddir = false);
	void create_bins(varrayfloat& indata, float binwidth, varrayfloat &bins, varrayfloat  &bincenters);

	template<typename T>
	inline T idl_median(std::valarray<T> indata)
	{
		std::nth_element(std::begin(indata), std::begin(indata) + indata.size() / 2, std::end(indata));
		return indata[indata.size() / 2];
	}
	template<typename T, typename S>
	inline T Cast(const S &data)
	{
		return static_cast<T>(data);
	}
	template<typename T>
	inline T Cast(const std::string &data)
	{
		string s = typeid(T).name();
		if (s == "int")
			return static_cast<T>(std::atoi(data.c_str()));

		else
			return static_cast<T>(std::atof(data.c_str()));

	}

	/*
		Plot histogram on log-y scale and fit 1/x decay curve to set threshold
		 :param array indata: input data to bin up
		:param int binmin: minimum bin value
		:param int binwidth: bin width
		:param str line_label: label for plotted histogram
		:param float old_threshold: (spike) plot the old threshold from IQR as well

		:returns:   threshold value
	   */
	inline double model(const input_vector&  input, const parameter_vector& params)
	{
		const double p0 = params(0);
		const double p1 = params(1);

		const double i0 = input(0);
		const double temp = p0*i0 + p1;

		return temp*temp;
	}
	// ----------------------------------------------------------------------------------------

	// This function is the "residual" for a least squares problem.   It takes an input/output
	// pair and compares it to the output of our model and returns the amount of error.  The idea
	// is to find the set of parameters which makes the residual small on all the data pairs.
	// Source : dlib documentation
	inline double residual(const std::pair<input_vector, double>& data, const parameter_vector& params)
	{
		return model(data.first, params) - data.second;
	}
	// ----------------------------------------------------------------------------------------
	// This function is the derivative of the residual() function with respect to the parameters.
	inline parameter_vector residual_derivative(const std::pair<input_vector, double>& data, const parameter_vector& params)
	{
		parameter_vector der(2);

		const double p0 = params(0);
		const double p1 = params(1);


		const double i0 = data.first(0);

		const double temp = p0*i0 + p1;

		der(0) = i0 * 2 * temp;
		der(1) = 2 * temp;

		return der;
	}
	/*
	decay function for line fitting
	p[0] = intercept
	p[1] = slope
	*/
	inline varrayfloat linear(const varrayfloat& X, parameter_vector& p)
	{
		return float(p(1)) * X + float(p(0));
	}



	template<typename T>
	inline float get_critical_values(std::vector<T>& indata, T binmin = T(0), T binwidth = T(1), float old_threshold = 0.)
	{


		float threshold;
		if (indata.size() > 0)  //sort indata ?
		{
			std::vector<T> dummy = np_abs(indata);
			dummy = np_ceil<T>(dummy);
			varrayfloat full_edges = arange<float>(float(3 * (*max(dummy.begin(), dummy.end()))), T(binmin), T(binwidth));
			dummy.clear();
			varrayfloat full_hist = histogram<int>(np_abs(indata), full_edges);
			if (full_hist.size() > 1)
			{
				// use only the central section (as long as it's not just 2 bins)
				int i = 0;
				int limit = 0;
				while (limit < 2)
				{
					if (npwhere<float>(full_hist, float(0.), "=").size()>0)
					{
						limit = npwhere<float>(full_hist, 0., "=")[i];  //where instead of argwhere??
						i++;
					}
					else
					{
						limit = full_hist.size();
						break;
					}
				}
				varrayfloat edges = full_edges[std::slice(0, limit, 1)];
				varrayfloat hist = full_hist[std::slice(0, limit, 1)];
				hist.apply(Log10Func);
				//Initialiser les parametres

				// DATA (X,Y)


				std::vector<std::pair<input_vector, double>> data;
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

				dlib::solve_least_squares_lm(dlib::objective_delta_stop_strategy(1e-7).be_verbose(),
					residual,
					dlib::derivative(residual),
					data,
					fit);
				varrayfloat fit_curve = linear(full_edges, fit);
				if (fit(1) < 0)
				{
					//in case the fit has a positive slope
					//where does fit fall below log10(-0.1)
					if (npwhere<float>(fit_curve, float(-1), "<").size()>0)
					{
						size_t fit_below_point1 = npwhere<float>(fit_curve, float(-1), "<")[0];
						varrayfloat dummy = full_hist[slice(fit_below_point1, full_hist.size() - fit_below_point1, 1)];
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
				threshold = *std::max_element(indata.begin(), indata.end()) + binwidth;
		}
		else
			threshold = *std::max_element(indata.begin(), indata.end()) + binwidth;

		return threshold;
	}

	template<typename T>
	inline float get_critical_values(std::valarray<T>& indata, T binmin = T(0), T binwidth=T(1), float old_threshold = 0.)
	{

		float threshold;
		if (indata.size() > 0)  //sort indata ?
		{
			std::valarray<T> dummy = indata;
			dummy.apply(MyApplyAbsFunc);
			dummy = np_ceil<T>(dummy);
			std::valarray<T> full_edges = arange<float>(float(3 * (*max(std::begin(dummy), std::end(dummy)))), T(binmin), T(binwidth));
			dummy.free();
			varrayfloat full_hist = histogram(indata.apply(MyApplyAbsFunc), full_edges);
			if (full_hist.size() > 1)
			{
				// use only the central section (as long as it's not just 2 bins)
				int i = 0;
				int limit = 0;
				while (limit < 2)
				{
					if (npwhere<float>(full_hist, float(0.), "=").size()>0)
					{
						limit = npwhere<float>(full_hist, 0., "=")[i];  //where instead of argwhere??
						i++;
					}
					else
					{
						limit = full_hist.size();
						break;
					}
				}
				varrayfloat edges = full_edges[std::slice(0, limit, 1)];
				varrayfloat hist = full_hist[std::slice(0, limit, 1)];
				hist.apply(Log10Func);
				//Initialiser les parametres

				// DATA (X,Y)


				std::vector<std::pair<input_vector, double>> data;
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

				dlib::solve_least_squares_lm(dlib::objective_delta_stop_strategy(1e-7).be_verbose(),
					residual,
					dlib::derivative(residual),
					data,
					fit);
				varrayfloat fit_curve = linear(full_edges, fit);
				if (fit(1) < 0)
				{
					//in case the fit has a positive slope
					//where does fit fall below log10(-0.1)
					if (npwhere<float>(fit_curve, float(-1), "<").size()>0)
					{
						size_t fit_below_point1 = npwhere<float>(fit_curve, float(-1), "<")[0];
						varrayfloat dummy = full_hist[std::slice(fit_below_point1, full_hist.size() - fit_below_point1, 1)];
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
				threshold = *std::max_element(std::begin(indata), std::end(indata)) + binwidth;
		}
		else
			threshold = *std::max_element(std::begin(indata), std::end(indata)) + binwidth;

		return threshold;
	}

	//Sum up a single month across all years(e.g.all Januaries)
	void concatenate_months(std::valarray<std::pair<int, int>>& month_ranges, varrayfloat& data, std::vector<CMaskedArray<float>>& this_month,
		std::vector<int>& year_ids, varrayfloat& datacount, float missing_value, bool hours = true);

	inline float percentiles(varrayfloat& data, float percent, bool idl=false);
	void winsorize(varrayfloat& data, float percent, bool idl = false);
	float IQR(varrayfloat data, double percentile = 0.25);

}