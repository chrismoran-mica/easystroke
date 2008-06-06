#ifndef __PREFDB_H__
#define __PREFDB_H__
#include <string>
#include <set>
#include <boost/serialization/access.hpp>

#include "locking.h"

enum TraceType { TraceStandard, TraceShape, TraceNone };
const int trace_n = 3;

struct ButtonInfo {
	guint button;
	guint state;
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version) {
		ar & button;
		ar & state;
	}
};

extern const double pref_p_default;
extern const int pref_delay_default;

class PrefDB {
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version) {
		{ Ref<std::set<std::string> > ref(exceptions); ar & *ref; }
		{ Ref<double> ref(p); ar & *ref; }
		{ Ref<ButtonInfo> ref(button); ar & *ref; }
		{ Ref<bool> ref(help); ar & *ref; }
		{ Ref<TraceType> ref(trace); ar & *ref; }
		{ Ref<int> ref(delay); ar & *ref; }
	}
	std::string filename;
public:
	PrefDB();

	Lock<std::set<std::string> > exceptions;
	Lock<double> p;
	Lock<ButtonInfo> button;
	Lock<bool> help;
	Lock<TraceType> trace;
	Lock<int> delay;

	void read();
	bool write() const;
};

typedef Ref<std::set<std::string> > RPrefEx;

PrefDB& prefs();
#endif