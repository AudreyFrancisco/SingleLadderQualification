#include <iomanip>
#include <iostream>
#include <utility>
#include <vector>

#include "TThresholdAnalysis.h" 

#include "THisto.h"
#include "TScan.h"
#include "TScanConfig.h"

#include <TCanvas.h> /*H*/
#include <TF1.h>
#include <TFile.h> /*H*/
#include <TGraph.h>
#include <TH1D.h> /*H*/
#include <TH2D.h> /*H*/
#include <TMath.h>
#include <TPaveText.h>

TThresholdResultChip::TThresholdResultChip (): TScanResultChip()
{;}

TThresholdResultChip::~TThresholdResultChip ()
{;}
 
void TThresholdResultChip::SetBoardIndex (unsigned int aBoardIndex)
{m_boardIndex = aBoardIndex;} 

void TThresholdResultChip::SetDataReceiver(unsigned int aDataReceiver)
{m_dataReceiver = aDataReceiver;}

void TThresholdResultChip::SetChipId(unsigned int aChipId)
{m_chipId = aChipId;}

void TThresholdResultChip::SetVPulseL(int aVPulseL)
{m_vPulseL = aVPulseL;}    

void TThresholdResultChip::SetVPulseH(int aVPulseH)
{m_vPulseH = aVPulseH;}    

void TThresholdResultChip::SetVPulseStep(int aVPulseStep)
{m_vPulseStep = aVPulseStep;} 

void TThresholdResultChip::SetNMask(int aNMask)
{m_nMask = aNMask;}
  
void TThresholdResultChip::SetCounterPixelsNoHits(int aCounterPixelsNoHits)
{m_counterPixelsNoHits = aCounterPixelsNoHits;}

void TThresholdResultChip::SetCounterPixelsStuck(int aCounterPixelsStuck)
{m_counterPixelsStuck = aCounterPixelsStuck;}

void TThresholdResultChip::SetCounterPixelsNoThreshold(int aCounterPixelsNoThreshold)
{m_counterPixelsNoThreshold = aCounterPixelsNoThreshold;}
  
void TThresholdResultChip::SetThresholdMean(double aThresholdMean){m_thresholdMean =  aThresholdMean;}

void TThresholdResultChip::SetThresholdStdDev(double aThresholdStdDev)
{m_thresholdStdDev = aThresholdStdDev;}

void TThresholdResultChip::SetNoiseMean(double aNoiseMean)
{m_noiseMean =  aNoiseMean;}

void TThresholdResultChip::SetNoiseStdDev(double aNoiseStdDev)
{m_noiseStdDev  = aNoiseStdDev;}

unsigned int TThresholdResultChip::GetBoardIndex()
{return m_boardIndex;} 

unsigned int TThresholdResultChip::GetDataReceiver()
{return m_dataReceiver;}

unsigned int TThresholdResultChip::GetChipId()
{return m_chipId;}

int TThresholdResultChip::GetVPulseL()
{return m_vPulseL;}

int TThresholdResultChip::GetVPulseH()
{return m_vPulseH;}

int TThresholdResultChip::GetVPulseStep()
{return m_vPulseStep;}

int TThresholdResultChip::GetNMask()
{return m_nMask;}

int TThresholdResultChip::GetCounterPixelsNoHits()
{return m_counterPixelsNoHits;}

int TThresholdResultChip::GetCounterPixelsStuck()
{return m_counterPixelsStuck;}

int TThresholdResultChip::GetCounterPixelsNoThreshold()
{return m_counterPixelsNoThreshold;}
  
double TThresholdResultChip::GetThresholdMean()
{return m_thresholdMean;}

double TThresholdResultChip::GetThresholdStdDev()
{return m_thresholdStdDev;}

double TThresholdResultChip::GetNoiseMean()
{return m_noiseMean;}

double TThresholdResultChip::GetNoiseStdDev()
{return m_noiseStdDev;}

// ================================
// ================================

TThresholdResult::TThresholdResult (): TScanResult () {;}

TThresholdResult::~TThresholdResult () {;}

void TThresholdResult::SetFileHicResult(FILE* aFileName)
{m_fileHicResult = aFileName;}

void TThresholdResult::SetFilePixelByPixelResult(FILE* aFileName)
{m_filePixelByPixelResult = aFileName;}

void TThresholdResult::SetFileStuckPixels(FILE* aFileName)
{m_fileStuckPixels = aFileName;}

FILE* TThresholdResult::GetFileHicResult()
{return m_fileHicResult;}

