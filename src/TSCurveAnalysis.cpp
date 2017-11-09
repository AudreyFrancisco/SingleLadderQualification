#include <iostream>
#include <vector>
#include <string>
#include "TSCurveAnalysis.h"
#include "DBHelpers.h"

#include <TMath.h>


double ErrorFunc2(double* x, double* par)
{
  //double y = par[0]+par[1]*TMath::Erf( (x[0]-par[2]) / par[3] );
  double y = par[2]*(1+TMath::Erf( (x[0]-par[0]) / (sqrt(2)*par[1]) ) );
  return y;
}



TSCurveAnalysis::TSCurveAnalysis(std::deque<TScanHisto> *histoQue, 
                                 TScan                  *aScan, 
                                 TScanConfig            *aScanConfig, 
                                 std::vector <THic*>     hics,
                                 std::mutex             *aMutex, 
                                 TSCurveResult         *aResult, 
                                 float                   resultFactor) 
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  m_nPulseInj           = m_config->GetNInj();
  m_resultFactor        = resultFactor;
  m_writeRawData        = m_config->GetParamValue("RAW_DATA");
  m_writeNoHitPixels    = false;
  m_writeNoThreshPixels = false;
  m_writeStuckPixels    = false;
  m_writeFitResults     = true;
  m_fDoFit              = true;

  m_speedy              = (m_config->GetParamValue("SPEEDY") != 0);
  if (IsThresholdScan()) {
    m_startPulseAmplitude = m_config->GetChargeStart();
    m_stopPulseAmplitude  = m_config->GetChargeStop ();
    m_stepPulseAmplitude  = m_config->GetChargeStep ();
  }
  else if (IsVCASNTuning()) {
    m_startPulseAmplitude = m_config->GetVcasnStart();
    m_stopPulseAmplitude  = m_config->GetVcasnStop ();
    m_stepPulseAmplitude  = m_config->GetVcasnStep ();   
  }
  else {
    m_startPulseAmplitude = m_config->GetIthrStart();
    m_stopPulseAmplitude  = m_config->GetIthrStop ();
    m_stepPulseAmplitude  = m_config->GetIthrStep ();
  }


  if (aResult) m_result = aResult;
  else         m_result = new TSCurveResult(); 
  FillVariableList ();
}



void TSCurveAnalysis::FillVariableList()
{
  if (IsThresholdScan()) {
    m_variableList.insert (std::pair <const char *, TResultVariable> ("Dead Pixels",   deadPix));
    m_variableList.insert (std::pair <const char *, TResultVariable> ("Pixels without threshold",   noThreshPix));
    m_variableList.insert (std::pair <const char *, TResultVariable> ("Hot Pixels",    hotPix));
    m_variableList.insert (std::pair <const char *, TResultVariable> ("av. Threshold", thresh));
    m_variableList.insert (std::pair <const char *, TResultVariable> ("Threshold RMS", threshRms));
    m_variableList.insert (std::pair <const char *, TResultVariable> ("av. Noise",     noise));
    m_variableList.insert (std::pair <const char *, TResultVariable> ("Noise RMS",     noiseRms));
  }
  else if (IsVCASNTuning()) { 
    m_variableList.insert (std::pair <const char *, TResultVariable> ("av. VCASN", vcasn));
  }
  else {                      
    m_variableList.insert (std::pair <const char *, TResultVariable> ("av. ITHR", ithr));
  }

}


bool TSCurveAnalysis::CheckPixelNoHits(TGraph* aGraph)
{

  for (int itrPoint=0; itrPoint<aGraph->GetN(); itrPoint++){
    double x =0;
    double y =0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y!=0){return false;}
  }

  return true;
}

bool TSCurveAnalysis::CheckPixelHot(TGraph* aGraph)
{
  for (int itrPoint=0; itrPoint<aGraph->GetN(); itrPoint++){
    double x =0;
    double y =0;
    aGraph->GetPoint(itrPoint, x, y);
    if (y<0.5*m_nPulseInj){return false;}
  }

  return true;
}


