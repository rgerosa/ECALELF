#include "../interface/EnergyScaleCorrection_class.h"
//#define DEBUG

// for exit(0)
#include <stdlib.h>
// for setw
#include <iomanip>

//#define DEBUG
//#define OldCategoryDefinition
//EnergyScaleCorrection_class::EnergyScaleCorrection_class(TString correctionFileName, bool isHggCat_):
EnergyScaleCorrection_class::EnergyScaleCorrection_class(TString correctionFileName, TString correctionType_,
							 TString smearingFileName, TString smearingType_):
  noCorrections(true), noSmearings(true), 
  correctionType(correctionType_), smearingType(smearingType_),
  runMin_map(), rgen_(NULL)
{
  if(smearingFileName.Sizeof()>1) noSmearings=false;
  else noSmearings=true;

  if(correctionType.Sizeof()<=1 && smearingType.Sizeof()>1) correctionType=smearingType;
#ifdef DEBUG
  std::cout << "[DEBUG] Using correctionType: " << correctionType << std::endl;
#endif

  if(correctionType.CompareTo("noCalib")==0){
    noCorrections=true;
  } else{
    if(correctionType.Contains("Run",TString::kIgnoreCase) 
       && ! correctionFileName.Sizeof()>1){
      std::cerr << "[ERROR] Correction type: " << correctionType << " but not correction file give" << std::endl;
      std::cerr << "        Correction file: " << correctionFileName << std::endl;
      exit(1);
    }
    noCorrections=false;
  }

  
  if(correctionType.Contains("Hgg",TString::kIgnoreCase) ||
     smearingType.Contains("Hgg",TString::kIgnoreCase)){
    isHggCat=true;
    //noCorrections=false;
  }

  if(!noCorrections) ReadFromFile(correctionFileName);
  runCorrection_itr=runMin_map.begin();

  if(!noSmearings) ReadSmearingFromFile(smearingFileName);
  rgen_=new TRandom3(0); // inizializzo il generatore con seed impostato sull'ora

  return;
}

EnergyScaleCorrection_class::~EnergyScaleCorrection_class(void){
  return;
}

TString EnergyScaleCorrection_class::GetElectronCategory(bool isEBEle, double R9Ele, double etaSCEle){

  TString category;

  if(isEBEle) category="EB";
  else category="EE";
  
  if(isHggCat){
    if(correctionType.Contains("Hgg",TString::kIgnoreCase)){
      if(correctionType.Contains("eta",TString::kIgnoreCase)){
	
	//    if(correctionType.CompareTo("Hgg_eta_runNumber")==0){
	if(isEBEle){
	  if (fabs(etaSCEle)>1.) category+="-absEta_1_1.4442";
	  else category+="-absEta_0_1";
	} else {
	  if (fabs(etaSCEle) > 2) category+="-absEta_2_2.5";
	  else category+="-absEta_1.566_2";
	}
      }
      if(correctionType.Contains("residual",TString::kIgnoreCase)||correctionType.Contains("R9",TString::kIgnoreCase)){
	if(R9Ele>0.94) category+="-gold";
	else category+="-bad";
	// 	if(R9Ele>0.94) category+="Gold";
	// 	else category+="Bad";
      }

    }
  }
#ifdef DEBUG
  std::cout << "[DEBUG] category: " << category << std::endl;
#endif  
  return category;
  
}

