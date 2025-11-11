#include <string>
#include <vector>


namespace conv {
	inline QStringList vec(const std::vector<std::string>& vec) {
		QStringList qlist;
		qlist.reserve(static_cast<int>(vec.size()));
		for (auto &s : vec) {
			qlist << QString::fromStdString(s);
		}
		return qlist;
	}

	inline std::vector<std::string> vec(const QStringList& qlist) {
		std::vector<std::string> vec;
		vec.reserve(qlist.size());
		for (auto &s : qlist) {
			vec.push_back(s.toStdString());
		}
		return vec;
	}	
}