FILE* TThresholdResult::GetFilePixelByPixelResult()
{return m_filePixelByPixelResult;}

FILE* TThresholdResult::GetFileStuckPixels()
{return m_fileStuckPixels;}

// ================================
// ================================


TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *aScanHistoQue,
                                       TScan                  *aScan,
                                       TScanConfig            *aScanConfig,
                                       std::vector <THic*>     hics,
                                       std::mutex             *aMutex,
                                       int                     resultFactor)
: TScanAnalysis(aScanHistoQue, aScan, aScanConfig, hics, aMutex)  
{
  
  // It is pulse amplitude, not charge, yet.
  m_startPulseAmplitude = m_config->GetChargeStart();
  m_stopPulseAmplitude  = m_config->GetChargeStop();
  m_stepPulseAmplitude  = m_config->GetChargeStep();
  m_nPulseInj           = m_config->GetNInj();
  
  m_fDoDumpRawData = true;
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
  
  // double yMin = TMath::MinElement(aGraph->GetN(),aGraph->GetY());
  // double yMax = TMath::MaxElement(aGraph->GetN(),aGraph->GetY());
  
  // if (yMin!=0 || yMax<=yMin || yMax>m_nPulseInj){return -1;}
  
  // double yMiddle = 0.5*(yMax - yMin);
  
  // int minPoint = TMath::LocMin( aGraph->GetN(),aGraph->GetY() );
  // int maxPoint = TMath::LocMax( aGraph->GetN(),aGraph->GetY() );
  
  // std::cout << "Y: " << yMin << ";" << yMax << std::endl;
  // std::cout << "X: " << minPoint << ";" << maxPoint << std::endl;
  
  // for (int itrPoint=minPoint; itrPoint<maxPoint; itrPoint++){
  //   double x =0;
  //   double y =0;
  //   aGraph->GetPoint(itrPoint, x, y);
  //   //std::cout << itrPoint << ":" << x << ";" << y << std::endl;
  //   if (y>=yMiddle){return x;}  
  // }
  
}

double ErrorFunc(double* x, double* par)
{
  double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  return y;
}

common::TErrFuncFitResult TThresholdAnalysis::DoFit(TGraph* aGraph)
{
  TF1 *fitfcn;
  if(m_resultFactor<1) {
    fitfcn = new TF1("fitfcn",
                          ErrorFunc,
                          m_stopPulseAmplitude*m_resultFactor,
                          m_startPulseAmplitude*m_resultFactor,
                          4);
  } else {
    fitfcn = new TF1("fitfcn", 
  			  ErrorFunc,
  			  m_startPulseAmplitude*m_resultFactor,
			  m_stopPulseAmplitude*m_resultFactor,
			  4);
  }
  // y@50%.
  fitfcn->SetParameter(0,0.5*m_nPulseInj); 
  // 0.5 of max. amplitude.
  fitfcn->SetParameter(1,0.5*m_nPulseInj); 
  // x@50%.
  fitfcn->SetParameter(2,0.5*(m_stopPulseAmplitude - m_startPulseAmplitude)*m_resultFactor);
  // slope of s-curve.  m_resultFactor MAY BE -1--make sure this doesn't cause any problems!! (WIP)
  fitfcn->SetParameter(3,0.5);
  
  aGraph->Fit("fitfcn","RQ");
  
  common::TErrFuncFitResult fitResult_dummy;
  if(fitfcn->GetParameter(0)>0 && m_resultFactor<0) {
    std::cout << "ERROR in line 241 of TAnalogAnalysis:  Unexpected resultFactor/threshold sign!" << std::endl;
    fitResult_dummy.threshold = 1;
    return fitResult_dummy;
    if(fitfcn->GetParameter(0) < 0) {
      fitResult_dummy.threshold = -1*fitfcn->GetParameter(0);  //for the ithr case
    } else {
      fitResult_dummy.threshold = fitfcn->GetParameter(0);
    }
    fitResult_dummy.noise     = fitfcn->GetParameter(1);
    fitResult_dummy.redChi2   = fitfcn->GetChisquare()/fitfcn->GetNDF();
  }
  delete fitfcn; 
  
  return fitResult_dummy;
}

