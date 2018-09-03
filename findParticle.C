#define findParticle_cxx
#include "findParticle.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>

float DeltaR(float a_eta, float a_phi, float b_eta, float b_phi){
// deltaR = (del_eta ^ 2 + del_phi ^ 2)^(1/2)
	float dEta= a_eta - b_eta;
	float dPhi= a_phi - b_phi;
	if (dPhi >= TMath::Pi() ){		
		dPhi = dPhi - 2* TMath::Pi();
	}
	else if (dPhi < -TMath::Pi() ){
		dPhi = dPhi + 2* TMath::Pi();
	}
	return (TMath::Sqrt( dEta * dEta + dPhi * dPhi ) ); //use define of deltaR
	
}
void findParticle::Loop(){
//   In a ROOT session, you can do:
//      root> .L MyClass.C
//      root> MyClass t
//      root> t.GetEntry(12); // Fill t data members with entry number 12
//      root> t.Show();       // Show values of entry 12
//      root> t.Show(16);     // Read and show values of entry 16
//      root> t.Loop();       // Loop on all entries
//

//     This is the loop skeleton where:
//    jentry is the global entry number in the chain
//    ientry is the entry number in the current Tree
//  Note that the argument to GetEntry must be:
//    jentry for TChain::GetEntry
//    ientry for TTree::GetEntry and TBranch::GetEntry
//
//       To read only selected branches, Insert statements like:
// METHOD1:
//    fChain->SetBranchStatus("*",0);  // disable all branches
//    fChain->SetBranchStatus("branchname",1);  // activate branchname
// METHOD2: replace line
//    fChain->GetEntry(jentry);       //read all branches
//by  b_branchname->GetEntry(ientry); //read only this branch
   if (fChain == 0) return;

// some hist define 
	TH1F* h1=new TH1F("h1","PT(h)",40,0,500);
	TH1F* h2=new TH1F("h2","DeltaR(h, a0)",40,0,10);
	TH1F* h2_2=new TH1F("h2_2","DeltaR(b,b~)",40,0,10);
	TH1F* h3=new TH1F("h3","PID_test",55,-20,35);
// some variables define
	float eta_h;
	float phi_h;
	float eta_a0;
	float phi_a0;
	float phi_b1; //b1= b (PID=5)
	float eta_b1;
	float phi_b2; //b2= b~ (PID=-5)
	float eta_b2;
 
   Long64_t nentries = fChain->GetEntriesFast();

   Long64_t nbytes = 0, nb = 0;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
      Long64_t ientry = LoadTree(jentry);
      if (ientry < 0) break;
      nb = fChain->GetEntry(jentry);   nbytes += nb;

//select Particle - Using Switch to less the using of "if loop"
		for (int i=0; i<kMaxParticle; i++){
			switch(Particle_PID[i]){
				case 25:			 //find the h
					h1->Fill(Particle_PT[i]);
					h3->Fill(Particle_PID[i]);
					eta_h = Particle_Eta[i];
					phi_h = Particle_Phi[i];
					break;
			
				case 28:         //find the a0
					eta_a0 = Particle_Eta[i];
					phi_a0 = Particle_Phi[i];
					break;
				
				case 5:         //find the b
					eta_b1 = Particle_Eta[i];
					phi_b1 = Particle_Phi[i];
					break;
	
				case -5:			//find b~
					eta_b2 = Particle_Eta[i];
					phi_b2 = Particle_Phi[i];
					break;	
				//see the particle PID=4 (c quark)
				/*case 4:
					std::cout<< jentry << std::endl;
					break;
				case -4:
					std::cout<< jentry << std::endl;		
					break;	
				*/	
			}
		}
      // if (Cut(ientry) < 0) continue;
		//Notice that the delta phi can not minus two var. simplely.

		/*the function DeltaR is used to cal. the DeltaR. the content is
		on the top*/
		h2->Fill(DeltaR(eta_h, phi_h, eta_a0, phi_a0) ); 
		h2_2->Fill(DeltaR(eta_b1,phi_b1, eta_b2, phi_b2) );
   }
	gStyle->SetOptStat(1111111);//check the "outside" value?

	h2_2->Draw();
	gPad->SetLogy(); //log y-axis 	
}