std::map< int, correction_t>::const_iterator EnergyScaleCorrection_class::FindRunCorrection_itr(int runNumber){
  std::map< int, correction_t>::const_iterator itr=runMin_map.begin();

  while (runNumber >= itr->first && itr!=runMin_map.end()){
#ifdef DEBUG
    std::cout << "runNumber - itr->first: " << runNumber << " - " << itr->first << std::endl;
#endif
    itr++;
  }
  itr--;
#ifdef DEBUG
  std::cout << "[DEBUG] runNumber - [itr->first: itr->second.runMax] " << runNumber << " - [" << itr->first << ":" << itr->second.runMax << "]" << std::endl;
#endif

  if(itr->second.runMax < runNumber){
    //    std::cerr << "[WARNING] runNumber > runs in recalib file: " << runNumber << std::endl;
    return runMin_map.end();
  }
  //  if(runNumber <= itr->second.runMax) // c'e' un altro errore ma non e' possibile
  return itr;
}

  
float EnergyScaleCorrection_class::getScaleOffset(int runNumber, bool isEBEle, double R9Ele, double etaSCEle){
  if(noCorrections) return 1;
  if(runMin_map.empty()){
    std::cerr << "[ERROR] runMin_map empty and calibration required" << std::endl;
    //std::cerr << "[ERROR] turning into noCorrection mode" << std::endl;
    noCorrections=true;
    exit(1);
    //return 1;
  }


  if(!(runNumber >= runCorrection_itr->first && runNumber <= runCorrection_itr->second.runMax)){
#ifdef DEBUG
    std::cout << "[DEBUG] starting FindRunCorrection: " << runNumber << "\t" << runCorrection_itr->first << "\t" << runCorrection_itr->second.runMax << std::endl;
#endif    
    runCorrection_itr=FindRunCorrection_itr(runNumber);
    if (runCorrection_itr==runMin_map.end()){
      runCorrection_itr=runMin_map.begin(); // lo reinizializzo per i prossimi run
      return 0; // se per esempio il runNumber non e' definito nel recalib file
    }
  }

  //  if (runMin_map.count(runNumber)==1){
#ifdef DDEBUG
  std::cout << "[DEBUG] " << runNumber << "\t" << runCorrection_itr->first << "\t" << runCorrection_itr->second.runMax << std::endl;
#endif
    
    const correction_map_t *correction_map_p = &(runCorrection_itr->second.correction_map);
    
#ifdef HggCatTry
  // questo se voglio provare a cercare prima le HggCat nel file 
  // e poi se non c'e' usare quelle EB/EE standard
  if(correction_map.count(GetElectronCategory(isEBEle, R9Ele, true))==1)
    return correction_map[GetElectronCategory(isEBEle, R9Ele, true)].first;
  else if(correction_map.count(GetElectronCategory(isEBEle, R9Ele, false))==1)
    return correction_map[GetElectronCategory(isEBEle, R9Ele, false)].first;
  else {
    std::cerr << "[ERROR] Electron category not found!!!!  Scale offset not applied" << std::endl;
    std::cout << "        " << runNumber << "\t" << runCorrection_itr->first << "\t" << runCorrection_itr->second.runMax << std::endl;
    exit(1);
    return 1;
  }
#endif

  TString category=GetElectronCategory(isEBEle, R9Ele, etaSCEle);
#ifdef DDEBUG
  std::cout << "[DEBUG] category(isEBEle, R9Ele) = " << category << "(" << isEBEle << ", " << R9Ele << ")" <<   correction_map_p->find(category)->second.first << std::endl;

#endif

  if(correction_map_p->count(category)==1)
    return correction_map_p->find(category)->second.first;
  else{
    std::cerr << "[ERROR] Electron category " << category << " not found!!!!  Scale offset not applied" << std::endl;
    std::cerr << "        " << runNumber << "\t" << runCorrection_itr->first << "\t" << runCorrection_itr->second.runMax << std::endl;
    exit(1);
    return 1;
  }

}

