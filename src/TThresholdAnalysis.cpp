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

void TThresholdResultChip::SetFileSummary(FILE* aFileName)
{m_fileSummary = aFileName;}  

void TThresholdResultChip::SetFilePixelNoHits(FILE* aFileName)
{m_filePixelNoHits = aFileName;}

void TThresholdResultChip::SetFilePixelStuck(FILE* aFileName)
{m_filePixelStuck = aFileName;}

void TThresholdResultChip::SetFilePixelNoThreshold(FILE* aFileName)
{m_filePixelNoThreshold = aFileName;} 

void TThresholdResultChip::SetFilePixelFitResult(FILE* aFileName)
{m_filePixelFitResult = aFileName;}

void TThresholdResultChip::SetFileRawData(FILE* aFileName)
{m_fileRawData = aFileName;} 

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

FILE* TThresholdResultChip::GetFileSummary()
{return  m_fileSummary;}

FILE* TThresholdResultChip::GetFilePixelNoHits()
{return  m_filePixelNoHits;}

FILE* TThresholdResultChip::GetFilePixelStuck()
{return m_filePixelStuck;}

FILE* TThresholdResultChip::GetFilePixelNoThreshold()
{return m_filePixelNoThreshold;}
 
FILE* TThresholdResultChip::GetFilePixelFitResult()
{return m_filePixelFitResult;}

FILE* TThresholdResultChip::GetFileRawData()
{return m_fileRawData;} 

// ================================
// ================================

TThresholdResult::TThresholdResult (): TScanResult () {;}

TThresholdResult::~TThresholdResult () {;}

// ================================
// ================================


TThresholdAnalysis::TThresholdAnalysis(std::deque<TScanHisto> *aScanHistoQue,
                                       TScan                  *aScan,
                                       TScanConfig            *aScanConfig,
                                       std::vector <THic*>     hics,
                                       std::mutex             *aMutex,
                                       float                   resultFactor)
