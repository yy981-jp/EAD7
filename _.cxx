		for (KIDList e: list) std::cout << e.dat << " | " << e.ex << "\n";
		std::cout << "DEL,ADD,EDI,RCL,EXIT): ";
		std::string in;
		std::cin >> in;
		if (is_or(in,"d","del","DEL")) {
			std::cout << "対象のIDを入力: "
			list.erase()
		} else if (is_or(in,"a","add","ADD")) {
			
		} else if (is_or(in,"e","edi","EDI")) {
			
		} else if (is_or(in,"r","rcl","RCL","recal")) {
			
		} else if (is_or(in,"x","exit","EXIT")) {
			
		}
