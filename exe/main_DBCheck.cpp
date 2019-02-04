#include <AlpideDB.h>
#include <AlpideDBEndPoints.h>
#include <DBHelpers.h>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <vector>


static const std::list<std::string> kHicActivities = {
    "OB HIC Assembly",       "OB-HIC Impedance Test", "OB HIC Qualification Test",
    "OB HIC Endurance Test", "OB HIC Shipment",       "OB HIC Reception",
    "OB HIC Reception Test", "OB-HIC TAB cut",        "OB HIC Fast Power Test"};

static const std::list<std::string> kHicActivitiesOL = {"OL HS Qualification Test",
                                                        "OL Stave Qualification Test"};

static const std::list<std::string> kHicActivitiesML = {"ML HS Qualification Test",
                                                        "ML Stave Qualification Test"};

static const std::list<std::string> kBareStaveActivitiesOL = {
    "OL-Stave w/o PB assembly", "OL-Stave final metrology",
    "OL-PBs and OL-BBs soldering to OL-Stave"};

static const std::list<std::string> kBareStaveActivitiesML = {
    "ML-Stave w/o PB assembly", "ML-Stave final metrology",
    "ML-PBs and ML-BBs soldering to ML-Stave"};

static const std::list<std::string> kStaveActivitiesOL = {
    "OL Stave Qualification Test", "OL-PBs and OL-BBs soldering to OL-Stave",
    "OL-PBs and OL-BBs folding for OL-Stave", "OL-Stave storage and shipment"};

static const std::list<std::string> kStaveActivitiesML = {
    "ML Stave Qualification Test", "ML-PBs and ML-BBs soldering to ML-Stave",
    "ML-PBs and ML-BBs folding for ML-Stave", "ML-Stave storage and shipment"};

static const std::list<std::string> kHalfStaveActivitiesOL = {
    "OL-HS soldering/desoldering", "U-arm gluing on OL-HS", "OL-Stave w/o PB assembly",
    "OL HS Qualification Test", "OL Stave Qualification Test"};

static const std::list<std::string> kHalfStaveActivitiesML = {
    "ML-HS soldering/desoldering", "U-arm gluing on ML-HS", "ML-Stave w/o PB assembly",
    "ML HS Qualification Test", "ML Stave Qualification Test"};

const int STAVECOMP     = 5;
const int BARESTAVECOMP = 3;
int       HALFSTAVECOMP;
int       NHICS;

int g_staveType;
int g_bareStaveType;
int g_hsUpperType;
int g_hsLowerType;
int g_hicType;

string g_hicLocation;
string g_staveLocation;

FILE *g_fpDB;
FILE *g_fpEos;

std::map<string, int> g_missing;

