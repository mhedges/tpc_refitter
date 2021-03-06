/******************************************************************************
 * Program: TPC Phase I data skimmer
 * Author: Michael Hedges
 * Purpose: Skim Igal's BASF2 output for relevant data to be stored in BEAST
 * global ntuples for analysis
******************************************************************************/

#define skimmer_cxx
#include "refitter.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <iostream>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <algorithm>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sys/time.h>

#include "TROOT.h"
#include "TGraph2D.h"
#include "TMath.h"
#include <TVector3.h>
#include <TVirtualFitter.h>

#define DEBUG 0
#define NROWS 336
#define NCOLS 80
#define NPERFILE 1000
#define ARRSIZE 2400

// Define weights for track fit in x,y,z
//float weight[3] = {1., 1., 1.};
float weight[3] = {250./sqrt(12.), 50./sqrt(12.), 250./sqrt(12.)};

int iTPC=0;

TGraph2D *m_gr;

void skimmer::fitTrack() {
  /* Find long aspect of track */
  int x_max_index = 0, y_max_index = 0;
  int x_min_index = 0, y_min_index = 0;
  double *x_vals, *y_vals, *z_vals;
  int npoints = 0;
  npoints = m_gr->GetN();
  x_vals = m_gr->GetX(); y_vals = m_gr->GetY(); z_vals = m_gr->GetZ();

  
  for (int ii=0; ii<npoints; ii++){

	if ( x_vals[ii] == m_gr->GetXmax() )
	  x_max_index = ii;
	else if ( x_vals[ii] == m_gr->GetXmin() )
	  x_min_index = ii;
	if ( y_vals[ii] == m_gr->GetYmax() )
	  y_max_index = ii;
	else if ( y_vals[ii] == m_gr->GetYmin() )
	  y_min_index = ii;
  }

  int p_min_idx = 0;
  int p_max_idx = 0;
  if ( ( m_gr->GetXmax() - m_gr->GetXmin() ) >=
	  ( m_gr->GetYmax() - m_gr->GetYmin() ) ) {
    p_min_idx = x_min_index;
    p_max_idx = x_max_index;
 } else {
    p_min_idx = y_min_index;
    p_max_idx = y_max_index;
  }
  
  // start fit track
  TVirtualFitter::SetDefaultFitter("Minuit");
  TVirtualFitter *min = TVirtualFitter::Fitter(0, 5);  // Fitting with theta and phi
  min -> SetObjectFit(m_gr);
  min -> SetFCN(SumDistance2);
  
  // MAKE QUIET
  double p1=-1;
  min->ExecuteCommand("SET PRINTOUT",&p1, 1);

  double arglist[6] = {-1, 0, 0, 0, 0, 0};

  TVector3 temp_vector3 (x_vals[p_max_idx]-x_vals[p_min_idx],
			 y_vals[p_max_idx]-y_vals[p_min_idx],
			 z_vals[p_max_idx]-z_vals[p_min_idx]);
  

  double init_theta = temp_vector3.Theta();
  double init_phi   = temp_vector3.Phi();


  double pStart[5] = {x_vals[p_min_idx], y_vals[p_min_idx], z_vals[p_min_idx],
	init_theta, init_phi};
  min -> SetParameter(0, "x0",    pStart[0], 0.01, 0, 0);
  min -> SetParameter(1, "y0",    pStart[1], 0.01, 0, 0);
  min -> SetParameter(2, "z0",    pStart[2], 0.01, 0, 0);
  min -> SetParameter(3, "theta", pStart[3], 0.0001, 0, 0);
  min -> SetParameter(4, "phi",   pStart[4], 0.0001, 0, 0);

  arglist[0] = 1000; // number of function calls
  arglist[1] = 0.0001; // tolerance
  min -> ExecuteCommand("MIGRAD", arglist, 2);
  
  for (int iPar = 0; iPar < 5; iPar++){
    par_fit[iPar] = min -> GetParameter(iPar);
    par_fit_err[iPar] = min -> GetParError(iPar);
  }
  
  phi   = par_fit[4]*180./3.14159;
  theta = par_fit[3]*180./3.14159;
  
  getTrackInfo();

  double amin, edm, errdef;
  int npar, nparx;
  min -> GetStats(amin, edm, errdef, npar, nparx);

  chi2 = amin/(double)(npoints-npar);
  /* Second arg comes from defs in constants.h */

  delete min;
}

