// Example: root -l 'DistribAmplCharge.C(true,"","~/godaq_rootfiles/analysis_v2.10.0/run9.root")'

#include "TFile.h"
#include "TTree.h"
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

TF1* Fit(bool ampl, TH1F* h) {
	Double_t low = 1400;
	Double_t high = 3000;
	TF1* f = 0;
	if(ampl) {
		h->GetXaxis()->SetRange(h->GetXaxis()->FindBin(0.),h->GetXaxis()->FindBin(3500.));
	} else {
		h->GetXaxis()->SetRange(h->GetXaxis()->FindBin(0.),h->GetXaxis()->FindBin(1000.));
	}
	int maxbin = h->GetMaximumBin();
	double AbscissaAtMax = h->GetXaxis()->GetBinCenter(maxbin);
	//cout << "AbscissaAtMax = " << AbscissaAtMax << endl;
	h->Fit("gaus", "Q", "", AbscissaAtMax - 0.20*AbscissaAtMax, AbscissaAtMax + 0.20*AbscissaAtMax);
	f = h->GetFunction("gaus");
	if(ampl) {
		h->GetXaxis()->SetRange(h->GetXaxis()->FindBin(0.),h->GetXaxis()->FindBin(4095.));
	} else {
		h->GetXaxis()->SetRange(h->GetXaxis()->FindBin(0.),h->GetXaxis()->FindBin(1022.));
	}
	/*
	for(int i = 0; i < 3; i++) {
		h->Fit("gaus", "Q", "", low, high);
		f = h->GetFunction("gaus");
		//cout << "here1" << endl;
		if(f) {
			Double_t mean = f->GetParameter(1);
			Double_t sigma = f->GetParameter(2);
			//cout << "here2" << endl;
			if(mean == 0) {
				cout << "ERROR: fitted mean is zero" << endl;
				return 0;
			}
			if(sigma > 0.20*2000) {
				sigma = 0.15*2000;
			}
			low = mean - 2 * sigma;
			high = mean + 2 * sigma;
		}
	}
	*/
	return f;
}

