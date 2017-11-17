#include <iostream>
#include <vector>
#include <string>
#include "TNoiseAnalysis.h"
#include "TNoiseOccupancy.h"
#include "DBHelpers.h"

TNoiseAnalysis::TNoiseAnalysis(std::deque<TScanHisto> *histoQue, 
                               TScan                  *aScan, 
                               TScanConfig            *aScanConfig,
                               std::vector <THic*>     hics, 
                               std::mutex             *aMutex, 
                               TNoiseResult           *aResult) 
: TScanAnalysis(histoQue, aScan, aScanConfig, hics, aMutex) 
{
  m_nTrig    = m_config->GetParamValue("NTRIG");
  m_noiseCut = m_nTrig / m_config->GetParamValue("NOISECUT_INV");

  if (aResult) m_result = aResult;
  else         m_result = new TNoiseResult();
  FillVariableList();
}


void TNoiseAnalysis::WriteResult() 
{
  char fName[200];
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TScanResultHic *hicResult = m_result->GetHicResult(m_hics.at(ihic)->GetDbId());
    WriteNoisyPixels (m_hics.at(ihic));
    if (m_config->GetUseDataPath()) {
      sprintf (fName, "%s/NoiseOccResult_%s.dat", hicResult->GetOutputPath().c_str(),
                                                  m_config->GetfNameSuffix());
    }
    else {
      sprintf (fName, "NoiseOccResult_%s_%s.dat", m_hics.at(ihic)->GetDbId().c_str(), 
                                                  m_config->GetfNameSuffix());
    }
    m_scan  ->WriteConditions (fName, m_hics.at(ihic));

    FILE *fp = fopen (fName, "a");
    hicResult->SetResultFile(fName);
    hicResult->WriteToFile  (fp);
    
    fclose(fp);
  }
}


void TNoiseAnalysis::WriteNoisyPixels(THic *hic)
{
  char fName[200];
  TNoiseResult    *result    = (TNoiseResult*)    m_result;
  TNoiseResultHic *hicResult = (TNoiseResultHic*) m_result->GetHicResult(hic->GetDbId());
 
  if (m_config->GetUseDataPath()) {
    sprintf (fName, "%s/NoisyPixels_%s.dat", hicResult->GetOutputPath().c_str(),
                                             m_config->GetfNameSuffix());
  }
  else {
    sprintf (fName, "NoisyPixels_%s_%s.dat", hic->GetDbId().c_str(), 
                                             m_config->GetfNameSuffix());
  }
  
  hicResult->SetNoisyFile(fName);

  FILE *fp     = fopen (fName, "w");

  for (unsigned int i = 0; i < result->m_noisyPixels.size(); i++) {
    if (!common::HitBelongsToHic(hic, result->m_noisyPixels.at(i))) continue;
    fprintf (fp, "%d %d %d %d\n", result->m_noisyPixels.at(i).chipId, 
                                  result->m_noisyPixels.at(i).region, 
                                  result->m_noisyPixels.at(i).dcol, 
                                  result->m_noisyPixels.at(i).address);
  }  
  fclose(fp);
}


void TNoiseAnalysis::Initialize()
{
  ReadChipList      ();
  CreateHicResults  ();
  m_isMasked = m_config->GetIsMasked();
  std::cout << "In noise analysis, number of hic results: " << m_result->GetNHics() << std::endl;
}


void TNoiseAnalysis::InitCounters()
{
  std::map<std::string, TScanResultHic*>::iterator it;
  for (it = m_result->GetHicResults().begin(); it != m_result->GetHicResults().end(); ++it) {
    TNoiseResultHic *result = (TNoiseResultHic*) it->second;
    result->m_backBias   = ((TNoiseOccupancy*) m_scan)->GetBackbias();
    result->m_isMasked   = m_isMasked;
    result->m_maxChipOcc = 0;
  }
}


void TNoiseAnalysis::FillVariableList() 
{
  m_variableList.insert (std::pair <const char *, TResultVariable> ("# of noisy Pixels",noisyPix));
  m_variableList.insert (std::pair <const char *, TResultVariable> ("Noise occupancy",noiseOcc));
}


void TNoiseAnalysis::AnalyseHisto (TScanHisto *histo)
{
      // TODO: check that hits from different hics / boards are considered correctly
      for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip ++) {
        TNoiseResultChip *chipResult = (TNoiseResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
        int               channel    = m_chipList.at(ichip).dataReceiver;
        int               chipId     = m_chipList.at(ichip).chipId;
        int               boardIndex = m_chipList.at(ichip).boardIndex;
        double            occ        = 0;
        double            denom      = 512. * 1024. * m_nTrig;

        if (!chipResult) {
	  std::cout << "Warning (TNoiseAnalysis): Missing chip result" << std::endl;
          continue;
	}
        for (int icol = 0; icol < 1024; icol ++) {
          for (int irow = 0; irow < 512; irow ++) {            
            // if entry > noise cut: add pixel to chipResult->AddNoisyPixel
            if ((*histo)(m_chipList.at(ichip), icol, irow) > m_noiseCut) {
	      TPixHit pixel = {boardIndex, channel, chipId, 0, icol, irow};
              chipResult->AddNoisyPixel(pixel);   // is this still needed?
              ((TNoiseResult*)m_result)->m_noisyPixels.push_back(pixel);
	    }
            occ += (*histo)(m_chipList.at(ichip), icol, irow);
	  }
	}
        // divide chipResult->m_occ by m_nTrig * 512 * 1024 and write to chipResult
        occ /= denom;
        chipResult->SetOccupancy(occ);
      }
}


