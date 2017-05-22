{

  int sh = 200;
  TFile * outf = new TFile("noise_curves.root", "RECREATE");

  bool longc = false;
  
  for(int c = 12; c < 14; c++) {
    for(int r = 8; r < 16; r++) {

      std::stringstream p;
      p << "Pixel " << c << "," << r;

      std::stringstream s;
      s << "thrscan_LSB_px"
	<< setfill('0') << setw(2) << c << "_"
	<< setfill('0') << setw(2) << r
	<< "_trim07.csv";

      std::stringstream h;
      h << "sh" << sh << "_"
	<< setfill('0') << setw(2) << c << "_"
	<< setfill('0') << setw(2) << r;
	   
      std::string name = "noise_" + h.str();
      TH1D* pixel_scurve = new TH1D(name.c_str(),"", 256, 0, 255);
    
      std::ifstream pxfile(s.str());
      if(!pxfile.is_open()) {
	std::cout << "Could not open matrix file \"" << s.str() << "\"" << std::endl;
	continue;
      }

      std::string line = "";
      while(std::getline(pxfile, line)) {
	if(!line.length() || '#' == line.at(0))
	  continue;

	std::stringstream ss(line);
	std::string value;
	int thr, column, row, flag, cnt, longcnt;

	int i = 0;
	while(getline(ss,value,',')) {
	  if(i == 0) thr = atoi(value.c_str());
	  else if(i == 1) column = atoi(value.c_str());
	  else if(i == 2) row = atoi(value.c_str());
	  else if(i == 3) flag = atoi(value.c_str());
	  else if(i == 4) cnt = atoi(value.c_str());
	  else if(i == 5) longcnt = atoi(value.c_str());
	  i++;
	}

	// Proper entry
	if(i == 5) {
	  longc = true;
	  if(row != r || column != c) continue;
	  pixel_scurve->SetBinContent(thr,cnt);
	}
	else if(i == 6) {
	  longc = false;
	  if(row != r || column != c) continue;
	  pixel_scurve->SetBinContent(thr,longcnt);
	}

      }

      pixel_scurve->GetXaxis()->SetTitle("threshold_LSB");
      if(longc) pixel_scurve->GetYaxis()->SetTitle("pixel counter 13-bit");
      else pixel_scurve->GetYaxis()->SetTitle("pixel counter 8-bit");

      pixel_scurve->SetTitle(name.c_str());

      TCanvas *c1 = new TCanvas("c","",0,0,700,700);
      pixel_scurve->Draw();

      h << ".pdf";
      std::string nm = "noise_" + h.str();
      c1->Print(nm.c_str());

      delete c1;
    }
  }

  outf->Write();
}
