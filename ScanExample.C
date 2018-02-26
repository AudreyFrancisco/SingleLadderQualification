main()
{

  // initialise setup
  // 1) Create config object (here: from config file)
  // 2) Create readout board and chips
  // 3) Pass pointer to readout board to chip objects
  // 4) Pass information on ChipId / ControlInterface / Receiver to readout board

  TConfig *      myConfig       = new TConfig("Config.cfg");
  TReadoutBoard *myReadoutBoard = new TMOSAIC(myConfig);

  std::vector<TAlpide *> Chips;

  for (int ichip = 0; ichip < myConfig->GetNChips(); ichip++) {
    Chips->push_back(new TAlpide(myConfig, ichip)); // create ichipth chip out of the config
    Chips[ichip]->SetReadoutBoard(
        myReadoutBoard); // set pointer to readout board in the chip object
    myReadoutBoard->AddChip(
        myConfig->GetChipId(ichip),           // add ChipId / ControlInterface / Receiver settings
        myConfig->GetControlInterface(ichip), // to readout board
        myConfig->GetReceiver(ichip));
  }

  // configure chips
  // a) write registers directly
  for (std::vector<TAlpide *>::iterator ichip = Chips.begin(); ichip != Chips.end(); ichip++) {
    ichip->WriteRegister(VCASN, 57);
    ichip->WriteRegister(ITHR, 51);
    ichip->WriteRegister(VPULSEH, 170);
    //...
  }

  // b) use predefined methods in TAlpideConfig class
  for (std::vector<TAlpide *>::iterator ichip = Chips.begin(); ichip != Chips.end(); ichip++) {
    TAlpideConfig::SettingsForBackBias(ichip, 3); // apply DAC settings for 3 V back bias
                                                  //...
  }

  // do the scan

  int  NBytes;
  char Buffer[];

  myReadoutBoard->Trigger(myConfig->GetNTriggers(), OPCODE_PULSE);

  // Format of data still to be defined (single events, full buffer ...
  myReadoutBoard->ReadEventData(NBytes, Buffer);

  TAlpideDecoder::DecodeEvent(Buffer, std::vector<TPixHit> Hits);
  // ...

  // delete chips + readout board
}