void EnergyScaleCorrection_class::Add(TString category_, int runMin_, int runMax_, double deltaP_, double err_deltaP_){

  // se le corrections non sono definite per Hgg (gold e bad) allora ignoro le righe che si riferiscono a queste categorie
  if( (!isHggCat) && (category_.Contains("gold") || category_.Contains("bad"))) {
#ifdef DEBUG
    std::cout << "[DEBUG] line of HggCat recalib: " << category_ << " ignore" << std::endl;
#endif
    return;
  }
  //  if( isHggCat && (!(category_.Contains("gold") || category_.Contains("bad")))){
#ifdef DEBUG
  //    std::cout << "[DEBUG] line of not HggCat recalib: " << category_ << " ignore" << std::endl;
#endif
  //   return;
  // }

  std::map <int, correction_t >::iterator itr=runMin_map.find(runMin_);
  if (itr!=runMin_map.end()){ // if exists
    if(itr->second.runMax != runMax_){
      std::cerr << "[ERROR] two run ranges with same runMin and different runMax" << std::endl;
      std::cerr << "        value not added" << std::endl;
      std::cerr << "        " << category_ << "\t" << runMin_ << "\t" << runMax_ << "\t" << deltaP_ << std::endl;
      std::cerr << "        " << itr->first << "\t" << itr->second.runMax << std::endl;
      return;
    }

    // probabilmente la categoria non e' ancora stata definita e la aggiungo
    //1-deltaP_; // deltaP_ e' lo shift, devo correggerlo (quindi -deltaP_)
    itr->second.correction_map[category_].first=deltaP_; // allo stato attuale passo come input il file con le correzioni (1-deltaP/100)
    itr->second.correction_map[category_].second=err_deltaP_; // inefficiente
    //#ifdef DEBUG
    std::cout << "[DEBUG] " << category_ << "\t" 
	      << itr->first << "-" << itr->second.runMax 
	      << "\t" << itr->second.correction_map.size() 
	      << "\t" << itr->second.correction_map[category_].first << std::endl;
    //"\t" << deltaP_ << "\t" << itr->second.correction_map[category_].first << std::endl;
    //std::cout << "[DEBUG] " << err_deltaP_ << "\t" << itr->second.correction_map[category_].second << std::endl;
    //#endif
    
    
  } else {
    correction_t corr;
    corr.runMax=runMax_;
    std::pair<double,double> pair_tmp;
    pair_tmp.first=deltaP_;
    pair_tmp.second=err_deltaP_;
    corr.correction_map[category_]=pair_tmp; //std::make_pair<double,double>(deltaP_, err_deltaP_); //(1-deltaP_, err_deltaP_);
    //    corr.correction_map[category_].second=err_deltaP_;
    runMin_map[runMin_]=corr; // aggiungo il nuovo
    itr=runMin_map.find(runMin_);
    
    std::cout << "[DEBUG] " << category_ << "\t" 
	      << itr->first << "-" << itr->second.runMax 
	      << "\t" << itr->second.correction_map.size() 
	      << "\t" << itr->second.correction_map[category_].first << std::endl;

  }
  return;
}

void EnergyScaleCorrection_class::ReadFromFile(TString filename){
  std::cout << "[STATUS] Reading recalibration values from file: " << filename << std::endl;
  ifstream f_in(filename);
  if(!f_in.good()){
    std::cerr << "[ERROR] file " << filename << " not readable" << std::endl;
    return;
  }
  
  int runMin, runMax;
#ifdef SHERVIN
  double deltaM_data, err_deltaM_data;
  double deltaM_MC, err_deltaM_MC;
#endif
  TString category, region2;
  double deltaP, err_deltaP;


  for(f_in >> category; f_in.good(); f_in >> category){
    f_in >> region2 
	 >> runMin >> runMax 
#ifdef SHERVIN
      	 >> deltaM_data >> err_deltaM_data 
      	 >> deltaM_MC >> err_deltaM_MC;
    deltaP=(deltaM_data - deltaM_MC)/91.188;
    err_deltaP=sqrt(err_deltaM_data*err_deltaM_data + err_deltaM_MC * err_deltaM_MC)/91.188;

#else
    >> deltaP >> err_deltaP;
#endif
    
#ifdef DEBUG
    std::cout << "[DEBUG]" << "\t" << "category" << "\t" << "runMin" << "\t" <<"deltaM_data" << "\t" << "deltaM_MC" << "\t" << "deltaP" << std::endl;
#ifdef SHERVIN
    std::cout << "[DEBUG]" << "\t" << category << "\t" << runMin << "\t" << runMax << "\t" << deltaM_data << "\t" << deltaM_MC << "\t" << deltaP << std::endl;
#else
    std::cout << "[DEBUG]" << "\t" << category << "\t" << runMin << "\t" << runMax << "\t" << "\t" << deltaP << "\t" << err_deltaP<< std::endl;
#endif
#endif
    Add(category, runMin, runMax, deltaP, err_deltaP);
  }
  
  f_in.close();
  runCorrection_itr=runMin_map.begin();
#ifdef DEBUG
  std::cout << "[STATUS DEBUG] runCorrection_itr->first: " << runCorrection_itr->first << "\t runMin_map.begin(): " << runMin_map.begin()->first << std::endl;
#endif

  return;
}



// tree is the input MC tree
// fastLoop = false if for any reason you don't want to change the branch status of the MC tree