: TScanAnalysis(aScanHistoQue, aScan, aScanConfig, hics, aMutex)  
{
  
  // It is pulse amplitude, not charge, yet.
  m_startPulseAmplitude = m_config->GetChargeStart();
  m_stopPulseAmplitude  = m_config->GetChargeStop();
  m_stepPulseAmplitude  = m_config->GetChargeStep();
  m_nPulseInj           = m_config->GetNInj();
  
  m_writeRawData        = false;
  m_writeNoHitPixels    = false;
  m_writeNoThreshPixels = false;
  m_writeStuckPixels    = false;
  m_writeFitResults     = true;
  m_fDoFit              = true;
  m_resultFactor        = resultFactor;

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

double ErrorFunc(double* x, double* par)
{
  double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  return y;
}



//Methods for a speedier fit:  find the mean of the derivative (erf->gaussian)
//  Differentiates manually; may be less accurate.
//  Returns 0 if |mean|>500, which happens when a pixel is received twice.  Ignore these when calculating the chip mean.
double meanGraph(TGraph* resultGraph) { //returns the weighted mean x value
  double sum=0.0;
  double norm=0.0;
  double * xs = resultGraph->GetX();
  double * ys = resultGraph->GetY();
  for (int i = 0; i < resultGraph->GetN(); i++) {
    sum += xs[i]*ys[i];  //xs=value, ys=weight
    norm += ys[i];
  }

  if(abs(sum/norm) > 500) {  //outliers occur when a pixel is received twice; return 0.
    /*std::cout << "WARNING: ignoring high mean of " << sum/norm << "!  (Probably received a pixel twice.)" << std::endl;
    std::cout << "  sum=" << sum << ", norm=" << norm << std::endl;
    std::cout << "  xs=[";
    for(int i=0; i<resultGraph->GetN(); i++) {
      std::cout << xs[i] << ",";
    }
    std::cout << "]" << std::endl << "  ys=[";
    for(int i=0; i<resultGraph->GetN(); i++) {
      std::cout << ys[i] << ",";
    }
    std::cout << "]" << std::endl;*/
    return 0;
  }

  return sum/norm;
}

double rmsGraph(TGraph* resultGraph) {
  double sum=0.0;
  double norm=0.0;
  double mean = meanGraph(resultGraph);
  if(mean==0) return 0;
  double * xs = resultGraph->GetX();
  double * ys = resultGraph->GetY();
  for (int i = 0; i < resultGraph->GetN(); i++) {
    sum += ys[i]*(xs[i]-mean)*(xs[i]-mean);
    norm += ys[i];
  }
  return sqrt(sum/norm);
}

void ddxGraph(TGraph* aGraph, TGraph* resultGraph) {
  //TGraph* resultGraph = new TGraph();
  double * xs = aGraph->GetX();  //all x-coords
  double * ys = aGraph->GetY();
  for (int i = 0; i < aGraph->GetN()-1; i++) {
    resultGraph->SetPoint(resultGraph->GetN(), xs[i], (ys[i+1]-ys[i])/(xs[i+1]-xs[i]));
  }
}


common::TErrFuncFitResult TThresholdAnalysis::DoFit(TGraph* aGraph, bool speedy)
//speedy=true will run the fast analysis in place of the slower fit.  Much faster,
//  but potentially less accurate.  (Shouldn't be a huge difference, though.)
//Note:  The slower version isn't working right now!
{
  common::TErrFuncFitResult fitResult_dummy;

  if(speedy) { //Looks good!
    //This should return a TGraph @ the derivative of aGraph ("d" option in copy constructor).  CHECK.
    TGraph* diffGraph = new TGraph();
    ddxGraph(aGraph, diffGraph); // = ddxGraph(aGraph);

    fitResult_dummy.threshold = abs(meanGraph(diffGraph));
    fitResult_dummy.noise     = rmsGraph(diffGraph);
    fitResult_dummy.redChi2   = 0; //not supported with this version; no fit ran
    delete diffGraph;
  }
  else {  //WARNING:  This version is still buggy!
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
    
    //common::TErrFuncFitResult fitResult_dummy;
    if(fitfcn->GetParameter(0)>0 && m_resultFactor<0) {
      std::cout << "ERROR in line 241 of TAnalogAnalysis:  Unexpected resultFactor/threshold sign!" << std::endl;
      fitResult_dummy.threshold = 1;
      return fitResult_dummy;
    }
    if(fitfcn->GetParameter(0) < 0) {
      fitResult_dummy.threshold = -1*fitfcn->GetParameter(0);  //for the ithr case
    } else {
      fitResult_dummy.threshold = fitfcn->GetParameter(0);
    }
    fitResult_dummy.noise     = fitfcn->GetParameter(1);
    fitResult_dummy.redChi2   = fitfcn->GetChisquare()/fitfcn->GetNDF();
    
    delete fitfcn; 
    
  }

  return fitResult_dummy;
}

void TThresholdAnalysis::Initialize()
{
  
  // Retrieving HistoMap from TThresholdScan, after it is initialized.
  // Creating map of TThresholdResults: 1 result for each chip.
  // Initializing TThresholdResult variables.
  // Initializing counters.
  
  while (!m_scan->IsRunning()){sleep(1);}
  
  std::cout << "Initializing " << m_analisysName << std::endl;
  
  TScanHisto histoDummy = m_scan->GetTScanHisto();
  
  std::map<int, THisto> histoMap_dummy= histoDummy.GetHistoMap();
  
  for (std::map<int, THisto>::iterator itr=histoMap_dummy.begin();
       itr!=histoMap_dummy.end(); 
       ++itr) {
    
    common::TChipIndex chipIndexDummy = common::GetChipIndex(itr->first);
    
    m_chipList.push_back(chipIndexDummy);
    
    TThresholdResultChip resultDummy;
    
    resultDummy.SetBoardIndex(chipIndexDummy.boardIndex);
    resultDummy.SetDataReceiver(chipIndexDummy.dataReceiver);
    resultDummy.SetChipId(chipIndexDummy.chipId);
    
    resultDummy.SetCounterPixelsNoHits      (0);
    resultDummy.SetCounterPixelsStuck       (0);
    resultDummy.SetCounterPixelsNoThreshold (0);
    
    resultDummy.SetThresholdMean   (0);
    resultDummy.SetThresholdStdDev (0);
    resultDummy.SetNoiseMean       (0);
    resultDummy.SetNoiseStdDev     (0);
    
    std::string fileNameDummy;
    fileNameDummy  = m_analisysName;
    fileNameDummy += "-";
    fileNameDummy += m_config->GetfNameSuffix();
    fileNameDummy += "-Summary";
    fileNameDummy  = common::GetFileName(chipIndexDummy,
  					 fileNameDummy);
    
    resultDummy.SetFileSummary(fopen(fileNameDummy.c_str(),"w"));
    if (m_writeNoHitPixels) {
      fileNameDummy  = m_analisysName;
      fileNameDummy += "-";
      fileNameDummy += m_config->GetfNameSuffix();
      fileNameDummy += "-PixelNoHits";
      fileNameDummy  = common::GetFileName(chipIndexDummy,
  					  fileNameDummy);
    
      resultDummy.SetFilePixelNoHits(fopen(fileNameDummy.c_str(),"w"));
    }
    
    if (m_writeStuckPixels) {
      fileNameDummy  = m_analisysName;
      fileNameDummy += "-";
      fileNameDummy += m_config->GetfNameSuffix();
      fileNameDummy += "-PixelStuck";
      fileNameDummy  = common::GetFileName(chipIndexDummy,
  					  fileNameDummy);
    
      resultDummy.SetFilePixelStuck(fopen(fileNameDummy.c_str(),"w"));
    }
    
    if (m_writeNoThreshPixels) {
      fileNameDummy  = m_analisysName;
      fileNameDummy += "-";
      fileNameDummy += m_config->GetfNameSuffix();
      fileNameDummy += "-PixelNoThreshold";
      fileNameDummy  = common::GetFileName(chipIndexDummy,
  					  fileNameDummy);
    
      resultDummy.SetFilePixelNoThreshold(fopen(fileNameDummy.c_str(),"w"));
    }
    
    if (m_writeFitResults) {
      fileNameDummy  = m_analisysName;
      fileNameDummy += "-";
      fileNameDummy += m_config->GetfNameSuffix();
      fileNameDummy += "-PixelFitResult";
      fileNameDummy  = common::GetFileName(chipIndexDummy,
  					  fileNameDummy);
    
      resultDummy.SetFilePixelFitResult(fopen(fileNameDummy.c_str(),"w"));
    }
    
    if (m_writeRawData) {
      fileNameDummy  = m_analisysName;
      fileNameDummy += "-";
      fileNameDummy += m_config->GetfNameSuffix();
      fileNameDummy += "-PixelRawData";
      fileNameDummy  = common::GetFileName(chipIndexDummy,
  					 fileNameDummy);
    
      resultDummy.SetFileRawData(fopen(fileNameDummy.c_str(),"w"));
    }

    std::pair<int,TThresholdResultChip> pairDummy;
    pairDummy = std::make_pair(itr->first,resultDummy);
    m_resultChip.insert(pairDummy);
    
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
	  
   	  if(m_writeRawData){
   	    int intIndexDummy = common::GetChipIntIndex(m_chipList.at(iChip)); 
	    
	    TThresholdResultChip resultDummy = m_resultChip.at(intIndexDummy);

   	    fprintf(m_resultChip.at(intIndexDummy).GetFileRawData(),
	    	    "%d %d %d %d\n", 
   	    	    iCol,row,iPulse,entries);
   	  }
	  
   	} // end loop over iPulse.
	
   	if (gDummy->GetN()==0){delete gDummy;continue;}
	
	int intIndexDummy = common::GetChipIntIndex(m_chipList.at(iChip));
	
	TThresholdResultChip resultDummy = m_resultChip.at(intIndexDummy);
	
	bool fPixelNoHits= CheckPixelNoHits(gDummy);
	bool fPixelStuck = CheckPixelStuck (gDummy);
	
	if (fPixelNoHits){
	  if (m_writeNoHitPixels) {
            fprintf(m_resultChip.at(intIndexDummy).GetFilePixelNoHits(), 
   	    	    "%d %d\n", 
		    iCol,row);
	  }
	  
	  m_resultChip.at(intIndexDummy).SetCounterPixelsNoHits(m_resultChip.at(intIndexDummy).GetCounterPixelsNoHits()+1);
	  
	} else if (fPixelStuck){
	  if (m_writeStuckPixels) {
            fprintf(m_resultChip.at(intIndexDummy).GetFilePixelStuck(), 
	 	    "%d %d\n", 
		    iCol,row);
	  }
	  
	  m_resultChip.at(intIndexDummy).SetCounterPixelsStuck(m_resultChip.at(intIndexDummy).GetCounterPixelsStuck()+1);
	  
	} else if (m_fDoFit){
	  
	  // MB - NEED TO SELECT GOOD FIT.
	  
	  common::TErrFuncFitResult fitResult;
	  fitResult=DoFit(gDummy, true); //testing speedy version for now!

	  if (m_writeFitResults) {
  	    fprintf(m_resultChip.at(intIndexDummy).GetFilePixelFitResult(), 
   	    	    "%d %d %f %f %f\n", 
   	    	    iCol,row,
   	    	    fitResult.threshold,
   	    	    fitResult.noise,
   	 	    fitResult.redChi2);
	  }
	  if (fitResult.threshold!=0) { //if no error/dead pixel
	    m_threshold.at(intIndexDummy).sum+=fitResult.threshold; //row;
	    m_threshold.at(intIndexDummy).sum2+=pow(fitResult.threshold,2); //row*row
	    m_threshold.at(intIndexDummy).entries+=1;
	    
	    m_noise.at(intIndexDummy).sum+=fitResult.noise; //row
	    m_noise.at(intIndexDummy).sum2+=pow(fitResult.noise,2); //row*row
	    m_noise.at(intIndexDummy).entries+=1;
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
  if ( m_resultChip.size()!=m_threshold.size() || 
       m_resultChip.size()!=m_noise.size()){ 
    std::cout<< "ERROR in " 
   	     << m_analisysName  
   	     << "!!!"
   	     <<std::endl; exit(EXIT_FAILURE);;
  }
  
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
  
  for (std::map<int,TThresholdResultChip>::iterator itr=m_resultChip.begin();
       itr!=m_resultChip.end(); 
       ++itr) {
    
    itr->second.SetThresholdMean(m_threshold.at(itr->first).mean);
    itr->second.SetThresholdStdDev(m_threshold.at(itr->first).stdDev);
    itr->second.SetNoiseMean(m_noise.at(itr->first).mean);
    itr->second.SetNoiseStdDev(m_noise.at(itr->first).stdDev);
    
    fprintf(itr->second.GetFileSummary(), 
       	    "Threshold mean: %f \n", 
       	    itr->second.GetThresholdMean() );
    
    fprintf(itr->second.GetFileSummary(), 
     	    "Threshold stdDev: %f \n", 
     	    itr->second.GetThresholdStdDev());
    
    fprintf(itr->second.GetFileSummary(), 
     	    "Noise mean: %f \n", 
     	    itr->second.GetNoiseMean());
    
    fprintf(itr->second.GetFileSummary(), 
     	    "Noise stdDev: %f \n", 
     	    itr->second.GetNoiseStdDev());
    
    fprintf(itr->second.GetFileSummary(), 
	    "counterPixelsNoHits: %d \n", 
	    itr->second.GetCounterPixelsNoHits());
    
    fprintf(itr->second.GetFileSummary(), 
	    "counterPixelsNoThreshold: %d \n", 
	    itr->second.GetCounterPixelsNoThreshold());
    
    fprintf(itr->second.GetFileSummary(), 
	    "counterPixelsStuck: %d \n", 
	    itr->second.GetCounterPixelsStuck()); 
    
    fclose(itr->second.GetFileSummary());
    if (m_writeNoHitPixels)    fclose(itr->second.GetFilePixelNoHits());
    if (m_writeStuckPixels)    fclose(itr->second.GetFilePixelStuck());
    if (m_writeNoThreshPixels) fclose(itr->second.GetFilePixelNoThreshold());
    if (m_writeFitResults)     fclose(itr->second.GetFilePixelFitResult());
    if (m_writeRawData)        fclose(itr->second.GetFileRawData()); 
    
    m_result->AddChipResult(itr->first,
			    &(itr->second));
    
  }
}

float TThresholdAnalysis::GetResultThreshold(int chip) {
  return m_threshold.at(chip).mean;
}


