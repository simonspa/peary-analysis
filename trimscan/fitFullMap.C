{

  TFile * outf = new TFile("noise_full_curves.root", "RECREATE");

  gStyle->SetPalette(1);
  
  TH2D * meanslope  = new TH2D("meanslope", "", 128, 0, 127, 128, 0, 127);
  TH2D * widthslope  = new TH2D("widthslope", "", 128, 0, 127, 128, 0, 127);

  TH1D * meanslope1d  = new TH1D("meanslope1d", "", 100, 0, -10);
  TH1D * widthslope1d  = new TH1D("widthslope1d", "", 50, -5, 5);

  // Produce one map per trim setting:
  std::map<int, TH2D*> means;
  std::map<int, TH2D*> widths;
  std::map<int, TH1D*> means1d;
  std::map<int, TH1D*> widths1d;
  
  for(int i = 0; i < 16; i++) {
    std::stringstream h;
    h << "_trim" << setfill('0') << setw(2) << i;
    std::string n;
    
    n = "means" + h.str();
    means[i]  = new TH2D(n.c_str(), "", 128, 0, 127, 128, 0, 127);
    n = "widths" + h.str();
    widths[i] = new TH2D(n.c_str(), "", 128, 0, 127, 128, 0, 127);
    n = "means1d" + h.str();
    means1d[i] = new TH1D(n.c_str(), "", 256, 0, 255);
    n = "widths1d" + h.str();
    widths1d[i] = new TH1D(n.c_str(), "", 50, 0, 10);
  }


  std::map<int, std::map<int, std::vector<double> > > allmeans;
  std::map<int, std::map<int, std::vector<double> > > allwidths;
  std::vector<double> trim;

  bool longc = false;

  
  for(int i = 0; i < 16; i++) {

    std::stringstream h;
    h << "_trim" << setfill('0') << setw(2) << i;

    std::stringstream s;
    s << "thrscan_LSB_full" << h.str() << ".csv";

    std::string name = "noise_" + h.str();

    // Initialize all pixel plots
    std::map<int, std::map<int, TH1D*> > pixel_scurve;
    for(int k = 0; k < 128; k++) {
      for(int j = 0; j < 128; j++) {
	std::stringstream q;
	q << setfill('0') << setw(2) << k << "_"
	  << setfill('0') << setw(2) << j << "_trim"
	  << setfill('0') << setw(2) << i;
	std::string name = "noise_" + q.str();
	pixel_scurve[k][j] = new TH1D(name.c_str(),"", 256, 0, 255);
      }
    }

    
    std::ifstream pxfile(s.str());
    if(!pxfile.is_open()) {
      std::cout << "Could not open matrix file \"" << s.str() << "\"" << std::endl;
      continue;
    }
    else {
      std::cout << "Open matrix file \"" << s.str() << "\"" << std::endl;
    }

    trim.push_back(i);

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
    std::cout << "TRIM " << i << " - Fitting";
    for(int k = 0; k < 128; k++) {
      for(int j = 0; j < 128; j++) {
      
	pixel_scurve[k][j]->SetTitle(h.str().c_str());
	pixel_scurve[k][j]->GetXaxis()->SetTitle("threshold_LSB");
    
	if(longc) pixel_scurve[k][j]->GetYaxis()->SetTitle("pixel counter 13-bit");
	else pixel_scurve[k][j]->GetYaxis()->SetTitle("pixel counter 8-bit");

	if(pixel_scurve[k][j]->GetEntries() < 1) continue;
	std::cout << ".";
	pixel_scurve[k][j]->Fit("gaus","Q");
	TF1 *fit1 = pixel_scurve[k][j]->GetFunction("gaus");
	allmeans[k][j].push_back(fit1->GetParameter(1));
	allwidths[k][j].push_back(fit1->GetParameter(2));

	means[i]->Fill(k,j,fit1->GetParameter(1));
	widths[i]->Fill(k,j,fit1->GetParameter(2));
	means1d[i]->Fill(fit1->GetParameter(1));
	widths1d[i]->Fill(fit1->GetParameter(2));

	// delete, so we don't keep them:
	delete pixel_scurve[k][j];
      }
    }
    std::cout << std::endl;
  }

  outf->Write();

  // Produce one Graph per pixel for all trims:
  std::cout << "FINAL FITTING";

  std::map<int, std::map<int, TGraph*> > gmeans;
  std::map<int, std::map<int, TGraph*> > gwidths;
  for(int k = 0; k < 128; k++) {
      for(int j = 0; j < 128; j++) {
	if(allmeans[k][j].empty() || allwidths[k][j].empty()) continue;
	std::cout << "x";
	gmeans[k][j] = new TGraph(allmeans[k][j].size(),&trim.at(0),&allmeans[k][j].at(0));
	gwidths[k][j] = new TGraph(allwidths[k][j].size(),&trim.at(0),&allwidths[k][j].at(0));

	gmeans[k][j]->Fit("pol1","Q");
	TF1 *fit1 = gmeans[k][j]->GetFunction("pol1");
	meanslope->Fill(k,j,fit1->GetParameter("p1"));
	meanslope1d->Fill(fit1->GetParameter("p1"));

	gwidths[k][j]->Fit("pol1","Q");
	TF1 *fit2 = gwidths[k][j]->GetFunction("pol1");
	widthslope->Fill(k,j,fit2->GetParameter("p1"));
	widthslope1d->Fill(fit2->GetParameter("p1"));

	std::stringstream q;
	q << setfill('0') << setw(2) << k << "_"
	  << setfill('0') << setw(2) << j;

	std::string name = "gmean_" + q.str();
	gmeans[k][j]->Write(name.c_str());
	name = "gwidth_" + q.str();
	gwidths[k][j]->Write(name.c_str());
      }
  }
  std::cout << std::endl;

  TCanvas *canv = new TCanvas("c","",0,0,700,700);
  meanslope->GetXaxis()->SetTitle("column");
  meanslope->GetYaxis()->SetTitle("row");
  meanslope->GetYaxis()->SetTitleOffset(1.4);
  meanslope->GetZaxis()->SetTitle("slope of noise mean");
  meanslope->Draw("colz");

  meanslope1d->GetXaxis()->SetTitle("slope of noise mean");
  meanslope1d->GetYaxis()->SetTitle("pixels");


  TCanvas *canv2 = new TCanvas("c2","",0,0,700,700);
  widthslope->GetXaxis()->SetTitle("column");
  widthslope->GetYaxis()->SetTitle("row");
  widthslope->GetYaxis()->SetTitleOffset(1.4);
  widthslope->GetZaxis()->SetTitle("slope of noise width");
  widthslope->Draw("colz");

  widthslope1d->GetXaxis()->SetTitle("slope of noise width");
  widthslope1d->GetYaxis()->SetTitle("pixels");

  outf->Write();
}