void TNoiseAnalysis::Finalize() 
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic ++) {
    TNoiseResultHic *hicResult = (TNoiseResultHic*) m_result->GetHicResults().at(m_hics.at(ihic)->GetDbId());
    hicResult->m_occ    = 0; 
    hicResult->m_nNoisy = 0;
    if (m_hics.at(ihic)->GetNChips() == 0) continue;

    for (unsigned int ichip = 0; ichip < m_chipList.size(); ichip++) {
      if (! (m_hics.at(ihic)->ContainsChip(m_chipList.at(ichip)))) continue;
      TNoiseResultChip *chipResult = (TNoiseResultChip*) m_result->GetChipResult(m_chipList.at(ichip));
      hicResult->m_occ    += chipResult->m_occ;
      hicResult->m_nNoisy += chipResult->m_noisyPixels.size();
      if (chipResult->m_occ > hicResult->m_maxChipOcc) hicResult->m_maxChipOcc = chipResult->m_occ;
    }
    hicResult->m_occ /= m_hics.at(ihic)->GetNChips();
    hicResult->m_errorCounter = m_scan->GetErrorCount(m_hics.at(ihic)->GetDbId());
  }

  WriteResult();

  m_finished = true;
}


void TNoiseResultChip::WriteToFile(FILE *fp) 
{
  fprintf(fp, "Noisy pixels:    %d\n", (int) m_noisyPixels.size());
  fprintf(fp, "Noise occupancy: %e\n", m_occ);
}


float TNoiseResultChip::GetVariable(TResultVariable var) 
{
  switch (var) {
  case noiseOcc:
    return m_occ;
  case noisyPix:
    return m_noisyPixels.size();
  default:
    std::cout << "Warning, bad result type for this analysis" << std::endl;
    return 0;  
  }
}


void TNoiseResultHic::WriteToFile(FILE *fp)
{
  fprintf(fp, "HIC Result:\n\n");

  fprintf (fp, "HIC Classification: %s\n\n", WriteHicClassification());

  fprintf(fp, "Noisy pixels:    %d\n", m_nNoisy);
  fprintf(fp, "Noise occupancy: %e\n", m_occ);  

  fprintf(fp, "\nNoisy pixel file: %s\n", m_noisyFile);

  fprintf(fp, "\nNumber of chips: %d\n\n", (int)m_chipResults.size());

  std::map<int, TScanResultChip*>::iterator it;

  for (it = m_chipResults.begin(); it != m_chipResults.end(); it++) {
    fprintf(fp, "\nResults chip %d:\n\n", it->first);
    it->second->WriteToFile(fp);
  }
  
  std::cout << std::endl << "Error counts (Test feature): " << std::endl;
  std::cout << "8b10b errors:  " << m_errorCounter.n8b10b << std::endl;
  std::cout << "corrupt events " << m_errorCounter.nCorruptEvent << std::endl;
  std::cout << "timeouts:      " << m_errorCounter.nTimeout << std::endl;
}


//TODO: add information on masking (number of masked pixels?)
void TNoiseResultHic::GetParameterSuffix (std::string &suffix, std::string &file_suffix) 
{
  if (m_isMasked) {
    suffix      = "masked ";
    file_suffix = "_masked";
  }
  else {
    suffix      = "";
    file_suffix = "";
  }
  suffix      += (std::to_string((int) m_backBias) + std::string("V"));
  file_suffix += (string("_") + std::to_string((int) m_backBias) + std::string("V"));
}


void TNoiseResultHic::WriteToDB (AlpideDB *db, ActivityDB::activity &activity) 
{
  std::string suffix, file_suffix, fileName, remoteName;
  std::size_t point, slash;

  GetParameterSuffix(suffix, file_suffix);

  DbAddParameter (db, activity, string ("Noisy pixels ") + suffix, (float) m_nNoisy);
  DbAddParameter (db, activity, string ("Noise occupancy ") + suffix, (float) m_occ);
  DbAddParameter (db, activity, string ("Maximum chip occupancy ") + suffix, (float) m_maxChipOcc);

  slash      = string(m_resultFile).find_last_of("/");
  fileName   = string(m_resultFile).substr (slash +1);    // strip path
  point      = fileName.find_last_of(".");
  remoteName = fileName.substr (0, point) + file_suffix + ".dat";
  DbAddAttachment (db, activity, attachResult, string(m_resultFile), remoteName);

  slash      = string(m_noisyFile).find_last_of("/");
  fileName   = string(m_noisyFile).substr (slash +1);    // strip path
  point      = fileName.find_last_of(".");
  remoteName = fileName.substr (0, point) + file_suffix + ".dat";
  DbAddAttachment (db, activity, attachResult, string(m_resultFile), remoteName);

}