std::string getUserName()
{
  std::array<char, 128>                    buffer;
  std::string                              result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(
      popen("klist | grep 'Default principal' | cut -d' ' -f3 | cut -d'@' -f1", "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  result.pop_back();
  return result;
}


void printUsage()
{
  std::cout << std::endl << "Usage:" << std::endl;
  std::cout << "   test_DBCheck staveName" << std::endl << std::endl;
}


void initTypeIds(AlpideDB *db, bool middleLayer)
{
  if (middleLayer) {
    g_staveType     = DbGetComponentTypeId(db, "Middle Layer Stave");
    g_bareStaveType = DbGetComponentTypeId(db, "Middle Layer Stave w/o PB&BB");
    g_hsUpperType   = DbGetComponentTypeId(db, "Middle Layer Half-Stave Upper");
    g_hsLowerType   = DbGetComponentTypeId(db, "Middle Layer Half-Stave Lower");
    HALFSTAVECOMP   = 5;
    NHICS           = 8;
  }
  else {
    g_staveType     = DbGetComponentTypeId(db, "Outer Layer Stave");
    g_bareStaveType = DbGetComponentTypeId(db, "Outer Layer Stave w/o PB&BB");
    g_hsUpperType   = DbGetComponentTypeId(db, "Outer Layer Half-Stave Upper");
    g_hsLowerType   = DbGetComponentTypeId(db, "Outer Layer Half-Stave Lower");
    HALFSTAVECOMP   = 8;
    NHICS           = 14;
  }
  g_hicType = DbGetComponentTypeId(db, "Outer Barrel HIC Module");
}


// checks if a given activity type appears in the component history
bool FindActivity(vector<ComponentDB::compActivity> history, string activityType, int &activityId)
{
  for (unsigned int i = 0; i < history.size(); i++) {
    if (history.at(i).Typename == activityType) {
      activityId = history.at(i).ID;
      return true;
    }
  }
  return false;
}


void addToMissing(string missingName)
{
  std::map<string, int>::iterator missingAct = g_missing.find(missingName);
  if (missingAct != g_missing.end()) {
    missingAct->second++;
  }
  else {
    g_missing.insert(pair<string, int>(missingName, 1));
  }
}


// check history for single activity only
bool DbCheckHistorySingle(AlpideDB *db, int compId, string compName, string activityType,
                          string &location)
{
  // exclude non-existent relations
  if (((compName.find("L") != string::npos) && (activityType.find("Upper") != string::npos)) ||
      ((compName.find("R") != string::npos) && (activityType.find("Lower") != string::npos)))
    return true;


  ComponentDB *componentDB = new ComponentDB(db);
  ActivityDB * activityDB  = new ActivityDB(db);

  vector<ComponentDB::compActivity> history;
  int                               ID;

  componentDB->GetComponentActivities(compId, &history);

  bool result = FindActivity(history, activityType, ID);
  if (!result) {
    std::cout << "Component " << compName << " is missing activity of type " << activityType
              << std::endl;
    if (g_fpDB)
      fprintf(g_fpDB, "Component %s is missing activity of type %s\n", compName.data(),
              activityType.data());
  }
  ActivityDB::activityLong actLong;
  activityDB->Read(ID, &actLong);
  location = actLong.Location.Name;
  return result;
}


// checks the component history for a list of activity types
// if an activity type is not found an error message is printed
// returns the number of missing activities
int DbCheckHistory(AlpideDB *db, int compId, string compName, list<string> activityTypes,
                   string &location)
{
  ComponentDB *                     componentDB = new ComponentDB(db);
  ActivityDB *                      activityDB  = new ActivityDB(db);
  ActivityDB::activityLong          actLong;
  vector<ComponentDB::compActivity> history;
  int                               ID;

  int missing = 0;
  componentDB->GetComponentActivities(compId, &history);


  for (list<string>::iterator it = activityTypes.begin(); it != activityTypes.end(); it++) {
    if (!FindActivity(history, *it, ID)) {
      std::cout << "Component " << compName << " is missing activity of type " << *it << std::endl;
      if (g_fpDB)
        fprintf(g_fpDB, "Component %s is missing activity of type %s\n", compName.data(),
                (*it).data());
      addToMissing(*it);
      missing++;
    }
    if (it == activityTypes.begin()) activityDB->Read(ID, &actLong);
  }

  location = actLong.Location.Name;
  return missing;
}


bool checkEos(string activityType, string location, string component)
{
  string retestPath, path, locDir, test;
  string basePath("/eos/project/a/alice-its/HicTests");
  GetServiceAccount(location, locDir);
  test = GetTestDirName(GetTestType(activityType));

  path = basePath + "/" + test + locDir + "/" + component;


  string username = getUserName();
  string command  = "ssh -K -o GSSAPITrustDNS=yes ";

  if (std::system(string(command + username + "@lxplus.cern.ch '[ -d " + path + " ]'").data()) ==
      0) {
    return true;
  }


  for (int retest = 1; retest < 20; retest++) {
    retestPath = path + "_Retest_" + to_string(retest);
    if (std::system(
            string(command + username + "@lxplus.cern.ch '[ -d " + retestPath + " ]'").data()) ==
        0) {
      return true;
    }
  }

  std::cout << "Unable to find eos path " << path << std::endl;

  if (g_fpEos) {
    fprintf(g_fpEos, "Component %s is missing eos data path %s\n", component.data(), path.data());
  }

  return false;
}


bool fillStaveComposition(AlpideDB *db, TChild stave, TChild &bareStave)
{
  std::vector<TChild> staveChildren;
  int                 nChildren = DbGetListOfChildren(db, stave.Id, staveChildren);

  if (nChildren != STAVECOMP) {
    std::cout << "Error, found only " << nChildren << " child(ren) for stave " << stave.Name
              << std::endl;
  }

  bool found = false;
  for (unsigned int i = 0; i < staveChildren.size(); i++) {
    if (staveChildren.at(i).Type == g_bareStaveType) {
      bareStave = staveChildren.at(i);
      found     = true;
    }
  }

  if (!found) {
    std::cout << "Error (FATAL), Did not find bare stave for stave " << stave.Name << std::endl;
    return false;
  }
  return true;
}

bool fillBareStaveComposition(AlpideDB *db, TChild bareStave, std::vector<TChild> &halfStaves)
{
  std::vector<TChild> bareStaveChildren;
  int                 nChildren = DbGetListOfChildren(db, bareStave.Id, bareStaveChildren);

  if (nChildren != BARESTAVECOMP) {
    std::cout << "Error, found only " << nChildren << " child(ren) for stave " << bareStave.Name
              << std::endl;
  }

  for (unsigned int i = 0; i < bareStaveChildren.size(); i++) {
    if ((bareStaveChildren.at(i).Type == g_hsUpperType) ||
        (bareStaveChildren.at(i).Type == g_hsLowerType)) {
      halfStaves.push_back(bareStaveChildren.at(i));
    }
  }

  if (halfStaves.size() != 2) {
    std::cout << "Error (FATAL), found wrong number of half staves: " << halfStaves.size()
              << std::endl;
    return false;
  }
  return true;
}


bool fillHalfStaveComposition(AlpideDB *db, TChild halfStave, std::vector<TChild> &hics)
{
  std::vector<TChild> halfStaveChildren;
  int                 nChildren = DbGetListOfChildren(db, halfStave.Id, halfStaveChildren);

  if (nChildren != HALFSTAVECOMP) {
    std::cout << "Error, found only " << nChildren << " child(ren) for half stave "
              << halfStave.Name << std::endl;
  }

  for (unsigned int i = 0; i < halfStaveChildren.size(); i++) {
    if (halfStaveChildren.at(i).Type == g_hicType) {
      hics.push_back(halfStaveChildren.at(i));
    }
  }

  return true;
}


bool fillComposition(AlpideDB *db, TChild &stave, TChild &bareStave,
                     std::vector<TChild> &halfStaves, std::vector<TChild> &hics)
{

  halfStaves.clear();
  hics.clear();

  stave.Type = g_staveType;
  stave.Id   = DbGetComponentId(db, stave.Type, stave.Name);
  if (stave.Id < 0) {
    std::cout << "Impossible to find stave " << stave.Name << ", exiting." << std::endl;
    return false;
  }

  if (!fillStaveComposition(db, stave, bareStave)) return false;
  if (!fillBareStaveComposition(db, bareStave, halfStaves)) return false;
  for (unsigned int i = 0; i < halfStaves.size(); i++) {
    if (!fillHalfStaveComposition(db, halfStaves.at(i), hics)) return false;
  }
  return true;
}


bool checkHicHistory(AlpideDB *db, TChild hic, bool middleLayer, int &missing)
{
  int    old_missing = missing;
  string location;

  missing += DbCheckHistory(db, hic.Id, hic.Name, kHicActivities, location);
  g_hicLocation = location;
  if (middleLayer) {
    missing += DbCheckHistory(db, hic.Id, hic.Name, kHicActivitiesML, location);
    if ((!DbCheckHistorySingle(db, hic.Id, hic.Name, "ML-HS-Upper assembly", location)) &&
        (!DbCheckHistorySingle(db, hic.Id, hic.Name, "ML-HS-Lower assembly", location))) {
      missing++;
      addToMissing("ML-HS assembly");
    }
  }
  else {
    missing += DbCheckHistory(db, hic.Id, hic.Name, kHicActivitiesOL, location);
    if ((!DbCheckHistorySingle(db, hic.Id, hic.Name, "OL-HS-Upper assembly", location)) &&
        (!DbCheckHistorySingle(db, hic.Id, hic.Name, "OL-HS-Lower assembly", location))) {
      missing++;
      addToMissing("OL-HS assembly");
    }
  }


  if (missing - old_missing) return false;
  return true;
}


bool checkHalfStaveHistory(AlpideDB *db, TChild halfStave, bool middleLayer, int &missing)
{
  // Note: this checks only if there is at least 1 of each activities, however the
  // testing activities should appear 7 times (once per HIC)
  // On the other hand, this is tested also for the HICs so the only nono-considered case is
  // all testing activities being attached to the HICs, but only part of them to the half-stave

  int    old_missing = missing;
  string location;

  if (middleLayer) {
    missing += DbCheckHistory(db, halfStave.Id, halfStave.Name, kHalfStaveActivitiesML, location);
    if ((!DbCheckHistorySingle(db, halfStave.Id, halfStave.Name, "ML-HS-Upper assembly",
                               location)) &&
        (!DbCheckHistorySingle(db, halfStave.Id, halfStave.Name, "ML-HS-Lower assembly",
                               location))) {
      missing++;
      addToMissing("ML-HS assembly");
    }
  }
  else {
    missing += DbCheckHistory(db, halfStave.Id, halfStave.Name, kHalfStaveActivitiesOL, location);
    if ((!DbCheckHistorySingle(db, halfStave.Id, halfStave.Name, "OL-HS-Upper assembly",
                               location)) &&
        (!DbCheckHistorySingle(db, halfStave.Id, halfStave.Name, "OL-HS-Lower assembly",
                               location))) {
      missing++;
      addToMissing("OL-HS assembly");
    }
  }

  if (missing - old_missing) return false;
  return true;
}


bool checkBareStaveHistory(AlpideDB *db, TChild bareStave, bool middleLayer, int &missing)
{
  int    old_missing = missing;
  string location;

  if (middleLayer) {
    missing += DbCheckHistory(db, bareStave.Id, bareStave.Name, kBareStaveActivitiesML, location);
  }
  else {
    missing += DbCheckHistory(db, bareStave.Id, bareStave.Name, kBareStaveActivitiesOL, location);
  }

  if (missing - old_missing) return false;
  return true;
}


bool checkStaveHistory(AlpideDB *db, TChild stave, bool middleLayer, int &missing)
{
  int    old_missing = missing;
  string location;

  if (middleLayer) {
    missing += DbCheckHistory(db, stave.Id, stave.Name, kStaveActivitiesML, location);
  }
  else {
    missing += DbCheckHistory(db, stave.Id, stave.Name, kStaveActivitiesOL, location);
  }
  g_staveLocation = location;

  if (missing - old_missing) return false;
  return true;
}


int main(int argc, char **argv)
{
  std::vector<TChild> halfStaves;
  std::vector<TChild> hics;
  TChild              stave, bareStave;
  bool                middleLayer   = false;
  int                 missing       = 0;
  int                 missingEos[6] = {0, 0, 0, 0, 0, 0};

  g_fpDB  = nullptr;
  g_fpEos = nullptr;

  if (argc != 2) {
    printUsage();
    return -1;
  }
  stave.Name = argv[1];

  if (stave.Name.find("OL") != string::npos) {
    middleLayer = false;
  }
  else if (stave.Name.find("ML") != string::npos) {
    middleLayer = true;
  }
  else {
    printUsage();
    return -1;
  }

  std::cout << "Checking DB for stave " << stave.Name << std::endl;

  AlpideDB *db = new AlpideDB(false);
  if (!db) {
    std::cout << "Error opening production DB" << std::endl;
    return -1;
  }

  initTypeIds(db, middleLayer);
  if (!fillComposition(db, stave, bareStave, halfStaves, hics)) return -1;

  if (hics.size() != (unsigned int)NHICS) {
    std::cout << "Error (FATAL), found wrong number of hics: " << hics.size() << std::endl;
    return -1;
  }

  g_missing.clear();

  char fName[50];

  sprintf(fName, "%s_missingActivities.txt", stave.Name.data());
  g_fpDB = fopen(fName, "w");
  sprintf(fName, "%s_missingData.txt", stave.Name.data());
  g_fpEos = fopen(fName, "w");

  int incompleteHics       = 0;
  int incompleteHalfstaves = 0;

  for (unsigned int i = 0; i < hics.size(); i++) {
    if (!checkHicHistory(db, hics.at(i), middleLayer, missing)) incompleteHics++;
  }

  for (unsigned int i = 0; i < halfStaves.size(); i++) {
    if (!checkHalfStaveHistory(db, halfStaves.at(i), middleLayer, missing)) incompleteHalfstaves++;
  }

  checkBareStaveHistory(db, bareStave, middleLayer, missing);
  checkStaveHistory(db, stave, middleLayer, missing);

  for (unsigned int i = 0; i < hics.size(); i++) {
    if (!checkEos("OB HIC Qualification Test", g_hicLocation, hics.at(i).Name)) missingEos[0]++;
    if (!checkEos("OB HIC Endurance Test", g_hicLocation, hics.at(i).Name)) missingEos[1]++;
    if (!checkEos("OB HIC Reception Test", g_staveLocation, hics.at(i).Name)) missingEos[2]++;
    if (!checkEos("OB HIC Fast Power Test", g_staveLocation, hics.at(i).Name)) missingEos[3]++;
    if (middleLayer) {
      if (!checkEos("ML HS Qualification Test", g_staveLocation, hics.at(i).Name)) missingEos[4]++;
      if (!checkEos("ML Stave Qualification Test", g_staveLocation, hics.at(i).Name))
        missingEos[5]++;
    }
    else {
      if (!checkEos("OL HS Qualification Test", g_staveLocation, hics.at(i).Name)) missingEos[4]++;
      if (!checkEos("OL Stave Qualification Test", g_staveLocation, hics.at(i).Name))
        missingEos[5]++;
    }
  }

  std::cout << std::endl << "Missing activities (total " << missing << "): " << std::endl;
  for (map<string, int>::iterator it = g_missing.begin(); it != g_missing.end(); it++) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
  std::cout << "HICs with incomplete history: " << incompleteHics << std::endl;

  std::cout << std::endl << "Missing eos directories: " << std::endl;
  std::cout << "HIC Qualification test: " << missingEos[0] << std::endl;
  std::cout << "HIC Endurance test: " << missingEos[1] << std::endl;
  std::cout << "HIC Reception test: " << missingEos[2] << std::endl;
  std::cout << "HIC Fast-Power test: " << missingEos[3] << std::endl;
  std::cout << "Half-Stave Qualification test: " << missingEos[4] << std::endl;
  std::cout << "Stave Qualification test: " << missingEos[5] << std::endl;

  fclose(g_fpDB);
  fclose(g_fpEos);
  return 0;
}