void DistribAmplCharge(bool ampl, TCut cut, TString fileName0, TString fileName1="", TString fileName2="", TString fileName3="", TString fileName4="", 
		       TString fileName5="", TString fileName6="", TString fileName7="", 
		       TString fileName8="", TString fileName9="") {
	gStyle->SetOptStat(0);
	gStyle->SetOptTitle(0);
	
	TChain ch("tree");
	ch.Add(fileName0);
	if(fileName1 != "") {
		ch.Add(fileName1);
	}
	if(fileName2 != "") {
		ch.Add(fileName2);
	}
	if(fileName3 != "") {
		ch.Add(fileName3);
	}
	if(fileName4 != "") {
		ch.Add(fileName4);
	}
	if(fileName5 != "") {
		ch.Add(fileName5);
	}
	if(fileName6 != "") {
		ch.Add(fileName6);
	}
	if(fileName7 != "") {
		ch.Add(fileName7);
	}
	if(fileName8 != "") {
		ch.Add(fileName8);
	}
	if(fileName9 != "") {
		ch.Add(fileName9);
	}
	
        TTreeReader reader(&ch);
        TTreeReaderValue<UInt_t> Run(reader, "Run");
        TTreeReaderValue<UInt_t> Evt(reader, "Evt");
	TTreeReaderValue<Int_t> NoPulses(reader, "NoPulses");
        TTreeReaderArray<UShort_t> IChanAbs240(reader, "IChanAbs240");
        TTreeReaderArray<Double_t> E(reader, "E");
        TTreeReaderArray<Double_t> Ampl(reader, "Ampl");
        TTreeReaderArray<Double_t> Charge(reader, "Charge");

	int Nbins = 200;
	float minX = 0;
	float maxX = 4095;
	if(!ampl) {
		float maxX = 1022;
	}
	
	std::vector<TH1F*> histos;
	for(int i = 0; i < 240; ++i) {
		histos.push_back(new TH1F(Form("histo_%i", i), Form("histo_%i", i), Nbins, minX, maxX));
	}
	
	ch.Draw(">>evtlist", cut);
	TEventList *evtlist = (TEventList*)gDirectory->Get("evtlist");
	int Nevents = evtlist->GetN();
	
	for(int i = 0; i < Nevents; i++) {
        //while (reader.Next()) {
		reader.SetEntry(evtlist->GetEntry(i));
		//cout << *Run << " " << *Evt << endl;
		if(*NoPulses == 2 && IChanAbs240[0] >= 120) {
			cout << "ERROR: IChanAbs240[0] >= 120" << endl;
			return;
		}
		if(*NoPulses == 2 && IChanAbs240[1] < 120) {
			cout << "ERROR: IChanAbs240[1] < 120" << endl;
			return;
		}
		for(int j = 0; j < *NoPulses; j++) {
			if(ampl) {
				histos[IChanAbs240[j]]->Fill(Ampl[j]);
			} else {
				histos[IChanAbs240[j]]->Fill(E[j]);
			}
		}
        }
	
	TCanvas* cLeft = new TCanvas("cLeft", "cLeft", 1500, 800);
	TCanvas* cRight = new TCanvas("cRight", "cRight", 1500, 800);
	cLeft->SetFillColor(7);
	cRight->SetFillColor(kYellow);
	cLeft->Divide(5, 6);
	cRight->Divide(5, 6);
	
	ofstream of("energy.csv");
	of << "# LAPD energy calibration constants (creation date: " << currentDateTime() 
	   << ", input files:" 
	   << " " << fileName0.Data() 
	   << " " << fileName1.Data() 
	   << " " << fileName2.Data()
	   << " " << fileName3.Data()
	   << " " << fileName4.Data()
	   << " " << fileName5.Data()
	   << " " << fileName6.Data()
	   << " " << fileName7.Data()
	   << " " << fileName8.Data()
	   << " " << fileName9.Data()
	   << ")" << endl;
	of << "# Calibration constant defined as the number of ADC counts corresponding to 511 keV" << endl;
        of << "# iChannelAbs240 calibConstant calibConstantError " << endl;
	
	// Draw right hemisphere
	for (int iQ = 0; iQ < 60; iQ++) {
		if(iQ < 30) {
			int irow = iQ/5;
			int icol = 4-iQ%5+1;
			int ipad = irow*5 + icol;
			cRight->cd(ipad);
		} else {
			int iQprime = iQ - 30;
			int irow = 5-iQprime/5;
			int icol = iQprime%5+1;
			int ipad = irow*5 + icol;
			cLeft->cd(ipad);
		}
		gPad->SetFillColor(kWhite);
 		gPad->SetBottomMargin(1.1);
 		gPad->SetTopMargin(0);
 		gPad->SetLeftMargin(1.1);
 		gPad->SetRightMargin(0.03);
 		gPad->SetGridx(1);
 		gPad->SetGridy(1);
		TLegend* leg = new TLegend(0.65,0.55,1,1);
		for (int iC = 0; iC < 4; iC++) {
			int iChanAbs240 = 4*iQ + iC;
			if(histos[iChanAbs240]->Integral() != 0) {
				int color;
				if(iC == 0)
					color = kRed;
				else if(iC == 1)
					color = kGreen+2;
				else if(iC == 2)
					color = kBlue;
				else if(iC == 3)
					color = kMagenta;
				histos[iChanAbs240]->Scale(1/histos[iChanAbs240]->Integral());
				histos[iChanAbs240]->SetLineColor(color);
				histos[iChanAbs240]->SetLineWidth(1);
				histos[iChanAbs240]->SetFillStyle(3001);
				histos[iChanAbs240]->SetFillColor(color);
				histos[iChanAbs240]->GetXaxis()->SetLabelSize(0.09);
				histos[iChanAbs240]->GetYaxis()->SetLabelSize(0.08);
				if(iC == 0) {
					histos[iChanAbs240]->Draw();
				} else {
					histos[iChanAbs240]->Draw("same");
				}
				cout << "iChanAbs240: " << iChanAbs240 ;
				if(histos[iChanAbs240]->GetEntries() > 0) {
					TF1* fitfunc = 0;
					fitfunc = Fit(ampl, histos[iChanAbs240]);
					if(fitfunc) {
						Double_t mean = fitfunc->GetParameter(1);
						Double_t sigma = fitfunc->GetParameter(2);
						Double_t meanErr = fitfunc->GetParError(1);
						cout << " ->  " << mean << "  " << meanErr << "  " << sigma << "  " << sigma/mean*100*2.35 << "%" ;
						if(sigma < 500 && meanErr < 0.20*mean) {
							of << iChanAbs240 << " " << mean << " " << meanErr << endl;
						} else {
							cout << " <=== Warning !!" ;
						}
						cout << endl;
					}
				} else {
					cout << endl;
					of << iChanAbs240 << " 0 0" << endl;
				}
				leg->AddEntry(histos[iChanAbs240], Form("Chan=%i", iChanAbs240), "f");
			}
		}
		leg->SetLineWidth(0);
		leg->Draw();
		//gPad->SaveAs(Form("pad_%i.png", iQ));
	}
	of.close();
}
