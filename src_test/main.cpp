#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>

#include "serial_command_cr.h"


void test() {
  CommandCardCRT crt;
  crt.Open("COM5",38400);
  if (crt.IsOpen()) {
    bool ret = crt.Init();
    if (ret) {
      std::cout << "\ninit ok";
      ret = crt.PermitMagCardOnly();
      if (ret) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // ret = crt.Capture();
        ret = crt.LEDBlinkRed();
        int stat = -1;
        while (stat != 2) {
          stat = crt.CardStatus();
          std::this_thread::sleep_for(std::chrono::seconds(1));
          std::cout << "\nlast stat " << stat;
        }
        uint8_t mag = 0;
        auto data = crt.EncodeMagDataStatus(&mag);
        std::bitset<4> bitmag = mag;
        if (bitmag.test(0)) {
          std::cout << "\nbit 0 defined";
          data = crt.ReadData(1);
          std::cout << "\nfound data: " << data;
        }
        if (bitmag.test(1)) {
          data = crt.ReadData(2);
          std::cout << "\nfound data: " << data;
          std::cout << "\nbit 1 defined";
        }
        if (bitmag.test(2)) {
          data = crt.ReadData(3);
          std::cout << "\nfound data: " << data;
          std::cout << "\nbit 2 defined";
        }
        if (bitmag.test(3)) {
          std::cout << "\nbit 3 defined";
        }
        std::cout << "\nencode msg fmt: " << data;
        std::this_thread::sleep_for(std::chrono::seconds(1));

        ret = crt.LEDOFF();

        ret = crt.Eject();

        std::cout << "\neject again resp " << ret;
      }
    } else {
      std::cout << "\ninit fail";
    }
    crt.Close();
  } else {
    std::cout << "\nnot open";
  }
}
void testhigh(const char* com) {
  int cmd = 2;
  int cur = 0;

  CommandCardCRT crt;
  crt.Open(com,38400);
  if (!crt.IsOpen()) {
    std::cout << "\ncannot open com: " << com;
    return;
  }
  do {
    std::cout << "\nhigh level test card reader";
    std::cout << "\n1. read card";
    std::cout << "\n2. end card";
    std::cout << "\n3. exit";
    std::cout << "\n";
    std::cin >> cur;
    switch (cur) {
      case 1: {
        int status = 0;
        auto resp = crt.ReadCard(10, &status);
        std::cout << "\ncard record1: " << resp.track1;
        std::cout << "\ncard record2: " << resp.track2;
      } break;
      case 2: {
        int ret = crt.RejectCardOrMoveBin(15);
        std::cout << "\nret " << ret;
      } break;
      default:
        std::cout << "\ninvalid menu..";
        break;
    }
  } while (cur != 3);
  
}
int main(int a) {
   int inputcmd = 0;
  int maxcmd = 22;
  bool askExit = false;

   CommandCardCRT crt;
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
      std::cout << "\n8) Blink LED";
      std::cout << "\n9) LED OFF";
      std::cout << "\n10) Data Mag";
      std::cout << "\n11) Close";
      std::cout << "\n12) OnICTestTypeBtn";
      std::cout << "\n13) OnFrontEnterCardBySwitchBtn";
      std::cout << "\n14) OnFrontNoCardEnterBtn";
      std::cout << "\n15) Activate Card ICC";
      std::cout << "\n16) SendCommand";
      std::cout << "\n18) Select Card";
      std::cout << "\n19) GetDataKartu";
      
      std::cout << "\nInput:";
      std::cin >> inputcmd;
    } while (inputcmd > maxcmd || inputcmd <= 0);
  
    
    switch (inputcmd) {
    case 1: {
       crt.Open("COM4",38400);
    } break;
    case 2: {
       crt.Init(true);
    } break;
    case 3: {
       crt.PermitMagCardOnly();
    } break;
    case 4: {
        int ret = -1;
        ret = crt.CardStatus();
        std::cout << ":ret:" <<ret ;
    } break;
    case 5: {
           crt.Eject();
    } break;
    case 6: {
      
     
    } break;
    case 7: {
        int datas = 1;
        std::cin >> datas;
        crt.ReadData(datas);
    } break;
        case 8: {
        crt.LEDBlinkGreen(); 
    } break;
        case 9: {
         crt.LEDOFF();
    } break;
    case 10:{
        crt.CheckMAgData();
    }break;
    case 11: {
         crt.Close();
    } break;
        case 12: {
         crt.OnICTestTypeBtn();
    } break;
    case 13:{
        crt.OnFrontEnterCardBySwitchBtn();
    }break;
    case 14: {
         crt.OnFrontNoCardEnterBtn();
    } break;
    case 15: {
        crt.OnCPURESETButton();
    } break;
    case 16: {
        std::string buffer;
        std::cout << "Masukkan perintah APDU: ";
        
        std::cin >> buffer; 
        crt.OnCPUAPDUSENDButton(buffer);
    } break;
    case 17: {
        std::string buffer;
        unsigned char ret;
        std::vector<uint8_t> GenResp;
        int tugas=0; int perintah=0; int inden=0;
        std::cout << "PaymentSystemEnvironment";
        if(crt.PaymentSystemEnvironment(&ret) == 0){
            buffer = crt.GetResponse(0,&ret,&GenResp);
            auto results = crt.parse(GenResp.data(), GenResp.size());
            
            tugas=1;
            crt.debugPrint(results, &tugas, &perintah, inden);
            std::cout << "Tugas="<<tugas<<" || "<<"Perintah="<<perintah <<std::endl;
            
            tugas=2;
            // if(perintah==1)
            // else if(perintah==2)

            
        }
        else{
          std::cout << "PaymentSystemEnvironment Failed!";
        }
    } break;
    case 18: {
      crt.SelectCard();
    }
    case 19: {
        int mydat;
        std::cout << "masukkan data: ";
        std::cin >> mydat; // Reads the input and stores it in myNumber
        
        DataKartuResponse ad;
        crt.GetDataKartu(12000,mydat,&ad);
    }
    
    }
  }
  return 0;
}