TTree *EnergyScaleCorrection_class::GetCorrTree(TChain *tree, bool fastLoop, 
						TString runNumberBranchName,
						TString R9EleBranchName,
						TString etaEleBranchName,
						TString etaSCEleBranchName,
						TString nPVBranchName
						){
  float nPVmean = 0;
  if(correctionType.Contains("nPV"))
    nPVmean = GetMean_nPV(tree, fastLoop,nPVBranchName);

  Int_t nPV_;
  Int_t runNumber_;
  Float_t etaEle_[2];
  Float_t etaSCEle_[2];
  Float_t R9Ele_[2];

  Float_t scaleEle_[2];

  TTree *newTree = new TTree("scaleEle_"+correctionType,correctionType);
  newTree->Branch("scaleEle", scaleEle_, "scaleEle[2]/F");

  if(fastLoop){
    tree->SetBranchStatus("*",0);
    tree->SetBranchStatus(runNumberBranchName,1);
    tree->SetBranchStatus(etaEleBranchName,1);
    tree->SetBranchStatus(etaSCEleBranchName,1);
    tree->SetBranchStatus(R9EleBranchName,1);
    tree->SetBranchStatus(nPVBranchName,1);
  }

  tree->SetBranchAddress(nPVBranchName, &nPV_);
  tree->SetBranchAddress(runNumberBranchName, &runNumber_);
  tree->SetBranchAddress(etaEleBranchName,etaEle_);
  tree->SetBranchAddress(etaSCEleBranchName,etaSCEle_);
  tree->SetBranchAddress(R9EleBranchName, R9Ele_);
  Long64_t nentries = tree->GetEntries();



  std::cout << "[STATUS] Get scaleEle for tree: " << tree->GetTitle() 
	    << "\t" << "with " << nentries << " entries" << std::endl;
  std::cerr << "[00%]";

  // loop over tree 
  for(Long64_t ientry = 0; ientry<nentries; ientry++){
    tree->GetEntry(ientry);
    scaleEle_[0] = ScaleCorrection(runNumber_, fabs(etaSCEle_[0]) < 1.4442, 
				   R9Ele_[0],etaEle_[0], nPV_, nPVmean);
    scaleEle_[1] = ScaleCorrection(runNumber_, fabs(etaSCEle_[1]) < 1.4442, 
				   R9Ele_[1],etaEle_[1], nPV_, nPVmean);
    newTree->Fill();
    if(ientry%(nentries/100)==0) std::cerr << "\b\b\b\b" << std::setw(2) << ientry/(nentries/100) << "%]";
  }

  
  if(fastLoop) tree->SetBranchStatus("*",1);
  tree->ResetBranchAddresses();
  tree->GetEntry(0);
  //  std::cout << "[WARNING] nPU > nPU max for " << warningCounter << " times" << std::endl;
  return newTree;
}



//============================== SMEARING
void EnergyScaleCorrection_class::AddSmearing(TString category_, int runMin_, int runMax_, 
				      double smearing_, double err_smearing_){

  // se le corrections non sono definite per Hgg (gold e bad) allora ignoro le righe che si riferiscono a queste categorie
  if( (!isHggCat) && (category_.Contains("gold",TString::kIgnoreCase) || category_.Contains("bad",TString::kIgnoreCase))) {
    return;
  }

  if(smearings.count(category_)!=0){
    std::cerr << "[WARNING] duplicated smearing number in input file" << std::endl;
    return;
  }
  correction_t corr;
  corr.runMax=runMax_;
  std::pair<double,double> pair_tmp;
  pair_tmp.first=smearing_;
  pair_tmp.second=err_smearing_;
  smearings[category_]=pair_tmp; 

  return;
}

void EnergyScaleCorrection_class::ReadSmearingFromFile(TString filename){
  std::cout << "[STATUS] Reading smearing values from file: " << filename << std::endl;
  ifstream f_in(filename);
  if(!f_in.good()){
    std::cerr << "[ERROR] file " << filename << " not readable" << std::endl;
    exit(1);
  }
  
  int runMin=0, runMax=900000;
  TString category, region2;
  double smearing, err_smearing;


  for(f_in >> category; f_in.good(); f_in >> category){
    f_in //>> region2 
	 //>> runMin >> runMax 
      >> smearing >> err_smearing;
    
    AddSmearing(category, runMin, runMax, smearing, err_smearing);
  }
  
  f_in.close();
  //  runCorrection_itr=runMin_map.begin();
  
  noSmearings=false;
  return;
}