void TSCurveAnalysis::PrepareFiles ()
{
  char fName[200];
  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TSCurveResultChip *result = (TSCurveResultChip*) m_result->GetChipResult(m_chipList.at(i));
    if (m_writeRawData) {
      sprintf(fName, "Threshold_RawData_%s_B%d_Rcv%d_Ch%d.dat", 
                      m_config->GetfNameSuffix(), 
  	              m_chipList.at(i).boardIndex, 
                      m_chipList.at(i).dataReceiver, 
                      m_chipList.at(i).chipId);
      result->SetRawFile(fName);
      result->m_rawFP = fopen(fName, "w");
    }

    if (m_writeFitResults) {
      sprintf(fName, "Threshold_FitResults__%s_B%d_Rcv%d_Ch%d.dat", 
                     m_config->GetfNameSuffix(), 
  	             m_chipList.at(i).boardIndex, 
                     m_chipList.at(i).dataReceiver, 
                     m_chipList.at(i).chipId);
      result->SetFitFile(fName);
      result->m_fitFP = fopen(fName, "w");
    }
  }
}


void TSCurveAnalysis::Initialize() 
{
  ReadChipList     ();
  CreateHicResults ();
  PrepareFiles     ();
}


// TODO: set file names here???
void TSCurveAnalysis::InitCounters ()
{
  for (unsigned int i = 0; i < m_chipList.size(); i++) {
    TSCurveResultChip *result = (TSCurveResultChip*) m_result->GetChipResult(m_chipList.at(i));
    
    result->m_thresholdAv  = 0;
    result->m_thresholdRms = 0;
    result->m_noiseAv      = 0;
    result->m_noiseRms     = 0;
    result->m_nEntries     = 0;
    result->m_noiseSq      = 0;
    result->m_threshSq     = 0;
    result->m_nNoThresh    = 0;
    result->m_nDead        = 0;
    result->m_nHot         = 0;
  }

  std::map<std::string, TScanResultHic*>::iterator it;

  for (it = m_result->GetHicResults().begin(); it != m_result->GetHicResults().end(); ++it) {
    TSCurveResultHic *result = (TSCurveResultHic*) it->second;
    result->m_nDead         = 0;
    result->m_nNoThresh     = 0;
    result->m_minChipAv     = -1;
    result->m_maxChipAv     = 999;
    result->m_backBias      = ((TSCurveScan *) m_scan)->GetBackbias();
    result->m_nominal       = ((TSCurveScan *) m_scan)->GetNominal ();
    result->m_VCASNTuning   = IsVCASNTuning  ();
    result->m_ITHRTuning    = IsITHRTuning   ();
    result->m_thresholdScan = IsThresholdScan();
  }
}


// in some cases (VCASN tuning with back bias) the number of hits drops again after 
// reaching the plateau, which confuses the root fit. In order to avoid this, fill 
// the graph with 100% values, once the plateau has been reached.
void TSCurveAnalysis::FillGraph(TGraph *aGraph) {
  int      nMin          = m_nPulseInj - 1;
  int      count         = 0;
  int      minForPlateau = 3;
  bool     plateau       = false;
  Double_t x, y;

  for (int i = 0; i < aGraph->GetN(); i++) {
    aGraph->GetPoint(i, x, y);
    if (!plateau) {  // search for N consecutive points above nMin
      if (y > nMin) count ++;
      else          count = 0;
      if (count == minForPlateau) {
        plateau = true;
      }
    }
    else {
      aGraph->SetPoint(i, x, m_nPulseInj);
    }
  }
//        gPixel->SetPoint(gPixel->GetN(), iPulse * m_resultFactor, entries);
}


