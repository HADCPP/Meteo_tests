#include "winds.h"


using namespace std;
using namespace UTILS;
using namespace boost;
using namespace PYTHON_FUNCTION;
using namespace boost::numeric::ublas;

namespace INTERNAL_CHECKS
{
	void logical_checks(CStation &station, std::vector<int> flag_col, std::ofstream& logfile)
	{
		CMetVar speed = station.getMetvar("windspeeds");
		CMetVar direction = station.getMetvar("winddirs");

		//recover direction information where the speed is Zero

		varraysize fix_zero_direction = npwhereAnd<float,bool>(speed.getData(), "=", float(0), direction.getAllData().m_mask, "=", true);
		direction.setData(fix_zero_direction, float(0),false);
		station.setQc_flags(float(-1),fix_zero_direction, flag_col[1]);

		//negative speeds

		varraysize  negative_speed = ma_masked_where(speed.getAllData(), "<", float(0));
		station.setQc_flags(float(1), negative_speed, flag_col[0]);

		//negative directions (don't try to adjust)

		varraysize  negative_direction = ma_masked_where(direction.getAllData(), "<", float(0));
		station.setQc_flags(float(1), negative_direction, flag_col[1]);

		//  wrapped directions (don't try to adjust)
		varraysize  wrapped_direction = ma_masked_where(direction.getAllData(), ">", float(360));
		station.setQc_flags(float(1), wrapped_direction, flag_col[1]);

		//no direction possible if speed == 0
		varraysize  bad_direction = ma_masked_where(direction.getAllData(), ">", float(360));
		station.setQc_flags(float(1), bad_direction, flag_col[1]);

		//northerlies given as 360, not 0 --> calm

		varraysize  bad_speed = npwhereAnd(speed.getData(), "=", float(0), direction.getData(), "!", float(0));
		station.setQc_flags(float(1), bad_speed, flag_col[0]);

		//and output to file/screen
		varraysize flag_locs0 = npwhere(station.getQc_flags(flag_col[0]), float(0), " > ");   // in case of direction fixes
		varraysize flag_locs1 = npwhere(station.getQc_flags(flag_col[1]), float(0), ">");  // in case of direction fixes

		print_flagged_obs_number(logfile, "Wind Logical Checks", "windspeeds", flag_locs0.size());
		print_flagged_obs_number(logfile, "Wind Logical Checks", "winddirs", flag_locs1.size());

		//copy flags into attribute
		speed.setFlags(flag_locs0, float(1));
		direction.setFlags(flag_locs1, float(1));
	}
	void wind_rose_check(CStation &station, int flag_col, boost::gregorian::date start, boost::gregorian::date  end,
		std::ofstream& logfile)
	{

	}


	void wdc(CStation &station, std::vector<int> flag_col, boost::gregorian::date start, boost::gregorian::date  end,
		std::ofstream& logfile)
	{
		//what to do about synergistic flagging - can there be more speed obs than dir obs or vv?
		logical_checks(station, { flag_col[0], flag_col[1] }, logfile);

	}
}