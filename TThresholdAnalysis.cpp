#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "TThresholdAnalysis.h"

#include <TF1.h>
#include <TGraph.h>
#include <TMath.h>

TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *aScanHistoQue, TScan *aScan, TScanConfig *aScanConfig, std::mutex *aMutex) : TScanAnalysis(aScanHistoQue, aScan, aScanConfig, aMutex)  
{
  
  // It is pulse amplitude, not charge, yet.
  m_startPulseAmplitude = m_config->GetChargeStart();
  m_stopPulseAmplitude  = m_config->GetChargeStop();
  m_stepPulseAmplitude  = m_config->GetChargeStep();
  m_nPulseInj           = m_config->GetNInj();
  
  m_fDoDumpRawData = true;
  m_fDoFit         = true;
  
}

//TODO: Implement HasData.
bool TThresholdAnalysis::HasData(TScanHisto &histo, TChipIndex idx, int col) 
{
  return true;
}

std::string TThresholdAnalysis::GetFileName(TChipIndex aChipIndex, std::string fileType)
{
  
  std::string fileName;
  fileName+=  m_config->GetfNameSuffix();
  fileName+= "-";
  fileName+= m_analisysName;
  fileName+= "-B";
  fileName+= std::to_string(aChipIndex.boardIndex);
  fileName+= "-Rx";
  fileName+= std::to_string(aChipIndex.dataReceiver);
  fileName+= "-chip";
  fileName+= std::to_string(aChipIndex.chipId);
  fileName+= "-";
  fileName+= fileType;
  fileName+= ".dat";
  
  return fileName;

}

bool TThresholdAnalysis::CheckPixelNoHits(TGraph* aGraph)
{
  
  for (int itrPoint=0; itrPoint<aGraph->GetN(); itrPoint++){
    double x =0;
    double y =0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y!=0){return false;} 
  }
  
  return true;
}

bool TThresholdAnalysis::CheckPixelStuck(TGraph* aGraph)
{
  double y_dummy =0;
  for (int itrPoint=0; itrPoint<aGraph->GetN(); itrPoint++){
    double x =0;
    double y =0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y<0.5*m_nPulseInj){return false;} 
  }
  
  return true;
}

double ErrorFunc(double* x, double* par)
{
  double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  return y;
}

std::pair<double,double> TThresholdAnalysis::DoFit(TGraph* aGraph)
{
  TF1* fitfcn = new TF1("fitfcn", 
			ErrorFunc,
			m_startPulseAmplitude*m_electronPerDac,
			m_stopPulseAmplitude*m_electronPerDac,
			4);
  // y@50%.
  fitfcn->SetParameter(0,0.5*m_nPulseInj); 
  // 0.5 of max. amplitude.
  fitfcn->SetParameter(1,0.5*m_nPulseInj); 
  // x@50%.
  fitfcn->SetParameter(2,0.5*(m_stopPulseAmplitude - m_startPulseAmplitude)*m_electronPerDac);
  // slope of s-curve.
  fitfcn->SetParameter(3,0.5);
  
  aGraph->Fit("fitfcn","RQ");
  
  double thresh=fitfcn->GetParameter(0);
  double noise =fitfcn->GetParameter(1);
  //double chi2  =fitfcn->GetChisquare()/fitfcn->GetNDF();
  
  // Make a struct for fit result!
  std::pair<double,double> fitResult_dummy;
  fitResult_dummy.first =thresh;
  fitResult_dummy.second=noise;
  
  delete fitfcn; 
  
  return fitResult_dummy;
}

void TThresholdAnalysis::Initialize()
{
  
  std::cout << "Initializing " << m_analisysName << std::endl;
  
  while (!m_scan->IsRunning()){sleep(1);}
  
  // Initialize counters.
  m_counterPixelsNoHits     =0;
  m_counterPixelsNoThreshold=0;
  m_counterPixelsStuck      =0;
  
  m_sumGoodThresholds    =0;
  m_counterGoodThresholds=0;
  
  // TODO
  // open files.
  
}

