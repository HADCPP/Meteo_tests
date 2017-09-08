#include "odd_cluster.h"


#include <string>
#include <iostream>
#include <algorithm>

using namespace std;



COddCluster::COddCluster(float start, float end, int length, float locations, float data_mdi, int last_data)
{
	COddCluster::m_start = start;
	COddCluster::m_end = end;
	COddCluster::m_length = length;
	COddCluster::m_locations = locations;
	COddCluster::m_data_mdi = data_mdi;
	COddCluster::m_last_data = last_data;
}
COddCluster::~COddCluster()
{
}
string COddCluster::toString()
{
	return " Odd cluster, starting :" + to_string(m_start) + ", ending " + to_string(m_end) + ", length"+ to_string(m_length);
}

namespace INTERNAL_CHECKS
{
	void occ(CStation station, std::vector<std::string> variable_list, std::vector<std::string>full_variable_list, std::vector<int>flag_col,
		std::ofstream &logfile, bool second)
	{
		/*
			the four options of what to do with each observation
			the keys give values which are subroutines, and can be called
			all subroutines have to take the same set of inputs
		*/
		enum Options { occ_normal, occ_start_cluster, occ_in_cluster, occ_after_cluster };
		int v = 0;
		for (string variable : variable_list)
		{
			CMetVar& st_var = station.getMetvar(variable);
			valarray<float> filtered_data = UTILS::apply_filter_flags(st_var);

			valarray<float> var_flags = station.getQc_flags()[v];	
			
			unsigned int prev_flag_number = 0;
			if (second)
			{
				//count currently existing flags
				valarray<float> dummy = var_flags[var_flags != float(0)];
				prev_flag_number = dummy.size();
			}
			//using IDL copy as method to ensure reproducibility (initially)
			COddCluster oc_details = COddCluster::COddCluster(UTILS::Cast<float>(st_var.getMdi()), UTILS::Cast<float>(st_var.getMdi()), 0, UTILS::Cast<float>(st_var.getMdi()), UTILS::Cast<float>(st_var.getMdi()), -1);
			int obs_type = 1;
			for (int time : station.getTime_data())
			{

			}
		}
	}
}