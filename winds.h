#include "station.h"
#include "utils.h"
#include "python_function.h"
#include "Utilities.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <cmath>


namespace INTERNAL_CHECKS
{
	/*
	Select occurrences of wind speed and direction which are 
    logically inconsistent with measuring practices.

   
    :param Station station: station object
    :param array flag_col: which columns to use in QC flag array
    :param file logfile: logfile to output to
  */
	void logical_checks(CStation &station, std::vector<int> flag_col, std::ofstream& logfile);

	/*
		Checks for large differences in the year-to-year wind-rose shape.  
		Uses RMSE and fits Gaussian.  Finds gap in distribution to flag beyond

		:param  station: station object
		:param int flag_col: which column to store the flags in
		:param datetime start: start of data
		:param datetime end: end of data
		
	*/
	void wind_rose_check(CStation &station, int flag_col, boost::gregorian::date start, boost::gregorian::date  end,
		std::ofstream& logfile);

	// Specific wind speed and direction checks
	void wdc(CStation &station, std::vector<int> flag_col, boost::gregorian::date start, boost::gregorian::date  end,
		std::ofstream& logfile);
}