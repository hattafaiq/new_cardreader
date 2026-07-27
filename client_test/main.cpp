#include <iostream>
#include <QCoreApplication>
#include <memory>
#include <qcoreapplication.h>
#include <qobject.h>
#include "cardreader.h"
#include <QDebug>
#include <bitset>

struct CardData{
    std::string track1;
    std::string track2;
    std::string track3;
};

void run1(int a,char**b){
    QCoreApplication app(a,b);
    auto mgr = std::make_unique<CardReaderMgr >();
    QObject::connect(mgr.get(), &CardReaderMgr::OnOpen,[&app,&mgr](bool status){
        qDebug()<<"status "<<status;
        mgr->EndThread();
        app.exit();
    });

    int inputcmd = 0;
  int maxcmd = 22;
  bool askExit = false;

  while (!askExit) {
    do {
      inputcmd = 0;
      std::cout << "\nDebug cdm command v1.0..\n";
      std::cout << "\n1) Open device";
      std::cout << "\n2) Init device";
      std::cout << "\n3) PermitMagCardOnly";
      std::cout << "\n4) Card Status";
      std::cout << "\n5) Eject";
      std::cout << "\n6)ProhibitCardIn";
      std::cout << "\n7) Read Data";
      std::cout << "\nInput:";
      std::cin >> inputcmd;
    } while (inputcmd > maxcmd || inputcmd <= 0);

    switch (inputcmd) {
    case 1: {
        mgr->setup("COM4",9233);
        mgr->Open();
    } break;
    case 2: {
        mgr->Init(); //buang kedepan false 
    } break;
    case 3: {
        mgr->PermitMagCardOnly(); 
    } break;
    case 4: {
       mgr->CardStatus(); 
    } break;
    case 5: {
       mgr->Eject(); 
    } break;
    case 6: {
       // card = mgr->CardStatus(); 
    } break;
    case 7: {
        CardData data;
        uint8_t status_mag = 0;
        mgr->WaitInputReadCard();
        // std::bitset<8> bit_mag = status_mag;
        // if(bit_mag.test(0)){
        //     data.track1 = mgr->ReadData(1);
        //     std::cout<<"Track 1 = "<<data.track1;
        // }
        // if(bit_mag.test(1)){
        //     data.track2 = mgr->ReadData(2);
        //     std::cout<<"Track 2 = "<<data.track2;
        // }
        // if(bit_mag.test(2)){
        //     data.track3 = mgr->ReadData(3);
        //     std::cout<<"Track 3 = "<<data.track3;
        // }
    } break;
    }
  }

    

    app.exec();
}

int main(int a,char**b)
{
    run1(a,b);
    return 0;
}