float EnergyScaleCorrection_class::getSmearingSigma(float energy, bool isEBEle, float R9Ele, float etaSCEle){
  //const correction_map_t *correction_map_p = &(runCorrection_itr->second.correction_map);
  TString category=GetElectronCategory(isEBEle, R9Ele, etaSCEle);
  
  if(smearings.count(category)==0){
    std::cerr << "[ERROR] Electron category " << category << " not found!!!!  Smearing not applied" << std::endl;
    exit(1);
  }

  return smearings.find(category)->second.first;
}

float EnergyScaleCorrection_class::getSmearing(float energy, bool isEBEle, float R9Ele, float etaSCEle){

  if(noSmearings) return 1;
  return rgen_->Gaus(1,getSmearingSigma(energy, isEBEle, R9Ele, etaSCEle));
}


TTree *EnergyScaleCorrection_class::GetSmearTree(TChain *tree, bool fastLoop, 
						 TString energyEleBranchName,
						 TString R9EleBranchName,
						 TString etaEleBranchName,
						 TString etaSCEleBranchName
						 
						 ){
  Float_t energyEle_[2];
  Float_t etaEle_[2];
  Float_t etaSCEle_[2];
  Float_t R9Ele_[2];

  Float_t smearEle_[2];

  TTree *newTree = new TTree("smearEle_"+correctionType,correctionType);
  newTree->Branch("smearEle", smearEle_, "smearEle[2]/F");

  if(fastLoop){
    tree->SetBranchStatus("*",0);
    tree->SetBranchStatus(energyEleBranchName,1);
    tree->SetBranchStatus(etaEleBranchName,1);
    tree->SetBranchStatus(etaSCEleBranchName,1);
    tree->SetBranchStatus(R9EleBranchName,1);
  }


  tree->SetBranchAddress(energyEleBranchName, energyEle_);
  tree->SetBranchAddress(etaEleBranchName,etaEle_);
  tree->SetBranchAddress(etaSCEleBranchName,etaSCEle_);
  tree->SetBranchAddress(R9EleBranchName, R9Ele_);
  
  // loop over tree 
  std::cerr << "Adding smearEle branch \t";
  std::cerr << "[00%]";

  Long64_t nentries = tree->GetEntries();
  for(Long64_t ientry = 0; ientry<tree->GetEntriesFast(); ientry++){
    tree->GetEntry(ientry);
    smearEle_[0] = getSmearing(energyEle_[0], fabs(etaSCEle_[0]) < 1.4442, 
			       R9Ele_[0],etaEle_[0]);
    smearEle_[1] = getSmearing(energyEle_[1], fabs(etaSCEle_[1]) < 1.4442, 
			       R9Ele_[1],etaEle_[1]);
    newTree->Fill();
    if(ientry%(nentries/100)==0) std::cerr << "\b\b\b\b" << std::setw(2) << ientry/(nentries/100) << "%]";

  }
  std::cerr << std::endl;
  
  if(fastLoop) tree->SetBranchStatus("*",1);
  tree->ResetBranchAddresses();
  tree->GetEntry(0);
  return newTree;
}




float EnergyScaleCorrection_class::ScaleCorrection(int runNumber, bool isEBEle, double R9Ele, double etaSCEle, 
						   int nPV, float nPVmean){
  float correction = 1;
  if(correctionType.Contains("Run", TString::kIgnoreCase)) correction*=getScaleOffset(runNumber, isEBEle, R9Ele, etaSCEle);
  
  if(correctionType.Contains("nPV",TString::kIgnoreCase)) correction *= 1- (-0.02 * (nPV -nPVmean))/100;
  
  return correction;

}


float EnergyScaleCorrection_class::GetMean_nPV(TChain *tree, bool fastLoop, 
						TString nPVBranchName
						){
  float nPVmean=0;
  Int_t nPV_;
  if(fastLoop){
    tree->SetBranchStatus("*",0);
    tree->SetBranchStatus(nPVBranchName,1);
  }

  tree->SetBranchAddress(nPVBranchName, &nPV_);
  Long64_t nentries = tree->GetEntries();

  // loop over tree 
  for(Long64_t ientry = 0; ientry<nentries; ientry++){
    tree->GetEntry(ientry);
    nPVmean+=nPV_;
  }
  nPVmean/=nentries;
  
  if(fastLoop) tree->SetBranchStatus("*",1);
  tree->ResetBranchAddresses();
  tree->GetEntry(0);
  //  std::cout << "[WARNING] nPU > nPU max for " << warningCounter << " times" << std::endl;
  return nPVmean;
}
