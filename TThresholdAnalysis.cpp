#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "TThresholdAnalysis.h" 

#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"

#include <TF1.h>
#include <TGraph.h>
#include <TMath.h>
#include <TPaveText.h>

TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *aScanHistoQue,
                                       TScan *aScan,
                                       TScanConfig *aScanConfig,
                                       std::vector <THic*> hics,
                                       std::mutex *aMutex,
                                       int resultFactor)
  : TScanAnalysis(aScanHistoQue, aScan, aScanConfig, hics, aMutex) 
{
  
  // N.B.:It is pulse amplitude, not charge yet.
  m_startPulseAmplitude = m_config->GetChargeStart();
  m_stopPulseAmplitude  = m_config->GetChargeStop();
  m_stepPulseAmplitude  = m_config->GetChargeStep();
  m_nPulseInj           = m_config->GetNInj();
  
  m_fDoFit         = true;
  m_resultFactor   = resultFactor;
  
  m_result = new TThresholdResult();
}

//TODO: Implement HasData.
bool TThresholdAnalysis::HasData(TScanHisto &histo, common::TChipIndex idx, int col) 
{
  return true;
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

double TThresholdAnalysis::FindStart(TGraph* aGraph)
{
  
  double xA = 0;
  double yA = 0;
  aGraph->GetPoint(0, xA, yA);
  
  double xB = 0;
  double yB = 0;
  aGraph->GetPoint(aGraph->GetN()-1, xB, yB);
  
  int initPoint = 0;
  int endPoint = 0;
  int incr = 0;
  
  if (yA==yB){
    return -1;
  } else if(yA<yB){
    initPoint=0;
    endPoint=aGraph->GetN()-1;
    incr = +1;
  } else if(yA>yB){
    initPoint=aGraph->GetN()-1;
    endPoint=0;
    incr = -1;
  }
  
  double upperPointX =0;
  double upperPointY =0;
  for (int itrPoint=initPoint; itrPoint<=endPoint; itrPoint+=incr){
    double xDummy =0;
    double yDummy =0;
    aGraph->GetPoint(itrPoint, xDummy, yDummy);
    
    if (yDummy == m_nPulseInj) {
      upperPointX = xDummy;
      upperPointY = yDummy;
      break;
    }
    
  }
  
  double lowerPointX =0;
  double lowerPointY =0;
  for (int itrPoint=endPoint; itrPoint>=initPoint; itrPoint-=incr){
    double xDummy =0;
    double yDummy =0;
    aGraph->GetPoint(itrPoint, xDummy, yDummy);
    
    if (yDummy == 0) {
      lowerPointX = xDummy;
      lowerPointY = yDummy;
      break;
    }
    
  }
  
  return (upperPointX + lowerPointX)*0.5;
  
}


double ErrorFunc(double* x, double* par)
{
  double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  return y;
}

common::TErrFuncFitResult TThresholdAnalysis::DoFit(TGraph* aGraph)
{
  TF1 *fitfcn = new TF1("fitfcn",
			ErrorFunc,
			m_stopPulseAmplitude*m_electronPerDac,
			m_startPulseAmplitude*m_electronPerDac,
			4);
  
  // y@50%.
  fitfcn->SetParameter(0,0.5*m_nPulseInj); 
  // 0.5 of max. amplitude.
  fitfcn->SetParameter(1,0.5*m_nPulseInj); 
  // x@50%.
  fitfcn->SetParameter(2,FindStart(aGraph));
  // slope of s-curve. 
  fitfcn->SetParameter(3,10);
  
  common::TErrFuncFitResult fitResult_dummy;
  fitResult_dummy.status = aGraph->Fit("fitfcn","RQ");
  fitResult_dummy.threshold = fitfcn->GetParameter(2);
  fitResult_dummy.noise     = fitfcn->GetParameter(3);
  fitResult_dummy.redChi2   = fitfcn->GetChisquare()/fitfcn->GetNDF();
  
  // std::cout << fitResult_dummy.threshold << "\t"
  //  	    << fitResult_dummy.noise << "\t"
  //  	    << fitResult_dummy.redChi2 << "\t"
  //  	    << fitResult_dummy.status << "\t"
  //  	    << std::endl;
  
  delete fitfcn; 
  
  return fitResult_dummy;
}

void TThresholdAnalysis::Initialize()
{
  
  // TBC: Necessary ???
  while (!m_scan->IsRunning()){sleep(1);}
  
  std::cout << "Initializing " << m_analisysName << std::endl;
  
  ReadChipList      ();
  CreateChipResults ();
  
  for (int iChip = 0; iChip < m_chipList.size();iChip ++ ) {
    
    TThresholdResultChip* chipResult = (TThresholdResultChip*) m_result->GetChipResult(m_chipList.at(iChip));
    
    chipResult->m_boardIndex=m_chipList.at(iChip).boardIndex; 
    chipResult->m_dataReceiver=m_chipList.at(iChip).dataReceiver;
    chipResult->m_chipId=m_chipList.at(iChip).chipId;
    
    chipResult->m_counterPixelsNoHits=0;
    chipResult->m_counterPixelsStuck=0;
    chipResult->m_counterPixelsNoThreshold=0;
    chipResult->m_counterPixelsYesThreshold=0;
    
    chipResult->m_threshold.sum=0;
    chipResult->m_threshold.sum2=0;
    chipResult->m_threshold.entries=0;
    
    chipResult->m_noise.sum=0;
    chipResult->m_noise.sum2=0;
    chipResult->m_noise.entries=0;
    
  }
  
  TThresholdResult* thresholdResult = (TThresholdResult*)m_result;
  char fName[100];
  
  sprintf (fName, "%s_%s_Summary.dat", 
	   m_analisysName,m_config->GetfNameSuffix());
  thresholdResult->SetFileSummary(fopen(fName,"w"));
  
  sprintf (fName, "%s_%s_PixelByPixel.dat", 
	   m_analisysName,m_config->GetfNameSuffix());
  thresholdResult->SetFilePixelByPixel(fopen(fName,"w"));
  
  fprintf(thresholdResult->GetFilePixelByPixel(),
	  "#%s %s %s %s %s %s %s %s\n", 
	  "BrdId",
	  "RxId",
	  "ChipId",
	  "Col","Row",
	  "Thresh[e]",
	  "Noise[e]",
	  "redChi2");
  
}

void TThresholdAnalysis::Run() 
{
  
  while (m_histoQue->size() == 0){sleep(1);}
  
  while ((m_scan->IsRunning() || (m_histoQue->size() > 0))) {
    
    if (m_histoQue->size()<= 0){usleep(300);continue;}
    
    while (!(m_mutex->try_lock()));
    
    TScanHisto scanHisto = m_histoQue->front();      
    
    m_histoQue->pop_front();
    m_mutex->unlock();
    
    int row = scanHisto.GetIndex();
    
    std::cout << "ANALYSIS: Found histo for row " << row << ", size = " << m_histoQue->size() << std::endl;
    
    for (int iChip = 0; iChip < m_chipList.size(); iChip++) {
      
      for (int iCol = 0; iCol < common::nCols; iCol ++) {
      
	TGraph* gDummy = new TGraph();
	
	int pulseRangeDummy = ((float)abs( m_startPulseAmplitude - m_stopPulseAmplitude))/ m_stepPulseAmplitude;
      
	for (int iPulse = 0; iPulse < pulseRangeDummy; iPulse++) {
      
	  int entries =(int)scanHisto(m_chipList.at(iChip), 
				      iCol, 
				      iPulse);
	  
	  gDummy->SetPoint(gDummy->GetN(),
			   iPulse*m_electronPerDac,
			   entries);
	  
	} // end loop over iPulse.
	
	if (gDummy->GetN()==0){ delete gDummy;continue;}
	
	TThresholdResultChip* chipResult = (TThresholdResultChip*) m_result->GetChipResult(m_chipList.at(iChip));
	
	bool fPixelNoHits= CheckPixelNoHits(gDummy);
       	bool fPixelStuck = CheckPixelStuck (gDummy);
	
	if (fPixelNoHits){
	  chipResult->m_counterPixelsNoHits++;
	} else if (fPixelStuck){
	  chipResult->m_counterPixelsStuck++;
	} else if (m_fDoFit){
	  
	  common::TErrFuncFitResult fitResult;
	  fitResult=DoFit(gDummy);
	  
	  TThresholdResult* thresholdResult = (TThresholdResult*)m_result;
	  fprintf(thresholdResult->GetFilePixelByPixel(),
	    	  "%d %d %d %d %d %f %f %f\n", 
	   	  chipResult->m_boardIndex,
	    	  chipResult->m_dataReceiver,
	    	  chipResult->m_chipId,
	    	  iCol,row,
	    	  fitResult.threshold,
	   	  fitResult.noise,
		  fitResult.redChi2); 
	  
	  // MB - NEED TO SELECT GOOD FIT.
	  m_cutChi2 = 5;// To implement from MK -> MB TBC.
	  if (fitResult.status!=0)
	    {chipResult->m_counterPixelsNoThreshold++;continue;}
	  
	  chipResult->m_counterPixelsYesThreshold++;
	  
	  chipResult->m_threshold.sum+=fitResult.threshold;
	  chipResult->m_threshold.sum2+=pow(fitResult.threshold,2);
	  chipResult->m_threshold.entries+=1;
	  
	  chipResult->m_noise.sum+=fitResult.noise;
	  chipResult->m_noise.sum2+=pow(fitResult.noise,2);
	  chipResult->m_noise.entries+=1;
	  
	}
	
	delete gDummy;
      } // end loop over iCol.
      
    } // end loop over iChip.
  } // end of "while" on m_scan and m_scanHistoQue.
  
}

void TThresholdAnalysis::Finalize()
{
  
  std::cout << "Finalizing " 
	    << m_analisysName 
	    << std::endl;
  
  TThresholdResult* thresholdResult = (TThresholdResult*)m_result;
  
  for (int iChip=0; iChip < m_chipList.size(); iChip++) {
    
    TThresholdResultChip* chipResult = (TThresholdResultChip*) m_result->GetChipResult(m_chipList.at(iChip));
    
    fprintf(thresholdResult->GetFileSummary(),
	    "\nBoard %d, Receiver %d, Chip %d\n",
	    chipResult->m_boardIndex,
       	    chipResult->m_dataReceiver, 
	    chipResult->m_chipId);
    fprintf(thresholdResult->GetFileSummary(),
	    "Pixels without hits[CAREFUL]: %d\n", 
     	    chipResult->m_counterPixelsNoHits);
    fprintf(thresholdResult->GetFileSummary(),
	    "Stuck pixels: %d\n", 
     	    chipResult->m_counterPixelsStuck);
    fprintf(thresholdResult->GetFileSummary(),
	    "Pixels without threshold: %d\n", 
     	    chipResult->m_counterPixelsNoThreshold);
    fprintf(thresholdResult->GetFileSummary(),
	     "Pixels with threshold: %d\n", 
	     chipResult->m_counterPixelsYesThreshold);
    
    if (chipResult->m_threshold.entries!=0){
      chipResult->m_thresholdMean = chipResult->m_threshold.sum/chipResult->m_threshold.entries;
      
      chipResult->m_thresholdStdDev= sqrt((chipResult->m_threshold.sum2/chipResult->m_threshold.entries) - pow(chipResult->m_thresholdMean,2) );
    }
    
    fprintf(thresholdResult->GetFileSummary(), 
	    "Threshold[e] %f +/- %f\n",
     	    chipResult->m_thresholdMean,
      	    chipResult->m_thresholdStdDev);
    
    if (chipResult->m_noise.entries!=0){
      chipResult->m_noiseMean = chipResult->m_noise.sum/chipResult->m_noise.entries;
      chipResult->m_noiseStdDev=sqrt((chipResult->m_noise.sum2/
				      chipResult->m_noise.entries)
				     - pow(chipResult->m_noiseMean,2) );
    }
    
    fprintf(thresholdResult->GetFileSummary(), 
	    "Noise[e] %f +/- %f\n",
     	    chipResult->m_noiseMean,
     	    chipResult->m_noiseStdDev);
    
  }
  
  fclose(thresholdResult->GetFileSummary());
  fclose(thresholdResult->GetFilePixelByPixel());
  
}

float TThresholdAnalysis::GetResultThreshold(int chip) {
  
  TThresholdResultChip* chipResult = (TThresholdResultChip*) m_result->GetChipResult(m_chipList.at(chip));
  return chipResult->m_thresholdMean;
  
}