// TODO: Write Raw Data, write fit data
void TSCurveAnalysis::AnalyseHisto (TScanHisto *histo) 
{
  int row = histo->GetIndex ();
  for (unsigned int iChip = 0; iChip < m_chipList.size(); iChip ++) {
    TSCurveResultChip *chipResult = (TSCurveResultChip *) m_result->GetChipResult (m_chipList.at(iChip));
    for (int iCol = 0; iCol < common::nCols; iCol ++) {
      TGraph *gPixel = new TGraph();
      for (int iPulse = m_startPulseAmplitude; iPulse < m_stopPulseAmplitude; iPulse ++) {
        int entries = (int)(*histo) (m_chipList.at(iChip), iCol, iPulse - m_startPulseAmplitude);
        gPixel->SetPoint(gPixel->GetN(), iPulse * m_resultFactor, entries);
        if (m_writeRawData) {
          fprintf (chipResult->m_rawFP, "%d %d %d %d\n", 
                   iCol, row, iPulse, entries);
	}
      }
      if (gPixel->GetN() == 0) {delete gPixel; continue;}
      if      (CheckPixelNoHits(gPixel)) {chipResult->m_nDead++;}
      else if (CheckPixelHot   (gPixel)) {chipResult->m_nHot ++;}
      else {
        if (IsVCASNTuning()) FillGraph(gPixel);
	common::TErrFuncFitResult fitResult = DoFit (gPixel, m_speedy);
        if (fitResult.threshold == 0) {
          chipResult->m_nNoThresh++;
	}
        else {
          chipResult->m_thresholdAv += fitResult.threshold;
          chipResult->m_noiseAv     += fitResult.noise;
          chipResult->m_noiseSq     += pow(fitResult.noise, 2);
          chipResult->m_threshSq    += pow(fitResult.threshold, 2);
          chipResult->m_nEntries    ++;
	}
        if (m_writeFitResults) {
          fprintf (chipResult->m_fitFP, "%d %d %f %f %f\n", 
                   iCol, row, fitResult.threshold, fitResult.noise, fitResult.redChi2);
	}
      }
      delete gPixel;
    }
  }
}