void skimmer::SumDistance2(int &, double *, double & sum, double * par, int ) {
  TGraph2D * m_gr = dynamic_cast<TGraph2D*>( (TVirtualFitter::GetFitter())->GetObjectFit() );
  //assert (m_gr != 0);
  double * px = m_gr->GetX();
  double * py = m_gr->GetY();
  double * pz = m_gr->GetZ();
  int np = m_gr->GetN();
  sum = 0;
  for (int i  = 0; i < np; ++i) {
    double d = distance2(px[i],py[i],pz[i],par);
    sum += d;
  }
}

double skimmer::distance2(double px,double py,double pz, double *p) {
   TVector3 xp(px,py,pz);
   TVector3 x0(p[0], p[1], p[2]);
   TVector3 u (TMath::Sin(p[3])*TMath::Cos(p[4]), TMath::Sin(p[3])*TMath::Sin(p[4]), TMath::Cos(p[3]));

   double coeff = u*(xp-x0);
   TVector3 n = xp - x0 - coeff * u;

   double dx = n.x();
   double dy = n.y();
   double dz = n.z();
   double d2_x = TMath::Power(dx/weight[0], 2);
   double d2_y = TMath::Power(dy/weight[1], 2);
   double d2_z = TMath::Power(dz/weight[2], 2);
   double d2 = d2_x + d2_y + d2_z;

   return d2;
}

void skimmer::getTrackInfo(){
  // Get track length
  TVector3 fit_position(par_fit[0], par_fit[1], par_fit[2]);
  TVector3 unit_direction(TMath::Sin(par_fit[3])*TMath::Cos(par_fit[4]), TMath::Sin(par_fit[3])*TMath::Sin(par_fit[4]), TMath::Cos(par_fit[3]));
  
  float t_const = -1e-10;
  float x_point = fit_position.x() + t_const * unit_direction.x();
  float y_point = fit_position.y() + t_const * unit_direction.y();
  float z_point = fit_position.z() + t_const * unit_direction.z();
  TVector3 initial_position(x_point, y_point, z_point);
  
  float min_distance = 1e+30;
  float max_distance = -1e+30;
  float distance = 0.0;

  double * X = m_gr->GetX();
  double * Y = m_gr->GetY();
  double * Z = m_gr->GetZ();
  int ipoints = m_gr->GetN();
  for (unsigned int iPoint = 0; iPoint < ipoints; iPoint++){
    TVector3 position (X[iPoint], Y[iPoint], Z[iPoint]);
    TVector3 position_vect = position - initial_position;
    distance = unit_direction * position_vect;
    if (distance < min_distance) min_distance = distance;
    if (distance > max_distance) max_distance = distance;	
  }
  
  t_length = max_distance - min_distance;  

  // Get impact parameters
  //                        x=0    x=end    y=0    y=end
  float end_positions[4] = {0.0, 250. * 80., 0.0, 50. * 336.};
  for (int iPos = 0; iPos < 4; iPos++){
    t_const = 0.0;
    if ( iPos < 2 ) t_const = (end_positions[iPos]-fit_position.X())/unit_direction.X();
    else t_const = (end_positions[iPos]-fit_position.Y())/unit_direction.Y();
    
    // it might need to change
    if ( iPos < 2 ) impact_pars[iPos] = y_point;
    else impact_pars[iPos] = x_point; 
  } 
}

