// example code to run Bulk Graviton->ZZ->ZlepZhad selections on electron-channel
// Try to get efficientcy of nPass3 by TH1F::Divide

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <TString.h>
#include <map>
#include <TH1D.h>
#include "TH1F.h" //new add
#include <TFile.h>
#include "untuplizer.h"
#include <TClonesArray.h>
#include <TLorentzVector.h>
#include "TGraph.h"
#include "TStyle.h" //for gStyle

//define some coefficient of distribution
#define x_min 0
#define x_max 110
#define bins 20

using namespace std;
void xAna_hh(std::string inputFile){

	//get TTree from file ...
	TreeReader data(inputFile.data());

   Long64_t nTotal=0;
   Long64_t nPass[20]={0};

   TH1F* h_SD=new TH1F("h_SD","",100,0,200);
	TH1F* h_nVtx =new TH1F("h_nVtx","",bins,x_min,x_max);   //new add for total
	TH1F* h_nPass3 = (TH1F*)h_nVtx->Clone("h_nPass3"); //add the same scale (h_nVtx) hist "nPass3"
	TH1F* h_nFail3 = (TH1F*)h_nVtx->Clone("h_nFail3"); //add the same scale (h_nVtx) hist "nFail3"
	TH1F* h_eff = (TH1F*)h_nVtx->Clone("h_eff"); //new hist. for (h_nPass3/h_nVtx)
	h_nVtx->Sumw2(); //creat structure to store sum of squares of weights
	h_nPass3->Sumw2();
   //Event loop
   for(Long64_t jEntry=0; jEntry<data.GetEntriesFast() ;jEntry++){

     if (jEntry % 50000 == 0)
       fprintf(stderr, "Processing event %lli of %lli\n", jEntry + 1, data.GetEntriesFast());

     data.GetEntry(jEntry);
     nTotal++;

     //0. has a good vertex
     Int_t nVtx        = data.GetInt("nVtx"); //here we get nVtx
	  h_nVtx->Fill(nVtx); //new add
     if(nVtx<1)continue;
     nPass[0]++;

     int nFATJet         = data.GetInt("FATnJet");
     const int nJets=nFATJet;
     TClonesArray* fatjetP4 = (TClonesArray*) data.GetPtrTObject("FATjetP4");
     Float_t*  fatjetSDmass = data.GetPtrFloat("FATjetSDmass");
     Int_t*   nSubSoftDropJet = data.GetPtrInt("FATnSubSDJet");
     vector<float>   *subjetSDCSV =  data.GetPtrVectorFloat("FATsubjetSDCSV", nFATJet);
     vector<float>   *subjetSDPx  =  data.GetPtrVectorFloat("FATsubjetSDPx", nFATJet);
     vector<float>   *subjetSDPy  =  data.GetPtrVectorFloat("FATsubjetSDPy", nFATJet);
     vector<float>   *subjetSDPz  =  data.GetPtrVectorFloat("FATsubjetSDPz", nFATJet);
     vector<float>   *subjetSDE   =  data.GetPtrVectorFloat("FATsubjetSDE", nFATJet);
     vector<bool>    &passFatJetLooseID = *((vector<bool>*) data.GetPtr("FATjetPassIDLoose"));
    

     int nSubBTag[2]={0}; // check only the leading two fat jets 
     int nGoodFatJet=0;
     for(int ij=0; ij<nJets; ij++)
     {
    	
		 TLorentzVector* thisJet = (TLorentzVector*)fatjetP4->At(ij);
		 if(thisJet->Pt()<30)continue;
		 if(fabs(thisJet->Eta())>2.5)continue;
		 if(fatjetSDmass[ij]<95 || fatjetSDmass[ij]>145)continue;
		 if(!passFatJetLooseID[ij])continue;


  	  	 for(int is=0; is < nSubSoftDropJet[ij]; is++){
	    if(subjetSDCSV[ij][is] < 0.605)continue;
	    if(nGoodFatJet<2)
	  		nSubBTag[nGoodFatJet]++;
     }

	  	  nGoodFatJet++;

    }
  
    // if each fat jet has at least one subjet btag
    if(nSubBTag[0]>0 && nSubBTag[1]>0)nPass[1]++;

    // if one of the fat jets has at least two subjet btags
    if((nSubBTag[0]>1 && nSubBTag[1]>0) || 
       (nSubBTag[0]>0 && nSubBTag[1]>1))nPass[2]++;

    // if both fat jets have at least two subjet btags
    if(nSubBTag[0]>1 && nSubBTag[1]>1) 
	 {
		nPass[3]++;
		h_nPass3->Fill(nVtx);
    }  
	else 
		h_nFail3->Fill(nVtx);

 	} // end of loop over entries

//get efficeincy(old)
	/*Double_t mi = h_nVtx->GetXaxis()->GetXmin();
  	Double_t ma = h_nVtx->GetXaxis()->GetXmax();
	Int_t nSteps = h_nVtx->GetXaxis()->GetNbins();
	Double_t nVtxValue[nSteps];
	Double_t eff[nSteps];
	float step = (ma-mi)/nSteps*1.0;
	for (Int_t j=0; j<nSteps; j++)
	{
		nVtxValue[j] = j*step + 0.5*step;
		if(h_nVtx->GetBinContent(j) != 0)		
		eff[j] = (float)h_nPass3->GetBinContent(j)/(float)h_nVtx->GetBinContent(j);
		//fprintf(stderr, "eff is %f\n", eff[j]);
	}

	TGraph *gr  = new TGraph(nSteps,nVtxValue,eff);*/

//get efficiency(new * using divide)

	h_eff->Divide(h_nPass3, h_nVtx, 1, 1, "B"); //it means h_eff = c1*h_nPass3/c2*h_nVtx, here I set c1=c2=1.

// check error propagation of eff. and the option "B" in divide
	//take eff = nPass / (nPass + nFail)

	for (Int_t j=0; j<bins; j++)	
	{

		//get content
		float x=h_nPass3->GetBinContent(j);
		float y=h_nFail3->GetBinContent(j);
		float sumxy4 = (x+y)*(x+y)*(x+y)*(x+y);

		//get error
		float x_e=h_nPass3->GetBinError(j);
		float y_e = h_nFail3->GetBinError(j);
		//show the errorbar get from "B" and calculate(error propagation).
		std::cout << "error of eff(" << j << ") =" << h_eff->GetBinError(j) << std::endl; //"B"
		float cal = sqrt( y*y/sumxy4*x_e*x_e + x*x/sumxy4*y_e*y_e );
		std::cout << "error by calculate: " << cal << endl; //"Calculate"
	}

//show the result
  std::cout << "nTotal    = " << nTotal << std::endl;
  for(int i=0;i<20;i++)
    if(nPass[i]>0)
      std::cout << "nPass[" << i << "]= " << nPass[i] << std::endl;
	

	gStyle->SetOptStat(1111111);//check the "outside" value?
	
   //h_nVtx->Draw();
	//h_nPass3->Draw();
	h_eff->Draw("E");		//draw hist. with error bar
	//gr->Draw("AC*");   //(old)
}

