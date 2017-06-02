void linearize() {

  // Global structure to hold averaged values:
  // Using a map gives sorting for free also duplicates are taken care of by replacing them
  std::map<int,std::pair<int,int>> thr_values;
  
  // Loop over MSB:
  for(int msb = 0; msb < 256; msb++) {

    std::ifstream adcfile("scan_LSB_MSB" + std::to_string(msb) + ".csv");
    if(!adcfile.is_open()) {
      std::cout << "Could not open file" << std::endl;
      continue;
    }

    // Accumulate values and average them:
    std::vector<double> voltages;
    int lsb_prev = 0;
    
    std::string line = "";
    while(std::getline(adcfile, line)) {
      if(!line.length() || '#' == line.at(0))
	continue;

      std::stringstream ss(line);
      std::string value;
      int lsb;
      double voltage;

      if(getline(ss,value,',')) lsb = atoi(value.c_str());
      if(getline(ss,value,',')) voltage = atof(value.c_str());

      if(lsb == lsb_prev) { voltages.push_back(voltage); }
      else {
	lsb_prev = lsb;
	double average = std::accumulate(voltages.begin(), voltages.end(), 0.0) / voltages.size();
	thr_values[static_cast<int>(average*1e4)] = std::make_pair(msb,lsb);
	//std::cout << msb << "-" << lsb << ": avg " << average << " key " << (average*1e4) << std::endl;
	voltages.clear();
	voltages.push_back(voltage);
      }
      
    }
    adcfile.close();
  }

  // Final table:
  int prev = 0;
  int gaps = 0;

  std::pair<int,int> prev_element;
  for(auto &m : thr_values) {

    // Fill the gaps with the previous value:
    while(m.first - prev > 1) {
      gaps++;
      thr_values[++prev] = prev_element;
      std::cout << "Filling gap: " << prev << " : " << prev_element.first << "-" << prev_element.second << std::endl;
    }

    std::cout << m.first << " : " << m.second.first << "-" << m.second.second << std::endl;
    prev_element = m.second;
    prev = m.first;
  }
  std::cout << "\nKeys: " << thr_values.size() << std::endl;
  std::cout << "Gaps: " << gaps << std::endl;

  // Write to file:
  std::ofstream myfile;
  myfile.open("thr_linearized.hpp");
  myfile << "std::map<int, std::pair<uint8_t,uint8_t>> threshold = {";
  for(auto &m : thr_values) {
    myfile << "{" << m.first << ",{" << m.second.first << "," << m.second.second << "}}, ";
  }
  myfile << "};";

}