//=MK TODO: clean up
//=MB cleaned and extended. Further improvement
// possible.
void TThresholdAnalysis::Run() 
{
  
  while (m_histoQue->size() == 0) {
    sleep(1);
  }
  
  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    
    if (m_histoQue->size() <= 0) {usleep (300);continue;}
    
    while (!(m_mutex->try_lock()));
      
    TScanHisto scanHisto = m_histoQue->front();      
    scanHisto.GetChipList(m_chipList);
    
    m_histoQue->pop_front();
    m_mutex->unlock();
    
    int row = scanHisto.GetIndex();
    
    std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
    for (int iChip = 0; iChip < m_chipList.size(); iChip++) {
      
      std::string fileNameDummy = GetFileName(m_chipList.at(iChip), "PixelsNoHits");
      m_filePixelNoHits         = fopen (fileNameDummy.c_str(), "a");
      fileNameDummy             = GetFileName(m_chipList.at(iChip), "PixelsStuck");
      m_filePixelStuck          = fopen (fileNameDummy.c_str(), "a");
      fileNameDummy             = GetFileName(m_chipList.at(iChip), "PixelsNoThreshold");
      m_filePixelNoThreshold    = fopen (fileNameDummy.c_str(), "a");
      
      if(m_fDoDumpRawData){
  	fileNameDummy = GetFileName(m_chipList.at(iChip), "RawData");
  	m_fileRawData = fopen (fileNameDummy.c_str(), "a");
      }
      if(m_fDoFit){
  	fileNameDummy = GetFileName(m_chipList.at(iChip), "FitResult");
	m_fileFitResults = fopen (fileNameDummy.c_str(), "a");
      }
      
      //= MB: 1024 should be replaced with const int n_cols.
      for (int iCol = 0; iCol < 1024; iCol ++) {
	
  	TGraph* gDummy = new TGraph();
	
  	int pulseRangeDummy = ((float)abs( m_startPulseAmplitude - m_stopPulseAmplitude))/ m_stepPulseAmplitude;
	
  	for (int iPulse = 0; iPulse < pulseRangeDummy; iPulse++) {
	  
  	  int entries =(int)scanHisto(m_chipList.at(iChip), 
  				      iCol, 
  				      iPulse);
	  
  	  gDummy->SetPoint(gDummy->GetN(),
			   iPulse*m_electronPerDac,
			   entries);
	  
  	  if(m_fDoDumpRawData){
  	    fprintf(m_fileRawData, 
  		    "%d %d %d %d\n", 
  		    iCol,row,iPulse,entries);
  	  }
	  
  	} // end loop over iPulse.
	
  	if (gDummy->GetN()==0){delete gDummy;continue;}
	
  	bool fPixelNoHits= CheckPixelNoHits(gDummy);
  	bool fPixelStuck = CheckPixelStuck (gDummy);
	
	if (fPixelNoHits){
  	  fprintf(m_filePixelNoHits, 
  		  "%d %d\n", 
  		  iCol,row);
	  
  	  // TODO: extend to all sizes.
  	  if (m_chipList.size()==1){ 
	    m_counterPixelsNoHits++;
  	  }
  	} else if (fPixelStuck){
	  fprintf(m_filePixelStuck, 
		  "%d %d\n", 
		  iCol,row);
	  
	  // TODO: extend to all sizes.
  	  if (m_chipList.size()==1){
  	    m_counterPixelsStuck++;
  	  }
  	} else if (m_fDoFit){
	  std::pair<double,double> fitResult;
	  fitResult=DoFit(gDummy);
	  
	  fprintf(m_fileFitResults, 
  		  "%d %d %f %f\n", 
  		  iCol,row,
		  fitResult.first,  //Threshold
		  fitResult.second); //Noise
	}
	
  	delete gDummy;
      } // end loop over iCol.
      
      fclose(m_filePixelNoHits);
      fclose(m_filePixelStuck);
      fclose(m_filePixelNoThreshold);
      if(m_fDoDumpRawData){fclose(m_fileRawData);}
      
    } // end loop over iChip.
  } // end of "while" on m_scan and m_scanHistoQue.
  
}

void TThresholdAnalysis::Finalize()
{
  std::cout << "Finalizing " 
	    << m_analisysName 
	    << std::endl;
  
  // TODO
  //Close files.
  
}
