void plot() {


  std::ifstream adcfile("scan_threshold.csv");
  if(!adcfile.is_open()) {
    std::cout << "Could not open file" << std::endl;
    return;
  }

  std::vector<double> threshold;
  std::vector<double> voltage;
  
  std::string line = "";
  while(std::getline(adcfile, line)) {
    if(!line.length() || '#' == line.at(0))
      continue;

    std::stringstream ss(line);
    std::string value;

    if(getline(ss,value,',')) threshold.push_back(atoi(value.c_str()));
    if(getline(ss,value,',')) voltage.push_back(atof(value.c_str()));

  }
  adcfile.close();


  TCanvas *canv = new TCanvas("c","",0,0,700,700);
  TGraph* gr = new TGraph(voltage.size(),&(threshold[0]),&(voltage[0]));
  gr->SetTitle("");
  gr->GetXaxis()->SetTitle("lin. threshold code");
  gr->GetYaxis()->SetTitle("threshold voltage [V]");
  gr->GetYaxis()->SetTitleOffset(1.3);
  gr->Draw("AC");
}
