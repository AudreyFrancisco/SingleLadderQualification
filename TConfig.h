class TConfig {
 private:
  std::vector <TBoardConfig *> fBoardConfigs;
  std::vector <TChipConfig *>  fChipConfigs;
 protected:
 public:
  TConfig (const char *fName);
  TConfig (int nBoards, int nChips);

  TChipConfig  *GetChipConfig  (int chipId);
  TBoardConfig *GetBoardConfig (int iBoard);
  void WriteToFile (const char *fName);

};
