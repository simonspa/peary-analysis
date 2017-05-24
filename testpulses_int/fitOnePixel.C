
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

void fitOnePixel() {
  TFile * outf = new TFile("scurves.root", "RECREATE");

  TH1D * means  = new TH1D("means", "", 16, 0, 15);

  Double_t trim[16], mean[16], width[16];
  Int_t n = 16;

  int r = 14, c = 12;
  bool longc = false;

  std::stringstream p;
  p << "Pixel " << c << "," << r;

  int i = 7;

    std::stringstream h;
    h << setfill('0') << setw(2) << c << "_"
      << setfill('0') << setw(2) << r << "_trim"
      << setfill('0') << setw(2) << i;

    std::string name = "scurve_" + h.str();
    TH1D* pixel_scurve = new TH1D(name.c_str(),"", 256, 0, 255);
    
    std::stringstream s;
    s << "thrscan_LSB_px" << h.str() << ".csv";

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

    TF1 *fit1 = new TF1( "fitFcn", fitFunction, 0, 100, 3);
    fit1->SetParName( 0, "mid" );
    fit1->SetParName( 1, "sigma" );
    fit1->SetParName( 2, "amp" );

    //
    // set start values:
    //
    fit1->SetParameter( 0, 60 ); // mid position
    fit1->SetParameter( 1, 2 ); // width
    fit1->SetParameter( 2, 90 ); // amp

    pixel_scurve->Fit("fitFcn", "WR", "ep" );// R = range from fitFcn; W -> weight=1

    trim[i] = i;

    double m  = fit1->GetParameter(1);
    double w = fit1->GetParameter(2);

    mean[i] = m;
    width[i] = w;
    
    std::cout << fit1->GetParameter(0) << ", "
	      << fit1->GetParameter(1) << ", "
	      << fit1->GetParameter(2) << std::endl;
    std::cout << "Mean: " << trim[i] << endl;

    TCanvas *c1 = new TCanvas("c","",0,0,700,700);
    pixel_scurve->Draw();
    //fit1->Draw("SAME");

    std::string pname = name + ".png";
    c1->Print(pname.c_str());

  outf->Write();

}
