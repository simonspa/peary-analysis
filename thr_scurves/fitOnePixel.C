
Double_t fitFunction( Double_t *x, Double_t *par ) {

  //return 0.5*par[0] * ( TMath::Erf( ( x[0] - par[1] )/sqrt(2)/par[2] ) + par[3] );

  // par0: mod
  // par1: sigma
  // par2: amp

  double t =  (x[0] - par[0]) / par[1];
  double aa = par[2] * TMath::Erfc(t);
  return aa;

  // f(x) = p2* ERFC()
}

double getReference(TString name, int r, int c) {

  bool longc = false;
  
  TH1D* pixel_scurve = new TH1D(name,"", 256, 0, 255);

  std::stringstream s;
  s << "thrscan_LSB_px" << name << "_bg.csv";

  std::ifstream pxfile(s.str());
  if(!pxfile.is_open()) {
    std::cout << "Could not open background file \"" << s.str() << "\"" << std::endl;
  }
  else {
    std::cout << "Open background file \"" << s.str() << "\"" << std::endl;
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
      if(row != r || column != c) continue;
      longc = true;
      pixel_scurve->SetBinContent(thr,cnt);
    }
    else if(i == 6) {
      if(row != r || column != c) continue;
      longc = false;
      pixel_scurve->SetBinContent(thr,longcnt);
    }
  }

  // Fit'em
  pixel_scurve->Fit("gaus","Q");
  TF1 *fit1 = pixel_scurve->GetFunction("gaus");
  double mean = fit1->GetParameter(1);
  double width = fit1->GetParameter(2);
  std::cout << "BG: width " << width << " mean " << mean << " right edge " << (mean + (2.35482/2*width)) << std::endl;

  // Alternative:
  TF1 *fit2 = new TF1( "fitFcn", fitFunction, 0, 30, 3);

  fit2->SetParName( 0, "mid" );
  fit2->SetParName( 1, "sigma" );
  fit2->SetParName( 2, "amp" );

  fit2->SetParameter( 1, 2 ); // width
  fit2->SetParameter( 2, 10 ); // amp
  fit2->SetParameter( 0, 5 ); // mid position

  pixel_scurve->Fit("fitFcn", "WR", "ep" );// R = range from fitFcn; W -> weight=1
  std::cout << "Alt: " << fit1->GetParameter(0);

  TCanvas *c1 = new TCanvas("c","",0,0,700,700);
  c1->cd();
  pixel_scurve->Draw();
  fit2->Draw("SAME");

  c1->Print(name + "_bg.png");

  return (mean + (width/2));
}


void fitOnePixel() {

  TFile * outf = new TFile("scurves.root", "RECREATE");

  std::vector<Double_t> tcharge, scurve_pos;

  int r = 14, c = 12;
  bool longc = false;

  std::stringstream p;
  p << "Pixel " << c << "," << r;

  int i = 7;

  std::stringstream ref;
  ref << setfill('0') << setw(2) << c << "_"
      << setfill('0') << setw(2) << r << "_trim"
      << setfill('0') << setw(2) << i;
  double reference = getReference(ref.str(),r,c);


    
  for(int tp = 130; tp < 150; tp++) {

    std::stringstream h;
    h << setfill('0') << setw(2) << c << "_"
      << setfill('0') << setw(2) << r << "_trim"
      << setfill('0') << setw(2) << i << "_tp"
      << tp;

    std::stringstream s;
    s << "thrscan_LSB_px" << h.str() << ".csv";

    std::ifstream pxfile(s.str());
    if(!pxfile.is_open()) {
      continue;
    }

    std::string name = "scurve_" + h.str();
    TH1D* pixel_scurve = new TH1D(name.c_str(),"", 256, 0, 255);
    

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
	pixel_scurve->AddBinContent(thr,cnt);
      }
      else if(i == 6) {
	longc = false;
	if(row != r || column != c) continue;
	pixel_scurve->AddBinContent(thr,longcnt);
      }
    }
    pxfile.close();
    

    pixel_scurve->GetXaxis()->SetTitle("threshold_LSB");
    
    if(longc) pixel_scurve->GetYaxis()->SetTitle("pixel counter 13-bit");
    else pixel_scurve->GetYaxis()->SetTitle("pixel counter 8-bit");

    pixel_scurve->SetTitle(h.str().c_str());

    TF1 *fit1;

    if(tp < 138) {
      fit1 = new TF1( "fitFcn", fitFunction, 30, 250, 3);
    }
    else {
      fit1 = new TF1( "fitFcn", fitFunction, 80, 250, 3);
    }

    fit1->SetParName( 0, "mid" );
    fit1->SetParName( 1, "sigma" );
    fit1->SetParName( 2, "amp" );

    //
    // set start values:
    //
    fit1->SetParameter( 1, 2 ); // width
    fit1->SetParameter( 2, 1000 ); // amp

    if(tp == 135) {
      fit1->SetParameter( 0, 55 ); // mid position
    }
    else if(tp == 138) {
      fit1->SetParameter( 0, 100 ); // mid position
    }
    else if (tp == 140) {
      fit1->SetParameter( 0, 130 ); // mid position
    }
    else if (tp == 142) {
      fit1->SetParameter( 0, 170 ); // mid position
    }
    else if(tp == 145) {
      fit1->SetParameter( 0, 215 ); // mid position
    }

    pixel_scurve->Fit("fitFcn", "WR", "ep" );// R = range from fitFcn; W -> weight=1

    TCanvas *c1 = new TCanvas("c","",0,0,700,700);
    c1->cd();
    pixel_scurve->Draw();
    fit1->Draw("SAME");

    std::string pname = name + ".png";
    c1->Print(pname.c_str());

    tcharge.push_back(tp);
    scurve_pos.push_back(fit1->GetParameter(0));
    delete c1;
  }

  TCanvas *canv = new TCanvas("c2","",0,0,700,700);
  canv->cd();
  TGraph* gr = new TGraph(tcharge.size(),&tcharge.at(0),&scurve_pos.at(0));

  gr->Fit("pol1","");
  TF1 *fit1 = gr->GetFunction("pol1");
  std::cout << "p0: " << fit1->GetParameter("p0");
  
  gr->SetTitle("SCurve pos. relative to noise");
  gr->GetXaxis()->SetTitle("test_cap_1_MSB");
  gr->GetYaxis()->SetTitle("scurve position [thr_LSB]");
  gr->GetYaxis()->SetTitleOffset(1.4);
  gr->Draw("A*");
  
  outf->Write();

}
