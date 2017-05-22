{
  for(int i = 0; i < 16; i++) {
  
    gStyle->SetPalette(1);
  
    TH1D * h1  = new TH1D("stddev", "", 41, 0, 20);
    TH1D * h2  = new TH1D("mean", "", 256, 0, 255);

    std::stringstream s;
    s << "thrscan_threshold_LSB_trim" << setfill('0') << setw(2) << i << ".csv";
    TString out = "scurves_" + s.str() + ".root";
    TFile * outf = new TFile(out, "RECREATE");
    std::string filename =  s.str();
    std::map<int, std::map<int, TH1D*> > pixel_scurve;
    
    std::ifstream pxfile(filename);
    if(!pxfile.is_open()) {
      std::cout << "Could not open matrix file \"" << filename << "\"" << std::endl;
      continue;
    }

    std::string line = "";
    for(int i = 0; i < 128; i++) {
      for(int j = 0; j < 128; j++) {
	std::string name = "scurve_" + std::to_string(i) + "_" + std::to_string(j);
	pixel_scurve[i][j] = new TH1D(name.c_str(),"", 256, 0, 255);
      }
    }
    
    while(std::getline(pxfile, line)) {
      if(!line.length() || '#' == line.at(0))
	continue;

      std::stringstream ss(line);
      std::string value;
      int thr, column, row, flag, cnt;

      int i = 0;
      while(getline(ss,value,',')) {
	if(i == 0) thr = atoi(value.c_str());
	else if(i == 1) column = atoi(value.c_str());
	else if(i == 2) row = atoi(value.c_str());
	else if(i == 3) flag = atoi(value.c_str());
	else if(i == 4) cnt = atoi(value.c_str());
	i++;
      }

      // Proper entry
      if(i == 5) {
	pixel_scurve[column][row]->SetBinContent(thr,cnt);
      }
    }

    //pixel_scurve[13][13]->Draw();

    for(int i = 0; i < 128; i++) {
      for(int j = 0; j < 128; j++) {
	if(pixel_scurve[i][j]->GetEntries() > 0) {
	  h1->Fill(pixel_scurve[i][j]->GetStdDev(1));
	  h2->Fill(pixel_scurve[i][j]->GetMean(1));
	}
      }
    }
 
    //outf->Write();
  }
}
