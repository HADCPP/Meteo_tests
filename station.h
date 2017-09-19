#pragma once
#include<string>
#include <vector>
#include <map>
#include <valarray>
#include <boost/numeric/ublas/matrix.hpp>


struct s_time
	{
		std::string units;
		std::vector<int> data;
	};


class CMetVar
{
public:

	CMetVar::CMetVar(std::string name="", std::string long_name="")
	{
		CMetVar::m_name = name;
		CMetVar::m_long_name = long_name;

		m_fdi = 0;
		m_flagged_value = 0;
	}
	

	std::string getName(){ return m_name; }
	std::string getLong_Name(){ return m_long_name; }
	std::string getUnits(){ return m_units; }
	void setDtype(std::string dtype){ m_dtype = dtype; }
	void setUnits(std::string units){ m_units = units; }
	void setCalendar(std::string calendar){ m_calendar = calendar; }
	void setMdi(std::string mdi){ m_mdi = mdi; }
	void setValidMax(std::string valid_max){ m_valid_max = valid_max; }
	void setValidMin(std::string valid_min){ m_valid_min = valid_min; }
	void setCoordinates(std::string coordinates){ m_coordinates = coordinates; }
	void setFdi(float fdi){ m_fdi = fdi; }
	void setCellmethods(std::string cell_methods){ m_cell_methods = cell_methods; }
	void setStandard_name(std::string standard_name){ m_standard_name = standard_name; }
	void setReportingStats(std::valarray<float> reporting_stats){ m_reporting_stats = reporting_stats; }
	void setFlagged_obs(std::valarray<float> flagged_obs){ m_flagged_obs = flagged_obs; }
	void setFlagged_value(float flagged_value){ m_flagged_value = flagged_value; }
	void setFlags(std::valarray<float> flag){ m_flags = flag; }
	void setFlags(std::valarray<size_t> v_flag,float flag){ m_flags[v_flag] = flag; }
	void setData(std::valarray<float> data){ m_data = data; }
	void setData(std::valarray<size_t> data,float val){ m_data[data] = val; }
	void setMaskedData(std::valarray<float> data){ m_masked_data = data; }
	std::string getMdi(){ return m_mdi; }
	float getFdi(){ return m_fdi; }
	std::string getDtype(){ return m_dtype; }
	std::string getStandardname(){ return m_standard_name; }
	std::valarray<float> getFlagged_obs(){ return m_flagged_obs; }
	std::valarray<float> getReportingStats(){ return m_reporting_stats; }
	std::valarray<float> getData(){ return m_data; }
	std::valarray<float> getMaskedData(){ return m_masked_data; }
	std::valarray<float> getFlags(){ return m_flags; }
	std::string getValidMax(){return m_valid_max ; }
	std::string getValidMin(){return m_valid_min ; }
	std::string getCoordinates(){ return m_coordinates; }
	std::string getCellmethods(){ return m_cell_methods; }
	std::string getCalendar(){ return m_calendar; }
	std::string toString() 	{ return "Variable :" + m_name + ", " + m_long_name; }

	void set_MetVar_attributes(std::string standard_name, std::string units, std::string mdi, std::string dtype)
	{
		m_units = units;
		m_standard_name = standard_name;
		m_dtype = dtype;
		m_mdi = mdi;

	}

protected :
	
	std::string m_name;
	std::string m_long_name;

	std::string m_units;
	std::string m_standard_name;
	std::string m_mdi;
	std::string m_dtype;
	std::string m_calendar;
	std::string m_valid_max;
	std::string m_valid_min;
	std::string m_coordinates;
	std::string m_cell_methods;
	float m_fdi;
	float m_flagged_value;
	std::valarray<float> m_reporting_stats;
	std::valarray<float> m_flags;
	std::valarray<float> m_data;
	std::valarray<float> m_masked_data; // données sans valeurs manquantes
	std::valarray <float> m_flagged_obs;
};



class CStation
{

public:
		CStation();
		CStation(std::string id, std::string name, double lat, double lon, double elev, std::string wmo);
		virtual ~CStation();

		
		double getLat()const{ return m_lat; }
		double getLon()const{ return m_lon; }
		double getElev()const{ return m_elev; }
		const std::string& getId()const{ return m_id; }
		const std::string& getName()const{ return m_name; }
		const std::string& getWmoId()const{ return m_wmoid; }
		// Remplir qc_flag correpondant à la variable qui se trouve à la position colonne
		void setQc_flags(std::valarray<float> qc_flags,int v){ m_qc_flags[v] = qc_flags; }
		void setQc_flags(float value, std::valarray<size_t> indices, int index){ m_qc_flags[index][indices] = value; }
		void setQc_flags(std::valarray<float> qc_flags, std::slice indices, int index){ m_qc_flags[index][indices] = qc_flags; }
		const std::vector<std::valarray<float>>& getQc_flags()const{ return m_qc_flags; }
		const std::valarray<float>& getQc_flags(int v){ return m_qc_flags[v]; }
		void setMetVar(CMetVar metvar, std::string var){ (m_Met_var)[var] = metvar; }
		CMetVar& getMetvar(std::string var){ return m_Met_var[var]; }
		void setTime_units(std::string units){ m_time.units = units; }
		void setTime_data(std::vector<int> data){ copy(data.begin(), data.end(), std::back_inserter(m_time.data)); }
		std::string getTime_units()const{ return m_time.units; }
		const std::vector<int>& getTime_data()const{ return m_time.data; }
		void setHistory(std::string history){ m_history = history; }
		const std::string& getHistory()const{ return m_history; }


		std::string toString();
		

protected:

	std::string m_id;
	std::string m_name;
	std::string m_wmoid;
	double m_lat;
	double m_lon;
	double m_elev;
	std::vector<std::valarray<float>> m_qc_flags;
	std::map<std::string, CMetVar >  m_Met_var;
	s_time m_time;
	std::string m_history;
};
