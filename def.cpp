#include "def.h"


namespace map {
	namespace index {
		std::unordered_map<std::string, int> to_number = {{"a",0},{"b",1},{"c",2},{"d",3},{"e",4},{"f",5},{"g",6},{"h",7},{"i",8},{"j",9}};
		std::unordered_map<int, std::string> to_alphabet = {{0,"a"},{1,"b"},{2,"c"},{3,"d"},{4,"e"},{5,"f"},{6,"g"},{7,"h"},{8,"i"},{9,"j"}};
	}
	namespace def {
		std::unordered_map<std::string, int> to_number = 
		{{"a",1},{"b",2},{"c",3},{"d",4},{"e",5},{"f",6},{"g",7},{"h",8},{"i",9},{"j",10},{"k",11},{"l",12},{"m",13},
		{"n",14},{"o",15},{"p",16},{"q",17},{"r",18},{"s",19},{"t",20},{"u",21},{"v",22},{"w",23},{"x",24},{"y",25},{"z",26},
		{"A",27},{"B",28},{"C",29},{"D",30},{"E",31},{"F",32},{"G",33},{"H",34},{"I",35},{"J",36},{"K",37},{"L",38},{"M",39},
		{"N",40,},{"O",41},{"P",42},{"Q",43},{"R",44},{"S",45},{"T",46},{"U",47},{"V",48},{"W",49},{"X",50},{"Y",51},{"Z",52}};
		std::unordered_map<int, std::string> to_alphabet = 
		{{1,"a"},{2,"b"},{3,"c"},{4,"d"},{5,"e"},{6,"f"},{7,"g"},{8,"h"},{9,"i"},{10,"j"},{11,"k"},{12,"l"},{13,"m"},
		{14,"n"},{15,"o"},{16,"p"},{17,"q"},{18,"r"},{19,"s"},{20,"t"},{21,"u"},{22,"v"},{23,"w"},{24,"x"},{25,"y"},{26,"z"},
		{27,"A"},{28,"B"},{29,"C"},{30,"D"},{31,"E"},{32,"F"},{33,"G"},{34,"H"},{35,"I"},{36,"J"},{37,"K"},{38,"L"},{39,"M"},
		{40,"N"},{41,"O"},{42,"P"},{43,"Q"},{44,"R"},{45,"S"},{46,"T"},{47,"U"},{48,"V"},{49,"W"},{50,"X"},{51,"Y"},{52,"Z"}};
	}
}

//0 50回連続 不明なエラー;
//1 開封期限を超えています ;
//2 error:fb;
//3 ルームが違います;
//4 error:time_number_to_alphabet;
//5 error:time_alphabet_to_number;
	EADdat::EADdat(std::string data, std::bitset<6> error): data(data), error(error) {}
	EADdat::operator bool() {return error.none();}
	std::string EADdat::code() {
		std::string message;
		if (error[0]) message+="50回連続で不明なエラーが発生しました ";
		if (error[1]) message+="開封期限を超えています ";
		if (error[2]) message+="error:fb ";
		if (error[3]) message+="ルームが違います ";
		if (error[4]) message+="error:time_number_to_alphabet ";
		if (error[5]) message+="error:time_alphabet_to_number";
		return message;
	}
