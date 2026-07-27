#include <bitset>
#include <chrono>
#include <cstdint>
#include <iostream>
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
int main(int a, char** b) {
  if (b[1] == nullptr) {
    std::cout << "\nusage " << b[0] << " com[1/2/3..]";
    return 1;
  }
  testhigh(b[1]);
  return 0;
}