void TSCurveAnalysis::Finalize() 
{
  TErrorCounter  errCount = ((TMaskScan*)m_scan)->GetErrorCount();
  TSCurveResult *result   = (TSCurveResult *) m_result;

  result->m_nTimeout = errCount.nTimeout;
  result->m_n8b10b   = errCount.n8b10b;
  result->m_nCorrupt = errCount.nCorruptEvent;

  for (unsigned int iChip = 0; iChip < m_chipList.size(); iChip ++) {
    TSCurveResultChip *chipResult = (TSCurveResultChip*) m_result->GetChipResult (m_chipList.at(iChip));
    chipResult->CalculateAverages();
    if (m_writeRawData)    fclose(chipResult->m_rawFP);
    if (m_writeFitResults) fclose(chipResult->m_fitFP);
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TSCurveResultHic *hicResult = (TSCurveResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip ++) {
      if (! (m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TSCurveResultChip *chipResult = (TSCurveResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
      if (chipResult->m_thresholdAv < hicResult->m_minChipAv) hicResult->m_minChipAv = chipResult->m_thresholdAv;
      if (chipResult->m_thresholdAv > hicResult->m_maxChipAv) hicResult->m_maxChipAv = chipResult->m_thresholdAv;   
    }
    if (m_hics.at(ihic)->GetHicType() == HIC_OB) {
      hicResult->m_class = GetClassificationOB(hicResult);
    }
    else {
      hicResult->m_class = GetClassificationIB(hicResult);
    }
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
  }
  WriteResult();
  m_finished = true;
}


//TODO: Add readout errors, requires dividing readout errors by hic (receiver)
//TODO: Make two cuts (red and orange)?
THicClassification TSCurveAnalysis::GetClassificationOB(TSCurveResultHic* result) {
  if (!IsThresholdScan()) return CLASS_GREEN;  // for the time being exclude class for tuning

  if (result->m_nNoThresh > m_config->GetParamValue("THRESH_MAXBAD_HIC_OB")) return CLASS_ORANGE;
  for (unsigned int ichip = 0; ichip < result->m_chipResults.size(); ichip ++) {
    int chipId = m_chipList.at(ichip).chipId & 0xf;
    TSCurveResultChip *chipResult = (TSCurveResultChip*) result->m_chipResults.at(chipId);
    if (chipResult->m_nDead + chipResult->m_nNoThresh
	> m_config->GetParamValue("THRESH_MAXBAD_CHIP_OB"))
      return CLASS_ORANGE;
    if (chipResult->m_noiseAv > (float) m_config->GetParamValue("THRESH_MAXNOISE_OB"))
      return CLASS_ORANGE;
  }
  return CLASS_GREEN;
}


THicClassification TSCurveAnalysis::GetClassificationIB(TSCurveResultHic* result) {
  if (!IsThresholdScan()) return CLASS_GREEN;  // for the time being exclude class for tuning

  if (result->m_nNoThresh > m_config->GetParamValue("THRESH_MAXBAD_HIC_IB")) return CLASS_ORANGE;
  for (unsigned int ichip = 0; ichip < result->m_chipResults.size(); ichip ++) {
    int chipId = m_chipList.at(ichip).chipId & 0xf;
    TSCurveResultChip *chipResult = (TSCurveResultChip*) result->m_chipResults.at(chipId);
    if (chipResult->m_nDead + chipResult->m_nNoThresh
	> m_config->GetParamValue("THRESH_MAXBAD_CHIP_IB"))
      return CLASS_ORANGE;
    if (chipResult->m_noiseAv > (float) m_config->GetParamValue("THRESH_MAXNOISE_IB"))
      return CLASS_ORANGE;
  }
  return CLASS_GREEN;
}


void TSCurveAnalysis::WriteResult()
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    if (IsThresholdScan())
      sprintf(fName, "ThresholdScanResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
    	                                              m_config->GetfNameSuffix());
    else if (IsVCASNTuning()) {
      sprintf(fName, "VCASNTuneResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
    	                                              m_config->GetfNameSuffix());
    }
    else {
      sprintf(fName, "ITHRTuneResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(),
    	                                              m_config->GetfNameSuffix());
    }
    m_scan->WriteConditions (fName, m_hics.at(ihic));

    FILE *fp = fopen (fName, "a");
    m_result->WriteToFileGlobal(fp);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->SetResultFile(fName);
    m_result->GetHicResult(m_hics.at(ihic)->GetDbId())->WriteToFile  (fp);
    fclose(fp);
  }	   				     
}


common::TErrFuncFitResult TSCurveAnalysis::DoSpeedyFit (TGraph *aGraph)
{
  common::TErrFuncFitResult Result;
  TGraph                    *diffGraph = new TGraph;

  ddxGraph (aGraph, diffGraph);
 
  Result.threshold = abs(meanGraph(diffGraph));
  Result.noise     = rmsGraph(diffGraph);
  Result.redChi2   = 0;

  delete diffGraph;
  return Result;
}


common::TErrFuncFitResult TSCurveAnalysis::DoRootFit (TGraph *aGraph)
{
  common::TErrFuncFitResult Result;
  TF1                      *fitfcn;

  if (IsITHRTuning()) {
    fitfcn = new TF1("fitfcn",
                     ErrorFunc2,
                     m_stopPulseAmplitude *m_resultFactor,
                     m_startPulseAmplitude*m_resultFactor,
                     4);
  }
  else {
    fitfcn = new TF1("fitfcn",
                     ErrorFunc2,
                     m_startPulseAmplitude*m_resultFactor,
                     m_stopPulseAmplitude *m_resultFactor,
                     4);
  }
  // Threshold start value
  fitfcn->SetParameter(0, FindStart(aGraph, m_resultFactor, m_nPulseInj)); 

  // Noise start value
  if (IsThresholdScan()) {
    fitfcn->SetParameter(1, 5);
  }
  else if (IsVCASNTuning()) {
    fitfcn->SetParameter(1, 1);
  }
  else {  // ITHR tuning
    fitfcn->SetParameter(1, 8);
  }
  // Amplitude start value
  fitfcn->SetParameter(2, .5*m_nPulseInj);

  aGraph->Fit("fitfcn","RQ");

  float threshold = abs(fitfcn->GetParameter(0));

  if ((threshold > abs(m_startPulseAmplitude * m_resultFactor)) && 
      (threshold < abs(m_stopPulseAmplitude  * m_resultFactor))) {
    Result.threshold = threshold;
    Result.noise     = fitfcn->GetParameter(1);
    Result.redChi2   = fitfcn->GetChisquare()/fitfcn->GetNDF();
  }
  else {  // result is outside scanned range -> usually unreliable values
    Result.threshold = 0;
    Result.noise     = 0;
    Result.redChi2   = 0;
  }

  delete fitfcn;

  return Result;
}


common::TErrFuncFitResult TSCurveAnalysis::DoFit(TGraph* aGraph, bool speedy)
{
  if (speedy) {
    return DoSpeedyFit(aGraph);
  }
  else {
    return DoRootFit(aGraph);
  }
}



////////////////////////////////////////////////////////////////////
// 
//     Methods for speedy fit 
//          - (find the mean of the derivative (erf->gaussian))
//          - returns 0 if |mean| > 500 (pixel received twice. 
//            (ignore these pixels when calculating chip mean)
//
////////////////////////////////////////////////////////////////////


double TSCurveAnalysis::meanGraph(TGraph* resultGraph) { //returns the weighted mean x value
  double  sum  = 0.0;
  double  norm = 0.0;
  double *xs   = resultGraph->GetX();
  double *ys   = resultGraph->GetY();

  for (int i = 0; i < resultGraph->GetN(); i++) {
    sum  += xs[i]*ys[i];  //xs=value, ys=weight
    norm += ys[i];
  }

  if(norm==0) { //dead pixel
    return -1;
  }
  if(abs(sum/norm) > 500) {  //outliers occur when a pixel is received twice; return 0.
    return 0;
  }
  return sum/norm;
}

double TSCurveAnalysis::rmsGraph(TGraph* resultGraph) {
  double sum  = 0.0;
  double norm = 0.0;
  double mean = meanGraph(resultGraph);

  if(mean == 0)  return 0;
  if(mean == -1) return -1;

  double *xs = resultGraph->GetX();
  double *ys = resultGraph->GetY();

  for (int i = 0; i < resultGraph->GetN(); i++) {
    sum  += ys[i]*(xs[i]-mean)*(xs[i]-mean);
    norm += ys[i];
  }

  if(sqrt(abs(sum/norm))>500) {
    return 0;
  }
  return sqrt(abs(sum/norm));
}

void TSCurveAnalysis::ddxGraph(TGraph* aGraph, TGraph* resultGraph) { //resultGraph contains the derivative of aGraph wrt x (1st order)
  //Results are at MIDPOINTS of the old graph!
  double *xs = aGraph->GetX();  //all x-coords
  double *ys = aGraph->GetY();

  for (int i = 0; i < aGraph->GetN()-1; i++) {
    //xval=avg of x1 and x2
    if(xs[i+1]==xs[i]) std::cout << "ERROR: repeated xval" << std::endl;
    resultGraph->SetPoint(resultGraph->GetN(), 0.5*(xs[i+1]+xs[i]), (ys[i+1]-ys[i])/(xs[i+1]-xs[i]));
  }
}




////////////////////////////////////////////////////////////////////
// 
//     Methods for root fit
//
////////////////////////////////////////////////////////////////////


float TSCurveAnalysis::FindStartStandard (TGraph* aGraph, int nInj) {
  float Upper = -1;
  float Lower = -1;   
  double * xs = aGraph->GetX();
  double * ys = aGraph->GetY();

  for (int i = 0; i < aGraph->GetN(); i ++) {
    if (ys[i] == nInj) {
      Upper = (float) xs[i];
      break;
    }
  }
  if (Upper == -1) return -1;
  for (int i = aGraph->GetN()-1; i > 0; i--) {
    if (ys[i] == 0) {
      Lower = (float)xs[i];
      break;
    }
  }
  if ((Lower == -1) || (Upper == -1)) {
    return -1;
  }
  if (Upper < Lower) {
    return -1;
  }
  return (Upper + Lower)/2.0;
}


float TSCurveAnalysis::FindStartInverse (TGraph* aGraph, int nInj) {
  float Upper = -1;
  float Lower = -1;
  double * xs = aGraph->GetX();
  double * ys = aGraph->GetY();

  for (int i = aGraph->GetN()-1; i>-1; i--) {
    if (ys[i] == nInj) {
      Lower = (float) xs[i];
      break;
    }
  }
  if(Lower==-1) return -1;
  for (int i = 0; i < aGraph->GetN(); i++) {
    if (ys[i] == 0) {
      Upper = (float)xs[i];
      break;
    }
  }
  if ((Lower == -1) || (Upper == -1)) {
    return -1;
  }
  if (Upper > Lower) {
    return -1;
  }
  return (Upper + Lower)/2.0;
}


float TSCurveAnalysis::FindStart (TGraph* aGraph, int resultFactor, int nInj) {
  if(resultFactor>0) {
    return FindStartStandard(aGraph, nInj);
  } else {
    return FindStartInverse(aGraph, nInj);
  }
}


void TSCurveResultChip::CalculateAverages() 
{
  if (m_nEntries > 0) {
    m_thresholdAv /= m_nEntries;
    m_noiseAv     /= m_nEntries;
    m_thresholdRms = sqrt (m_threshSq / m_nEntries - pow(m_thresholdAv, 2));
    m_noiseRms     = sqrt (m_noiseSq  / m_nEntries - pow(m_noiseAv,     2));
  }
}


void TSCurveResultChip::WriteToFile (FILE *fp)
{
  if (m_analysis->IsThresholdScan()) {
    fprintf(fp, "Pixels without hits:      %d\n",   m_nDead);
    fprintf(fp, "Pixels without threshold: %d\n",   m_nNoThresh);
    fprintf(fp, "Hot pixels:               %d\n\n", m_nHot);

    fprintf(fp, "Av. Threshold: %.1f\n", m_thresholdAv);
    fprintf(fp, "Threshold RMS: %.1f\n", m_thresholdRms);

    fprintf(fp, "Av. Noise:     %.1f\n", m_noiseAv);
    fprintf(fp, "Noise RMS:     %.1f\n", m_noiseRms);

  }
  else if (m_analysis->IsVCASNTuning()) {
    fprintf(fp, "Av. VCASN: %.1f\n", m_thresholdAv);
    fprintf(fp, "VCASN RMS: %.1f\n", m_thresholdRms);
  }
  else {                            // ITHR
    fprintf(fp, "Av. ITHR: %.1f\n", m_thresholdAv);
    fprintf(fp, "ITHR RMS: %.1f\n", m_thresholdRms);
  }

}


float TSCurveResultChip::GetVariable (TResultVariable var)
{
  switch (var) {
  case vcasn: 
    return m_thresholdAv;
  case ithr:
    return m_thresholdAv;
  case thresh:
    return m_thresholdAv;
  case threshRms:
    return m_thresholdRms;
  case noise:
    return m_noiseAv;
  case noiseRms:
    return m_noiseRms;
  case deadPix:
    return m_nDead;
  case noThreshPix:
    return m_nNoThresh;
  case hotPix:
    return m_nHot;
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;
  }
}


void TSCurveResultHic::GetParameterSuffix (std::string &suffix, std::string &file_suffix) {
  if (m_thresholdScan) {
    if (m_nominal) {
      suffix      = " threshold nominal ";
      file_suffix = "_ThreshNominal";
    }
    else {
      suffix      = " threshold tuned ";
      file_suffix = "_ThreshTuned";
    }
  }
  else if (m_VCASNTuning) {
    suffix      = " VCASN tune ";
    file_suffix = "_VCASNTune";
  }
  else if (m_ITHRTuning) {
    suffix      = " ITHR tune ";
    file_suffix = "_ITHRTune";
  }
  suffix      += (std::to_string((int) m_backBias) + std::string("V"));
  file_suffix += (string("_") + std::to_string((int) m_backBias) + std::string("V"));
}


void TSCurveResultHic::WriteToDB (AlpideDB *db, ActivityDB::activity &activity)
{
  std::string suffix, file_suffix, fileName;

  GetParameterSuffix(suffix, file_suffix);

  if (m_thresholdScan) {
    DbAddParameter (db, activity, string ("Dead pixels") + suffix,              (float) m_nNoThresh);
    DbAddParameter (db, activity, string ("Pixels without") + suffix, (float) m_nNoThresh);
  }
  DbAddParameter (db, activity, string ("Minimum chip avg") + suffix, (float) m_minChipAv);
  DbAddParameter (db, activity, string ("Maximum chip avg") + suffix, (float) m_maxChipAv);

  std::size_t point = string(m_resultFile).find_last_of(".");
  fileName = string(m_resultFile).substr (0, point) + file_suffix + ".dat";
  DbAddAttachment (db, activity, attachResult, string(m_resultFile), fileName);

}


void TSCurveResultHic::WriteToFile (FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip*>::iterator it;
  
  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResult chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }

  std::cout << std::endl << "Error counts (Test feature): " << std::endl;
  std::cout << "8b10b errors:  " << m_errorCounter.n8b10b << std::endl;
  std::cout << "corrupt events " << m_errorCounter.nCorruptEvent << std::endl;
  std::cout << "timeouts:      " << m_errorCounter.nTimeout << std::endl;
}


void TSCurveResult::WriteToFileGlobal (FILE *fp)
{
  fprintf(fp, "8b10b errors:\t%d\n",    m_n8b10b);
  fprintf(fp, "Corrupt events:\t%d\n",  m_nCorrupt);
  fprintf(fp, "Timeouts:\t%d\n",        m_nTimeout);
}


