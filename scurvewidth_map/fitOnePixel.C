
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

  TH2D * widths  = new TH2D("widths", "", 128, 0, 127, 128, 0, 127);
  TH1D * widths1d  = new TH1D("widths1d", "", 100, 0, 20);

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

  for(int r = 0; r < 8; r++) {
    for(int c = 0; c < 8; c++) {

      std::stringstream h;
      h << setfill('0') << setw(2) << c << "_"
	<< setfill('0') << setw(2) << r << "_trim"
	<< setfill('0') << setw(2) << i;

      std::stringstream s;
      s << "thrscan_LSB_px" << h.str() << ".csv";

      std::string name = "noise_" + h.str();

      std::ifstream pxfile("data-incl-noise-2/" + s.str());
      if(!pxfile.is_open()) {
	std::cout << "Could not open matrix file \"" << s.str() << "\"" << std::endl;
	continue;
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
      pxfile.close();
    }
  }

  // Fit'em all
  int sigcnt = 0;
  std::cout << "Fitting";
  for(int k = 0; k < 128; k++) {
    for(int j = 0; j < 128; j++) {
      if(pixel_scurve[k][j]->GetEntries() < 1) continue;
      //std::cout << ".";

      // Calculate the range:
      Int_t bin1 = pixel_scurve[k][j]->FindLastBinAbove(2000);
      Int_t range_low = pixel_scurve[k][j]->GetXaxis()->GetBinCenter(bin1);

      Int_t bin2 = pixel_scurve[k][j]->FindLastBinAbove(0);
      Int_t range_high = 2*pixel_scurve[k][j]->GetXaxis()->GetBinCenter(bin2) - pixel_scurve[k][j]->GetXaxis()->GetBinCenter(bin1);

      // edge:
      Int_t bin3 = pixel_scurve[k][j]->FindLastBinAbove(1000);
      Int_t range_edge = pixel_scurve[k][j]->GetXaxis()->GetBinCenter(bin3);

      // Check for pixels with dips:
      Int_t numberofbins = pixel_scurve[k][j]->GetSize();
      Int_t xprev = 0;
      Int_t xprevprev = 0;
      for(int i = 1; i < numberofbins-1; i++) {
        Int_t x = pixel_scurve[k][j]->GetBinContent(i);
        if (xprevprev - xprev > 500 && x - xprev > 500) {
	  std::cout << "Potential dip issue: " << k << " " << j << ", dip depth: " << (xprev-x) << ", position " << pixel_scurve[k][j]->GetXaxis()->GetBinCenter(i) << std::endl;
	  break;
        }
	xprevprev = xprev;
	xprev = x;
    }
      
      // Pixels with spike issues:
      if(bin1 > -1 && (bin2-bin1) < 50) {
	std::cout << "Potential spike issue: " << k << " " << j << ", distance spike to zero-response: " << (bin2-bin1) << std::endl;
      }
      
      TF1 *fit1 = new TF1( "fitFcn", fitFunction, range_low + 5, range_high, 3);
      fit1->SetParName( 0, "mid" );
      fit1->SetParName( 1, "sigma" );
      fit1->SetParName( 2, "amp" );

      //
      // set start values:
      //
      fit1->SetParameter( 0, range_edge ); // mid position
      fit1->SetParameter( 1, 6 ); // width
      fit1->SetParameter( 2, 1000 ); // amp

      pixel_scurve[k][j]->Fit("fitFcn", "QWR", "ep" );// R = range from fitFcn; W -> weight=1

      if(fit1->GetParameter(1) < 0 || fit1->GetParameter(1) > 20) {
	std::cout << "Pixel " << k << " " << j << " out-of-bound sigma: " << fit1->GetParameter(1) << std::endl;
	  sigcnt++;
	  continue;
      }
      widths->Fill(k,j,fit1->GetParameter(1));
      widths1d->Fill(fit1->GetParameter(1));

      /*std::cout << fit1->GetParameter(0) << ", "
		<< fit1->GetParameter(1) << ", "
		<< fit1->GetParameter(2) << std::endl;
      */
    }
  }
  std::cout << std::endl;

  std::cout << "Failed: " << sigcnt << std::endl;
  
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

  outf->Write();

}
