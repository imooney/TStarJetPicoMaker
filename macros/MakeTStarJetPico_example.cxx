
//#include "TStarJetPicoUtils.h"


/* STAR Collaboration - Nick Elsey
  
    Example of how to use TStarJetPicoMaker to produce
    a tree of TStarJetPicoEvents
  
    this macro produces TStarJetPicoEvents from STAR muDSTs. 
    Requires the STAR libraries & muDST files. Defaults 
    are set to a test production using the test.list file
    ( 10 files located on distributed disk from y14 high 
    luminosity production )
  
    arguments -- 
    nEvents:    number of events to reproduce
    filelist:   list of filenames & paths to muDSTs
    filelistMC: list of filenames & paths to minimcs
    nametag:    identifier used in output file name
    nFiles:     number of files to accept from the file list
    trigSet:    an identifier for the switch statement defining
                trigger IDs to be reproduced by the maker
*/

void MakeTStarJetPico_example(int nEvents = 1e3,
			      const char* filelist = "/star/u/imooney/TStarJetPicoMaker/list/some_MuDsts.list"/*test.list*/, 
				  char* filelistMC = "/star/u/imooney/TStarJetPicoMaker/list/some_minimcs.list",
                 const char* nametag  = "TStarJetPicoMaker_test",
                 int nFiles = 6,
                 int trigSet =0
                  )
{
  // load STAR libraries
  gROOT->Macro( "LoadLogger.C" );
  gROOT->Macro( "loadMuDst.C" );
  
  gSystem->Load( "StarMagField.so" );
  gSystem->Load( "StMagF" );
  gSystem->Load( "StDetectorDbMaker" );
  gSystem->Load( "StTpcDb" );
  gSystem->Load( "St_db_Maker" );
  gSystem->Load( "StDbUtilities" );
  gSystem->Load( "StMiniMcEvent" );
  gSystem->Load( "StMiniMcMaker" );
  gSystem->Load( "StDaqLib" );
  gSystem->Load( "StEmcRawMaker" );
  gSystem->Load( "StEmcADCtoEMaker" );
  gSystem->Load( "StEpcMaker" );
  gSystem->Load( "StTriggerUtilities" );
  gSystem->Load( "StDbBroker" );
  gSystem->Load( "libgeometry_Tables" );
  gSystem->Load( "StEEmcUtil" );
  gSystem->Load( "StEEmcDbMaker" );
  gSystem->Load( "StPreEclMaker" );
  gSystem->Load( "StEpcMaker" );
  gSystem->Load("StPicoEvent.so");
  gSystem->Load("StPicoDstMaker.so");
  // load local StRefMultCorr
  gSystem->Load( "libStRefMultCorr.so" );
  // load local TStarJetPico library & its maker
  gSystem->Load( "libTStarJetPico.so" );
  gSystem->Load( "libTStarJetPicoMaker.so" );
	
  StChain* chain           = new StChain( "StChain" );
  TChain * mcChain         = new TChain ( "TChain" );
  mcChain = TStarJetPicoUtils::BuildChainFromFileList( filelistMC, "StMiniMcTree" );
  
  StMuDstMaker* muDstMaker = new StMuDstMaker( 0, 0, "", filelist, "", nFiles );
  //StMiniMcMaker* miniMcMaker = new StMiniMcMaker(filelistMC);

  St_db_Maker *dbMaker     = new St_db_Maker( "StarDb", "MySQL:StarDb" );
  //StMiniMcMaker* mcMaker = new StMiniMcMaker();
  StEEmcDbMaker* eemcb     = new StEEmcDbMaker( "eemcDb" );
  StEmcADCtoEMaker *adc    = new StEmcADCtoEMaker();
  StPreEclMaker *pre_ecl   = new StPreEclMaker();
  StEpcMaker *epc          = new StEpcMaker();
  
  // get control table so we can turn off BPRS zero-suppression and save hits from "bad" caps
  controlADCtoE_st* control_table  = adc->getControlTable();
  control_table->CutOff[1]         = -1;
  control_table->CutOffType[1]     = 0;
  control_table->DeductPedestal[1] = 2;
  adc->saveAllStEvent( kTRUE );
  
  // simulates a trigger response based on an ADC value & trigger definitions
  StTriggerSimuMaker* trigsim = new StTriggerSimuMaker();
  trigsim->setMC( false ); //CHANGE IF GOING BACK TO DATA!!
  trigsim->useBemc();
  trigsim->useEemc();
  trigsim->useBbc();
  trigsim->useOnlineDB();
  trigsim->bemc->setConfig( StBemcTriggerSimu::kOffline);
  
  //making the nametag for output root files
  std::string filestr = (std::string) std::string(filelistMC);
  std::string front = "minimcs";
  size_t front_len = front.size();
  size_t pos_front = filestr.find(front);
  size_t pos_begin = pos_front + front_len;
  std::string back = ".list";
  size_t back_len = back.size();
  size_t pos_back = filestr.find(back);

  std::string unique_id = filestr.substr(pos_begin, pos_back - pos_begin);
  std::string unique_name = "pt-hat"+unique_id;

  // builds the TStarJetPicoDST
  //format: TStarJetPicoMaker::TStarJetPicoMaker(std::string outFileName, TChain* mcTree, inputMode input, std::string name, int nFiles, int trigSet)
  TStarJetPicoMaker *jetPicoMaker = new TStarJetPicoMaker( Form( "%s.root", /*nametag*/unique_name ), mcChain, 1, nametag, nFiles, trigSet );
  jetPicoMaker->ProcessMC(1);
  jetPicoMaker->SetVertexSelector( TStarJetPicoMaker::VpdOrRank );
  jetPicoMaker->SetTowerAcceptMode( TStarJetPicoMaker::RejectBadTowerStatus );
  jetPicoMaker->SetStRefMultCorrMode( TStarJetPicoMaker::FillNone );
  jetPicoMaker->EventCuts()->SetVzRange( -30, 30 );
  jetPicoMaker->EventCuts()->SetRefMultRange( 0, 7000 );
  jetPicoMaker->SetTowerEnergyMin( 0.15 );
  jetPicoMaker->SetTrackEtaRange( -1.5, 1.5 );
  jetPicoMaker->SetTrackFitPointMin( 10 );
  jetPicoMaker->SetTrackDCAMax( 3.0 );
  jetPicoMaker->SetTrackFlagMin( 0 );
  // set triggers to be reproduced
  switch (trigSet) {
  case 0://BHT2*BBCMB for Y15 p+Au
    //COMMENTING OUT THESE TWO EVENT CUTS BECAUSE WE'LL DO TRIGGER SELECTION ONCE WE HAVE THE PICOS
    //jetPicoMaker->EventCuts()->AddTrigger(500205);
    //jetPicoMaker->EventCuts()->AddTrigger(500215);
    //these are the AuAu triggers
    /*jetPicoMaker->EventCuts()->AddTrigger(450202);
      jetPicoMaker->EventCuts()->AddTrigger(450212);
      jetPicoMaker->EventCuts()->AddTrigger(450203);
      jetPicoMaker->EventCuts()->AddTrigger(450213);
    */
      break;
  case 1://JP2 for Y15 p+Au [CAUTION USING THIS - ABNORMAL JP2 THRESHOLD FOR NEGATIVE RAPIDITY]
    //COMMENTING OUT THESE TWO EVENT CUTS BECAUSE WE'LL DO TRIGGER SELECTION ONCE WE HAVE THE PICOS
    //jetPicoMaker->EventCuts()->AddTrigger(500401);
    //jetPicoMaker->EventCuts()->AddTrigger(500411);
      
      //these are the AuAu triggers
      /*
      jetPicoMaker->EventCuts()->AddTrigger(450010);
      jetPicoMaker->EventCuts()->AddTrigger(450020);
      jetPicoMaker->EventCuts()->AddTrigger(450008);
      jetPicoMaker->EventCuts()->AddTrigger(450018);
      jetPicoMaker->EventCuts()->AddTrigger(450012);
      jetPicoMaker->EventCuts()->AddTrigger(450022);
      */
      break;
    default :
      break;
  }

  // for each event, print the memory usage
  // helpful for debugging
  StMemStat memory;
  memory.PrintMem( NULL );
	
  if ( chain->Init() ) { cout<<"StChain failed init: exiting"<<endl; return;}
  cout << "chain initialized" << endl;
	
  TStopwatch total;
  TStopwatch timer;
	
  int i=0;
  while ( i < nEvents && chain->Make() == kStOk ) {
    if ( i % 500 == 0 ) {
      cout<<"done with event "<<i;
      cout<<"\tcpu: "<<timer.CpuTime()<<"\treal: "<<timer.RealTime()<<"\tratio: "<<timer.CpuTime()/timer.RealTime();//<<endl;
      timer.Start();
      memory.PrintMem( NULL );
    }
    i++;
    chain->Clear();
  }
  
  chain->ls( 3 );
  chain->Finish();
  printf( "my macro processed %i events in %s", i, nametag );
  cout << "\tcpu: " << total.CpuTime() << "\treal: " << total.RealTime() << "\tratio: " << total.CpuTime() / total.RealTime() << endl;

  cout << endl;
  cout << "-------------" << endl;
  cout << "(-: Done :-) " << endl;
  cout << "-------------" << endl;
  cout << endl;
}