void TThresholdAnalysis::Initialize()
{
  
  // Retrieving HistoMap from TThresholdScan, after it is initialized.
  // Creating output files.
  // Initializing TThresholdResult variables.
  // Initializing counters.
  
  while (!m_scan->IsRunning()){sleep(1);}
  
  std::cout << "Initializing " << m_analisysName << std::endl;
  
  std::string fileNameDummy;
  fileNameDummy  = m_analisysName;
  fileNameDummy += "-";
  fileNameDummy += m_config->GetfNameSuffix();
  fileNameDummy += "-HicResult";
  fileNameDummy += ".dat";
  
  m_resultThreshold->SetFileHicResult(fopen(fileNameDummy.c_str(),"w"));

  fileNameDummy  = m_analisysName;
  fileNameDummy += "-";
  fileNameDummy += m_config->GetfNameSuffix();
  fileNameDummy += "-PixelByPixel";
  fileNameDummy += ".dat"; 
  
  m_resultThreshold->SetFilePixelByPixelResult(fopen(fileNameDummy.c_str(),"w"));
  
  fileNameDummy  = m_analisysName;
  fileNameDummy += "-";
  fileNameDummy += m_config->GetfNameSuffix();
  fileNameDummy += "-StuckPixels";
  fileNameDummy += ".dat"; 
  
  m_resultThreshold->SetFileStuckPixels(fopen(fileNameDummy.c_str(),"w"));
    
  TScanHisto histoDummy = m_scan->GetTScanHisto();
  
  std::map<int, THisto> histoMap_dummy= histoDummy.GetHistoMap();
  
  for (std::map<int, THisto>::iterator itr=histoMap_dummy.begin();
       itr!=histoMap_dummy.end(); 
       ++itr) {
    
    common::TChipIndex chipIndexDummy = common::GetChipIndex(itr->first);
    
    m_chipList.push_back(chipIndexDummy);
    
    TThresholdResultChip* dummyResultChip = new TThresholdResultChip();
    dummyResultChip->SetCounterPixelsNoHits(0);
    dummyResultChip->SetCounterPixelsStuck(0);
    dummyResultChip->SetCounterPixelsNoThreshold(0);
    
    m_resultThreshold->AddChipResult(itr->first,
    				     dummyResultChip);
  }
  
  std::pair<int,common::TStatVar> pairDummy;
  for (std::map<int, THisto>::iterator itr=histoMap_dummy.begin();
       itr!=histoMap_dummy.end(); 
       ++itr) {
    
    common::TStatVar thresholdDummy;
    thresholdDummy.sum=0;
    thresholdDummy.sum2=0;
    thresholdDummy.entries=0;
    pairDummy = std::make_pair(itr->first,thresholdDummy);
    m_threshold.insert(pairDummy);
    
    common::TStatVar noiseDummy;
    noiseDummy.sum=0;
    noiseDummy.sum2=0;
    noiseDummy.entries=0;
    pairDummy = std::make_pair(itr->first,noiseDummy);
    m_noise.insert(pairDummy);
  } 
  
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
	int iPulseStart;
        int iPulseStop;
    	TGraph* gDummy = new TGraph();
	if(m_resultFactor > 1) { //regular scan
   	  iPulseStop = ((float)abs( m_startPulseAmplitude - m_stopPulseAmplitude))/ m_stepPulseAmplitude;
          iPulseStart = 0;
	} else if(m_resultFactor==1) { //vcasn
          iPulseStart = 40; //range of vcasn values scanned over.
          iPulseStop  = 60; //not changing in the forseeable future...but might.
        } else { //else ithr
          iPulseStart = 30;
          iPulseStop  = 100;
        }
   	for (int iPulse = iPulseStart; iPulse < iPulseStop; iPulse++) {
	  
    	  int entries =(int)scanHisto(m_chipList.at(iChip), 
    				      iCol, 
    				      iPulse);
	  
   	  gDummy->SetPoint(gDummy->GetN(),
   			   iPulse*m_resultFactor,
   			   entries);
	  
	  if(m_fDoDumpRawData){;}
	  
	} // end loop over iPulse.
	
	if (gDummy->GetN()==0){ delete gDummy;continue;}
	
	common::TChipIndex dummyChipIndex = m_chipList.at(iChip);
	
	TThresholdResultChip* resultDummy = (TThresholdResultChip*) m_resultThreshold->GetChipResult(dummyChipIndex);
	
	bool fPixelNoHits= CheckPixelNoHits(gDummy);
	bool fPixelStuck = CheckPixelStuck (gDummy);
	
	if (fPixelNoHits){
	  int dummyCounter = resultDummy->GetCounterPixelsNoHits();
	  resultDummy->SetCounterPixelsNoHits(dummyCounter+1);
	} else if (fPixelStuck){
	  int dummyCounter = resultDummy->GetCounterPixelsStuck();
	  resultDummy->SetCounterPixelsStuck(dummyCounter+1);
	} else if (m_fDoFit){
	  
	  common::TErrFuncFitResult fitResult;
	  fitResult=DoFit(gDummy);
	  
	  fprintf(m_resultThreshold->GetFilePixelByPixelResult(),
		  "%d %d %d %d %d %f %f %f\n", 
		  dummyChipIndex.boardIndex,
		  dummyChipIndex.dataReceiver,
		  dummyChipIndex.chipId,
		  iCol,row,
		  fitResult.threshold,
		  fitResult.noise,
		  fitResult.redChi2);
	  
	  // MB - NEED TO SELECT GOOD FIT.
	  // if (fitResult.status!=4){continue;}
	  int maxRedChi2 = 5;// From MK.
	  
	  for (int itr=0; itr<gDummy->GetN(); itr++) {
	    
	    double x =0;
	    double y =0;
	    
	    gDummy->GetPoint(itr,x,y);
	    
	    hSuperDummyA->Fill(x,y);/*H*/
	    if (fitResult.status==0){
	      hSuperDummyB->Fill(x,y);/*H*/
	    }else if (fitResult.status==4){
	      hSuperDummyC->Fill(x,y);/*H*/
	    }
	    
	  }
	  
	  int intIndexDummy = common::GetChipIntIndex(dummyChipIndex);
	  m_threshold.at(intIndexDummy).sum+=fitResult.threshold;
	  m_threshold.at(intIndexDummy).sum2+=pow(fitResult.threshold,2);
	  m_threshold.at(intIndexDummy).entries+=1;
	  m_noise.at(intIndexDummy).sum+=fitResult.noise;
	  m_noise.at(intIndexDummy).sum2+=pow(fitResult.noise,2);
	  m_noise.at(intIndexDummy).entries+=1;
	  
	  hSuperMeanA->Fill(fitResult.threshold);/*H*/
	  hSuperNoiseA->Fill(fitResult.noise);/*H*/
	  hSuperRedChi2A->Fill(fitResult.redChi2);/*H*/
	  hSuperStatusA->Fill(fitResult.status);/*H*/
	  
	  hSuperNPointsVsStatusA->Fill(fitResult.status, /*H*/
				       gDummy->GetN());/*H*/
	  hSuperNPointsVsChi2A->Fill(fitResult.redChi2, 
				     gDummy->GetN());/*H*/
	  
	  if (fitResult.status==0) {
	    hSuperMeanB->Fill(fitResult.threshold);/*H*/
	    hSuperNoiseB->Fill(fitResult.noise);/*H*/
	    hSuperRedChi2B->Fill(fitResult.redChi2);/*H*/
	    hSuperStatusB->Fill(fitResult.status);/*H*/
	    
	    hSuperNPointsVsStatusB->Fill(fitResult.status, /*H*/
					 gDummy->GetN());/*H*/
	    hSuperNPointsVsChi2B->Fill(fitResult.redChi2, 
				       gDummy->GetN());/*H*/
	  } else if (fitResult.status==4) {
	    hSuperMeanC->Fill(fitResult.threshold);/*H*/
	    hSuperNoiseC->Fill(fitResult.noise);/*H*/
	    hSuperRedChi2C->Fill(fitResult.redChi2);/*H*/
	    hSuperStatusC->Fill(fitResult.status);/*H*/
	    
	    hSuperNPointsVsStatusC->Fill(fitResult.status, /*H*/
					 gDummy->GetN());/*H*/
	    hSuperNPointsVsChi2C->Fill(fitResult.redChi2, 
				       gDummy->GetN());/*H*/
	  }
	  
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
  
  // Sanity check.
  // if ( m_resultChip.size()!=m_threshold.size() || 
  //      m_resultChip.size()!=m_noise.size()){ 
  //   std::cout<< "ERROR in " 
  //  	     << m_analisysName  
  //  	     << "!!!"
  //  	     <<std::endl; exit(EXIT_FAILURE);;
  // }
  
  for (std::map<int,common::TStatVar>::iterator itr=m_threshold.begin();
       itr!=m_threshold.end(); 
       ++itr) {
    
    if (itr->second.entries==0){continue;}
    
    double mean   = itr->second.sum/itr->second.entries;
    double stdDev = sqrt((itr->second.sum2/itr->second.entries) 
    			 - pow(mean,2) );
    
    itr->second.mean   = mean;
    itr->second.stdDev = stdDev;
  }
  
  for (std::map<int,common::TStatVar>::iterator itr=m_noise.begin();
       itr!=m_noise.end(); 
       ++itr) {
    
    if (itr->second.entries==0){continue;}
    
    double mean   = itr->second.sum/itr->second.entries;
    double stdDev = sqrt((itr->second.sum2/itr->second.entries) 
    			 - pow(mean,2) );
    
    itr->second.mean   = mean;
    itr->second.stdDev = stdDev;
    
  }
  
  for (int iChip=0; iChip < m_chipList.size(); iChip++) {
    
    common::TChipIndex dummyChipIndex = m_chipList.at(iChip);
    
    TThresholdResultChip* resultDummy = (TThresholdResultChip*) m_resultThreshold->GetChipResult(dummyChipIndex);
    
    int dummyIntIndex = common::GetChipIntIndex(dummyChipIndex);
    
    resultDummy->SetThresholdMean(m_threshold.at(dummyIntIndex).mean);
    resultDummy->SetThresholdStdDev(m_threshold.at(dummyIntIndex).stdDev);
    resultDummy->SetNoiseMean(m_noise.at(dummyIntIndex).mean);
    resultDummy->SetNoiseStdDev(m_noise.at(dummyIntIndex).stdDev);
    
    fprintf( m_resultThreshold->GetFileHicResult(), 
	     "%d %d %d %d %d %d %f %f %f %f",
	     dummyChipIndex.boardIndex,
	     dummyChipIndex.dataReceiver,
	     dummyChipIndex.chipId,
	     resultDummy->GetCounterPixelsNoHits(),
	     resultDummy->GetCounterPixelsStuck(),
	     resultDummy->GetCounterPixelsNoThreshold(),
	     m_threshold.at(dummyIntIndex).mean,
	     m_threshold.at(dummyIntIndex).stdDev,
	     m_noise.at(dummyIntIndex).mean,
	     m_noise.at(dummyIntIndex).stdDev
	     );
  }
  
  fclose(m_resultThreshold->GetFileHicResult());
  fclose(m_resultThreshold->GetFilePixelByPixelResult());
  fclose(m_resultThreshold->GetFileStuckPixels());
  
  TPaveText* p0;
  
  TCanvas* c0 = new TCanvas();/*H*/
  c0->cd();/*H*/
  hSuperDummyA->Draw("COLZ");/*H*/
  c0->SaveAs("hSuperDummyA.pdf");/*H*/
  
  c0->cd();/*H*/
  hSuperDummyB->Draw("COLZ");/*H*/
  c0->SaveAs("hSuperDummyB.pdf");/*H*/
  
  c0->cd();/*H*/
  hSuperDummyC->Draw("COLZ");/*H*/
  c0->SaveAs("hSuperDummyC.pdf");/*H*/
  
  // Save plots.
  TFile* file_output =new TFile( Form("TThresholdAnalysis-%s.root",m_config->GetfNameSuffix()),"RECREATE");
  file_output->cd();
  
  hSuperMeanA->Write("MeanA");/*H*/
  hSuperNoiseA->Write("NoiseA");/*H*/
  hSuperRedChi2A->Write("RedChi2A");/*H*/
  hSuperStatusA->Write("StatusA");/*H*/
  hSuperDummyA->Write("hSuperDummyA");/*H*/
  hSuperNPointsVsStatusA->Write("NPointsVsStatusA");/*H*/
  hSuperNPointsVsChi2A->Write("NPointsVsChi2A");/*H*/
  
  hSuperMeanB->Write("MeanB");/*H*/
  hSuperNoiseB->Write("NoiseB");/*H*/
  hSuperRedChi2B->Write("RedChi2B");/*H*/
  hSuperStatusB->Write("StatusB");/*H*/
  hSuperDummyB->Write("hSuperDummyB");/*H*/
  hSuperNPointsVsStatusB->Write("NPointsVsStatusB");/*H*/
  hSuperNPointsVsChi2B->Write("NPointsVsChi2B");/*H*/
  
  hSuperMeanC->Write("MeanC");/*H*/
  hSuperNoiseC->Write("NoiseC");/*H*/
  hSuperRedChi2C->Write("RedChi2C");/*H*/
  hSuperStatusC->Write("StatusC");/*H*/
  hSuperDummyC->Write("hSuperDummyC");/*H*/
  hSuperNPointsVsStatusC->Write("NPointsVsStatusC");/*H*/
  hSuperNPointsVsChi2C->Write("NPointsVsChi2C");/*H*/
  
  file_output->Print();
  file_output->Close();
  
}

float TThresholdAnalysis::GetResultThreshold(int chip) {
  return m_threshold.at(chip).mean;
}


