#include "station.h"

#include<string>
#include<iostream>
#include <algorithm>

using namespace std;

struct CMetVar;

	CStation::CStation(string id,string name, double lat, double lon, double elev,string wmo)
	{
		m_id = id;
		m_lon = lon;
		m_lat = lat;
		m_elev = elev;
		m_name = name;
		m_wmoid = wmo;
	}
	CStation::~CStation()
	{
	}
	string CStation::toString()
	{
		return "CStation  :" + m_id + "  "+ m_name+  "+  Longitude: " + to_string(m_lon) + "  Latitude:  " + to_string(m_lat) + "  Elevation  :" + to_string(m_elev);

	}
	double CStation::getLat(){ return CStation::m_lat; }
	double CStation::getLon(){ return CStation::m_lon; }
	double CStation::getElev(){ return CStation::m_elev; }
	string CStation::getId(){ return CStation::m_id; }
	string CStation::getName(){ return CStation::m_name; }
	string CStation::getWmoId(){ return CStation::m_wmoid; }
	void CStation::setQc_flags(std::valarray<std::valarray<float>> qc_flags){ CStation::m_qc_flags = qc_flags; }
	void CStation::setQc_flags(std::valarray<float> qc_flags, std::slice indices, int index)
	{
		CStation::m_qc_flags[index][indices] = qc_flags;
	}
	std::valarray<std::valarray<float>> CStation::getQc_flags(){ return CStation::m_qc_flags; }
	void CStation::setMetVar(CMetVar metvar, string var){ (CStation::m_Met_var)[var]=metvar; }
	CMetVar* CStation::getMetvar(string var)
	{ 
		
		return &CStation::m_Met_var[var]; 
	}
	void CStation::setTime_units(string units){ m_time.units = units; }
	void CStation::setTime_data(std::vector<int> data){ copy(data.begin(), data.end(), std::back_inserter(m_time.data)); }
	std::string CStation::getTime_units(){ return CStation::m_time.units; }
	std::vector<int> CStation::getTime_data(){ return CStation::m_time.data; }
	void CStation::setHistory(string history){ CStation::m_history = history; }
	std::string CStation::getHistory(){ return CStation::m_history; }

	