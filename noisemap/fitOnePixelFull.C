{

  TFile * outf = new TFile("noise_curves.root", "RECREATE");
  gStyle->SetPalette(1);

  TH2D * means  = new TH2D("means", "", 128, 0, 127, 128, 0, 127);
  TH2D * widths  = new TH2D("widths", "", 128, 0, 127, 128, 0, 127);

  TH1D * means1d  = new TH1D("means1d", "", 256, 0, 255);
  TH1D * widths1d  = new TH1D("widths1d", "", 50, 0, 10);


  int i = 7;
  bool longc = false;

  // Initialize all pixel plots
  std::map<int, std::map<int, TH1D*> > pixel_scurve;
  for(int k = 0; k < 128; k++) {
    for(int j = 0; j < 128; j++) {
      std::stringstream h;
      h << setfill('0') << setw(2) << k << "_"
	<< setfill('0') << setw(2) << j << "_trim"
	<< setfill('0') << setw(2) << i;
      std::string name = "noise_" + h.str();
      pixel_scurve[k][j] = new TH1D(name.c_str(),"", 256, 0, 255);
    }
  }


      std::stringstream s;
      s << "thrscan_LSB_full_trim" << setfill('0') << setw(2) << i << ".csv";

      std::ifstream pxfile(s.str());
      if(!pxfile.is_open()) {
	std::cout << "Could not open matrix file \"" << s.str() << "\"" << std::endl;
      }
      else {
	std::cout << "Open matrix file \"" << s.str() << "\"" << std::endl;
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
	  pixel_scurve[column][row]->SetBinContent(thr,cnt);
	}
	else if(i == 6) {
	  longc = false;
	  pixel_scurve[column][row]->SetBinContent(thr,longcnt);
	}

      }

  // Fit'em all
  std::cout << "Fitting";
  for(int k = 0; k < 128; k++) {
    for(int j = 0; j < 128; j++) {
      if(pixel_scurve[k][j]->GetEntries() < 1) continue;
      std::cout << ".";
      pixel_scurve[k][j]->Fit("gaus","Q");
      TF1 *fit1 = pixel_scurve[k][j]->GetFunction("gaus");
      means->Fill(k,j,fit1->GetParameter(1));
      widths->Fill(k,j,fit1->GetParameter(2));
      means1d->Fill(fit1->GetParameter(1));
      widths1d->Fill(fit1->GetParameter(2));

    }
  }
  std::cout << std::endl;

  TCanvas *c1 = new TCanvas("c","",0,0,700,700);
  means->Draw("colz");
  means->GetXaxis()->SetTitle("column");
  means->GetYaxis()->SetTitle("row");
  means->GetYaxis()->SetTitleOffset(1.3);
  means->GetZaxis()->SetTitle("<threshold_LSB>");
  means->GetZaxis()->SetTitleOffset(1.7);
  c1->Print("means.png");

  TCanvas *c2 = new TCanvas("c2","",0,0,700,700);
  widths->Draw("colz");
  widths->GetXaxis()->SetTitle("column");
  widths->GetYaxis()->SetTitle("row");
  widths->GetYaxis()->SetTitleOffset(1.3);
  widths->GetZaxis()->SetTitle("#sigma(threshold_LSB)");
  widths->GetZaxis()->SetTitleOffset(1.5);
  c2->Print("widths.png");

  TCanvas *c3 = new TCanvas("c3","",0,0,700,700);
  widths1d->Draw();
  widths1d->GetXaxis()->SetTitle("#sigma(threshold_LSB)");
  widths1d->GetYaxis()->SetTitle("pixels");
  widths1d->GetYaxis()->SetTitleOffset(1.3);
  c2->Print("widths_dist.png");

  TCanvas *c4 = new TCanvas("c4","",0,0,700,700);
  means1d->Draw();
  means1d->GetXaxis()->SetTitle("<threshold_LSB>");
  means1d->GetYaxis()->SetTitle("pixels");
  means1d->GetYaxis()->SetTitleOffset(1.3);
  c1->Print("means_dist.png");

  outf->Write();

}
