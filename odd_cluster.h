#pragma once
#include "station.h"
#include "utils.h"
#include "python_function.h"

#include <iostream>
#include <valarray>
#include <vector>
#include <string>

class COddCluster
{
public:
	
	COddCluster(float start, float end, int length, float locations, float data_mdi, int last_data);
	virtual ~COddCluster();


	std::string toString();
protected:
	float m_start;
	float m_end;
	float m_length;
	float m_locations;
	float m_data_mdi;
	int m_last_data;
};

namespace INTERNAL_CHECKS
{
	/*
		Check for odd clusters of data surrounded by missing
		up to 6hr / 24hr surrounded by at least 48 on each side

		: param MetVar station : the station object
		: param list variable_list : list of observational variables to process
		: param list flag_col : the columns to set on the QC flag array
		: param datetime datastart : dataset start time
		: param file logfile : logfile to store outputs
		: param bool second : run for second time
		: returns :
		*/
	void occ(CStation station, std::vector<std::string> variable_list, std::vector<std::string>full_variable_list, std::vector<int>flag_col,
		std::ofstream &logfile, bool second = false);
}