void skimmer::getHitside(){
  
  // Get Hitside information.  This is a 4 digit number. 1111 is all edges, 0 is
  // no edges.  Order is top, bottom, right, left.  11 is alpha from sources.

  int cut_dim = 500; //um (it should be 250*integer)
  int set_edge_row_up = 335-cut_dim/250*5, set_edge_row_dw = cut_dim/250*5;
  int set_edge_col_up = 79-cut_dim/250, set_edge_col_dw = cut_dim/250;
  
  int c_row_up = 0, c_row_dw = 0, c_col_up = 0, c_col_dw = 0;
  int c_row_up_col_up = 0, c_row_up_col_dw = 0, c_row_dw_col_up = 0, c_row_dw_col_dw = 0;
  int c_inside = 0;
  
  int ncorners = 0;
  int check_edge = 0, nhits_edge = 0;
  int icol, irow;
  int nedges;

  for (int iPoint = 0; iPoint < npoints; iPoint++){
    check_edge = 0;
    icol = col[iPoint];
    irow = row[iPoint];
    if ( irow > set_edge_row_up ) {
      c_row_up = 1;
      check_edge = 1;
    } 
    if ( irow < set_edge_row_dw ) {
      c_row_dw = 1;
      check_edge = 1;
    } 
    if ( icol > set_edge_col_up ) {
      c_col_up = 1;
      check_edge = 1;
    }
    if ( icol < set_edge_col_dw ) {
      c_col_dw = 1;
      check_edge = 1;
    }
    
    if ( check_edge == 1)  nhits_edge += 1;
    
    if ( irow > set_edge_row_up && icol > set_edge_col_up ) 
      c_row_up_col_up = 1;
    if ( irow > set_edge_row_up && icol < set_edge_col_dw ) 
      c_row_up_col_dw = 1;
    if ( irow < set_edge_row_dw && icol > set_edge_col_up ) 
      c_row_dw_col_up = 1;
    if ( irow < set_edge_row_dw && icol < set_edge_col_dw ) 
      c_row_dw_col_dw = 1;
    if (! ( irow < set_edge_row_dw || irow > set_edge_row_up || icol < set_edge_col_dw || icol > set_edge_col_up ) )
      c_inside = 1;
    
  }
  nedges = c_row_up + c_row_dw +c_col_up + c_col_dw;
  ncorners = c_row_up_col_up + c_row_up_col_dw + c_row_dw_col_up + c_row_dw_col_dw;
  nedges = nedges - ncorners;
  
  if ( c_inside == 1 ){
    if ( c_row_up_col_up+c_row_dw_col_up > 1 || 
	 c_row_up_col_dw+c_row_dw_col_dw > 1 || 
	 c_row_dw_col_up+c_row_dw_col_dw > 1 || 
	 c_row_up_col_up+c_row_up_col_dw > 1 ) 
      nedges += 1;
  }
  if ( nedges >= 2 && c_inside == 1){
    nedges = 2;
  }

  hitside = (unsigned short)(c_row_up*1000 + c_row_dw*100 + c_col_up*10 + c_col_dw);

}
//
void skimmer::Loop(TString FileName, TString OutputName)
{
   TFile df(FileName,"READ");

   // Get the TTree
   TTree *dtr = (TTree*)df.Get("tree");

   //// Initialize the TTree
   Init(dtr);

   //// Activate all TBranches
   dtr->SetBranchStatus("*",1);

   //// Make new TTree with relevant data
   TFile *ofile = new TFile(OutputName, "RECREATE");
   TTree *tr = new TTree("tr","TPC Event Data");

   tr->Branch("event",&event,"event/I");
   tr->Branch("npoints",&npoints,"npoints/I");
   tr->Branch("row",&row,"row[npoints]/I");
   tr->Branch("col",&col,"col[npoints]/I");
   tr->Branch("bcid",&bcid,"bcid[npoints]/I");
   tr->Branch("tot",&tot,"tot[npoints]/I");
   tr->Branch("e",&e,"e[npoints]/F");
   tr->Branch("tstamp",&tstamp,"tstamp/D");
   tr->Branch("tot_sum",&tot_sum,"tot_sum/I");
   tr->Branch("e_sum",&e_sum,"e_sum/D");
   tr->Branch("time_range",&time_range,"time_range/I");
   tr->Branch("chi2",&chi2,"chi2/F");
   tr->Branch("t_length",&t_length,"t_length/D");
   tr->Branch("de_dx",&de_dx,"de_dx/D");
   tr->Branch("theta",&theta,"theta/D");
   tr->Branch("phi",&phi,"phi/D");
   tr->Branch("par_fit",&par_fit,"par_fit[6]/D");
   tr->Branch("par_fit_err",&par_fit_err,"par_fit_err[6]/D");
   tr->Branch("hitside",&hitside,"hitside/s");
   tr->Branch("impact_pars",&impact_pars,"impact_pars[4]/F");
   tr->Branch("top_alpha",&top_alpha,"top_alpha/I");
   tr->Branch("bottom_alpha",&bottom_alpha,"bottom_alpha/I");
   tr->Branch("xray",&xray,"xray/I");
   tr->Branch("neutron",&neutron,"neutron/I");
   tr->Branch("proton",&proton,"proton/I");
   tr->Branch("other",&other,"other/I");
   tr->Branch("vectors",&vectors,"vectors[npoints][3]/D");
   tr->Branch("c_vector",&c_vector,"c_vector[3]/D");
   tr->Branch("c_rms",&c_rms,"c_rms/D");
   tr->Branch("distances",&distances,"distances[npoints]/D");
   tr->Branch("detnb",&detnb,"detnb/I");
   
   int nentries = dtr->GetEntries();
   cout << "\n\n\n" << "Number of entries = " << nentries << "\n\n\n" << endl;

   // Initialize and fill a vector of npoints for all events for identifying
   // duplicated events
   vector<int> evt_points;
   for (Long64_t jentry=0; jentry<nentries;jentry++) {
    	   Long64_t ientry = LoadTree(jentry);
		   getentry = dtr->GetEntry(jentry);
		   npoints = MicrotpcMetaHits_m_pixNb[0];
		   evt_points.push_back (npoints);
   }

   for (Long64_t jentry=0; jentry<nentries;jentry++) {

	  event = jentry;

	  neutron = 0;
	  top_alpha = 0;
	  bottom_alpha = 0;
	  xray = 0;
	  other = 0;
	  proton = 0;
	  
	  m_gr = new TGraph2D();

      if (jentry %1000 == 0) cout << "\n\n\n\nEvent Counter: " << jentry << endl;

      Long64_t ientry = LoadTree(jentry);
	  getentry = dtr->GetEntry(jentry);

	  detnb = MicrotpcMetaHits_m_detNb[0];
	  tstamp = MicrotpcMetaHits_m_ts_start[0][0];
      
      // Skip event if t_stamp is 0
	  if (tstamp == 0) continue;

	  npoints = MicrotpcMetaHits_m_pixNb[0];
	  time_range = MicrotpcRecoTracks_m_time_range[0];
      sum_e = MicrotpcRecoTracks_m_esum[0];
	  xray= MicrotpcRecoTracks_m_partID[0][0];
	  
	  double x,y,z;

	  //cout << "Event #: "<< jentry << endl;
	  tot_sum = 0;
	  e_sum = 0;

      for (int pixn=0; pixn<npoints;pixn++) {
		col[pixn]=static_cast<int>(MicrotpcDataHits_m_column[pixn]);
		x = static_cast<double>(col[pixn]*250.0); 

		row[pixn]=static_cast<int>(MicrotpcDataHits_m_row[pixn]);
		y = static_cast<double>(row[pixn]*50.0);

		bcid[pixn]=static_cast<int>(MicrotpcDataHits_m_BCID[pixn]);
		z = -static_cast<double>(bcid[pixn]*250.0);

		tot[pixn]=MicrotpcDataHits_m_TOT[pixn];
		tot_sum += tot[pixn];

		// Conversion from TOT to energy
		if (tot[pixn]<3) {
				// Convert from TOT to total charge collected by pixel
				e[pixn] = q*((c1*tot[pixn]-a1*b1)/(a1-tot[pixn])); 
				//cout << e[pixn] << endl;
				// Convert from total charge to primary ionization (before GEMs)
				//e[pixn] /= (gem1_gain*gem2_gain);
				// Convert to energy (KeV) using work function of He:C02 mixture
				//e[pixn] *= w*1E-3;
		}
		else if (tot[pixn]>2){
				// Convert from TOT to total charge collected by pixel
				e[pixn] = q*((c2*tot[pixn]-a2*b2)/(a2-tot[pixn])); 
				//cout << e[pixn] << endl;
				// Convert from total charge to primary ionization (before GEMs)
				//e[pixn] /= (gem1_gain*gem2_gain);
				// Convert to energy (KeV) using work function of He:C02 mixture
				//e[pixn] *= w*1E-3;
		}
		e_sum += e[pixn];

		vectors[pixn][0] = x;
		vectors[pixn][1] = y;
		vectors[pixn][2] = z;

		c_vector[0] += x;
		c_vector[1] += y;
		c_vector[2] += z;
      }

	  // Check if event is duplicate of earlier event in data tree
	  bool duplicate = 0;
	  bool duppix = 0;
	  for (int ev_num = 0; ev_num<jentry; ev_num++){
			  if (evt_points[ev_num] != npoints) continue;
			  else if (evt_points[ev_num] == npoints){
					  int prev_entry = dtr->GetEntry(ev_num);
					  int pix = static_cast<int>(MicrotpcMetaHits_m_pixNb[0]);
					  for (int pix_num=0; pix_num<pix; pix_num++){
	                 		if (MicrotpcDataHits_m_column[pix_num] != col[pix_num]) { 
	                 				duppix = 0;
	                 				break;
	                 		}
	                 		if (MicrotpcDataHits_m_row[pix_num] != row[pix_num]) {
	                 				duppix = 0;
	                 				break;
	                 		}
	                 		if (MicrotpcDataHits_m_BCID[pix_num] != bcid[pix_num]) {
	                 				duppix = 0;
	                 				break;
	                 		}
	                 		if (MicrotpcDataHits_m_TOT[pix_num] != tot[pix_num]) {
	                 				break;
	                 				duppix = 0;
	                 		}
	                 		duppix = 1;
					  }
					  if (duppix == 1){
							  duplicate = 1;
							  break;
					  }
			  }
	  }
	  
	  if (duplicate == 1) continue;

	  // Continue analyzing current event
	  getentry = dtr->GetEntry(jentry);

	  c_vector[0] /= npoints;
	  c_vector[1] /= npoints;
	  c_vector[2] /= npoints;

	  TVector3 centroid(c_vector[0],c_vector[1],c_vector[2]);
	  
	  // Calculate RMS of points from centroid
	  double c_rms2 = 0.0;
	  for (int i=0; i<npoints;i++){
		x = vectors[i][0];
		y = vectors[i][1];
		z = vectors[i][2];
		TVector3 temp(x,y,z);
		c_rms2 += (centroid - temp).Mag2();
		
		// Get the distance from the centroid
		TVector3 t(centroid - temp);
		distances[i]=t.Mag();

	  }
	  c_rms2 /= npoints;
	  c_rms = TMath::Power(c_rms2, 0.5);

	  getHitside();
	  
	  //Call fitter 
	  //if ((hitside == 11 || hitside == 0) && (npoints > 20 && tot_sum > 25)){
	  //if ((hitside == 11 || hitside == 0) && (npoints > 20 && e_sum > 98010)){
	  if ((hitside == 11 || hitside == 0) && npoints > 0){

	     if (hitside == 11 && MicrotpcRecoTracks_m_partID[0][4] == 1) top_alpha = 1;
		 else if (hitside == 11 && MicrotpcRecoTracks_m_partID[0][3] == 1) bottom_alpha = 1;
	     double x2,y2,z2;
		 double sigma;
	     for (int i = 0; i < npoints; ++i) {

		   // Reject outliers of distance/c_rms > 4 sigma

		   sigma = distances[i]/c_rms;
		   if (sigma > 4.0) {
			 tot_sum -= tot[i];
			 e_sum -= e[i];
		   }

		   else if (sigma < 4.0) {
		   // Calculate x, y, z positions in microns 
	         x2 = static_cast<double>(col[i]*250.0); 
	         y2 = static_cast<double>(row[i]*50.0);
	         z2 = -static_cast<double>(bcid[i]*250.0);

		     int n1 = m_gr->GetN();

		     m_gr->SetPoint(n1, x2, y2, z2);
		   }
		 }
	     fitTrack();
		 //de_dx = tot_sum/t_length;
		 if (t_length == 0) de_dx = 0;
		 else de_dx = e_sum/t_length;
		 //if (de_dx > 0.08 && hitside == 0) neutron = 1;
		 //else if (de_dx < 0.08 && npoints >= 40) proton = 1;
		 if (de_dx > 178.0 && hitside == 0) neutron = 1;
		 else if (de_dx < 179 && npoints >= 40) proton = 1;

	  }


	  else {
		 if (xray != 1) other = 1;
		
		 // Reset fit variables to 0 if track is not passed to fit function
	     chi2 = 0.;
	     theta = 0.;
	     phi = 0.;
		 de_dx = 0;
		 t_length = 0;

	     for (int m=0; m<5; m++){
	   	    par_fit[m] = 0.;
	   	    par_fit_err[m] = 0.;
		 }

		 for (int n=0; n<4; n++){
	   	    impact_pars[n] = 0.;
		 }
	  }
	  
	  //Fill tree
	  tr->Fill();

	  //Delete track
	  m_gr->Delete();
	  //if (jentry > 10000) break;
   }
   tr->Write();
   ofile->Write();
   ofile->Close();
}


int main(int argc, char * argv[])
{
  skimmer s;
  s.Loop(argv[1], argv[2]);
  return 1;
}
