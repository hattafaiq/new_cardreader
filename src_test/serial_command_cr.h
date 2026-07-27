#ifndef SERIAL_COMMAND_CRT_H
#define SERIAL_COMMAND_CRT_H
#include <string_view>
#include <cstdint>
#include <cstring>
#include <vector>
#include <iostream>
#include <memory>
#include <string>

#include <chrono>
#include <functional>
#include <bitset>
#include <thread>

#include <sstream>
#include <algorithm>
#include <iomanip>
#include "Windows.h"

class SerialLoader;

struct JenisKartuResponse{
    bool status = false;
    std::string jenis = "";
};

struct DataKartuResponse{
	std::string cardnumber = "";
	std::string track2data = "";
	std::string modecard = "";
	std::string tag5F34 = "";
	std::string iccdata = "";
	bool status = false; 
};

// 9F1A	Terminal Country
// 95	TVR (Terminal Status)
// 5F2A	Currency Code
// 9A	Date (YYMMDD)
// 9C	Transaction Type
// 9F37	Unpredictable No

// •	8E 10 ...: Tag 8E (Cardholder Verification Method / CVM List). Menentukan bagaimana pemilik kartu harus diverifikasi (misal: PIN atau tanda tangan).
// •	9F0D 05 BC608C8800: Issuer Action Code (Default).IACDefault
// •	9F0E 05 0010000000: Issuer Action Code (Denial).IACDenial
// •	9F0F 05 BC689C9800: Issuer Action Code (Online).IACOnline
//     Tag	Deskripsi	Panjang	Contoh Data (Hex)
// 9F02	Amount (Nominal)	6 Byte	000000010000 (Rp 10.000)
// 9F03	Amount Other	6 Byte	000000000000
// 9F1A	Terminal Country	2 Byte	0360 (Indonesia)
// 95	TVR (Terminal Status)	5 Byte	0000000000 (Asumsi aman)
// 5F2A	Currency Code	2 Byte	0360 (IDR)
// 9A	Date (YYMMDD)	3 Byte	260512 (12 Mei 2026)
// 9C	Transaction Type	1 Byte	00 (Purchase)
// 9F37	Unpredictable No	4 Byte	AABBCCDD (Random)

// --- STRUCTURES FOR HOLDING CARD DATA ---
struct CardDetails {
  // Data Kartu Umum (Dari READ RECORD / GPO)
    std::string expiryDate = "Tidak Ditemukan";
    std::string cardholderName = "Tidak Ditemukan";
    std::string serviceCode = "Tidak Ditemukan";
    std::string discretionaryData = "Tidak Ditemukan";
    bool isTrack2Found = false;

    //data untuk generate ac format cdol 1
    
    std::string tvr = "0000000000";// aku masih pusing kalo membatasi TVR //Tag 95
    std::string cardholderverification = "Tidak Ditemukan";
    std::string countrycode = "Tidak Ditemukan"; //Tag 5F1A sama dengan curency code

    std::string IACDefault = "Tidak Ditemukan";
    std::string IACDenial = "Tidak Ditemukan";
    std::string IACOnline = "Tidak Ditemukan";

    std::string cdol1Raw = "Tidak Ditemukan";    // Aturan pengisian 29 byte transaksi
    std::string cdol2Raw = "Tidak Ditemukan";    // Aturan pengisian 29 byte transaksi
    bool isPanFound = false;

    // Data Kriptogram (Dari GENERATE AC)
    std::string arqc = "Tidak Ditemukan";        // Tag 9F26
    std::string cid = "Tidak Ditemukan";         // Tag 9F27
    std::string iad = "Tidak Ditemukan";        // Tag 9F10
    std::string atc = "Tidak Ditemukan";         // Tag 9F36
    std::string unpredictableNo = "Tidak Ditemukan";//generate dari kita 9F37
    std::string TerminalVerificationResults = "0000000000"; //Tag 95 hasil verivikasi //hardcode karena belum tau rumus dan kesepakatanya
    std::string transactionDate = "Tidak Ditemukan"; // tag 9A (YYMMDD)
    std::string transactionType = "00"; //00 asumsi purcase butuh info dari mandiri mau transaksi tipenya apa //tag 9C //hardcode karena belum tau rumus dan kesepakatanya
    std::string ammountDeposit = "000000010000"; //Tag 9F02
    std::string ammountOther = "000000000000"; //Tag 9F03
    std::string pan = "Tidak Ditemukan"; //5A
    std::string currencyCode = "Tidak Ditemukan"; //tag 5F2A
    std::string ApplicationInterchangeProfile = "Tidak Ditemukan"; //Tag 82
    std::string terminalCountry = "Tidak Ditemukan";//Tag 9F1A
    std::string panSequenceNumber = "xxx";        // Tag 5F34
    std::string TerminalCapabilities = "604020"; //9F33 //hardcode karena belum tau rumus dan kesepakatanya
    std::string CardholderVerificationMethod = "020002"; //9F34 //hardcode karena belum tau rumus dan kesepakatanya
    // 020002 (Unconditional): Terminal meminta PIN Online secara mutlak tanpa memedulikan jenis transaksinya. Baik transaksi itu belanja biasa, tarik tunai, maupun cashback, terminal akan selalu meminta PIN Online.020202 (Conditional): Terminal meminta PIN Online karena melihat jenis transaksinya adalah pembelian barang/jasa standar (bukan tarik tunai/cashback). Nilai ini muncul karena disesuaikan dengan isi aturan (CVM List) yang tertanam di dalam chip kartu.
    std::string TerminalType = "14"; //9F35 //hardcode karena belum tau rumus dan kesepakatanya
    std::string InterfaceDevice = "5331444D30303032"; 
    std::string DedicatedFile = "kosong";
    std::string ApplicationVersionNumber = "01"; //hardcode karena belum tau rumus dan kesepakatanya
    std::string TransactionSequenceCounter = "0";

    std::string track2Data = "Kosong";
    std::string posEntryMode ="Kosong";
    std::string IccFull = "Kosong";
    bool isAcSuccess = false;
    std::string ApplicationCurrencyExponent = "0";//0x9F44
    std::string priority="0";
};

struct CardData{
    std::string track1;
    std::string track2;
    std::string track3;
};

struct TLV {
    uint32_t tag;
    uint32_t length;
    std::vector<uint8_t> value;

    // Helper untuk mendapatkan value dalam bentuk Hex String
    std::string getValueHex() const;
};

class CommandCardCRT{
public:
    explicit CommandCardCRT();
    ~CommandCardCRT();
    std::string selectHighestPriorityAid(std::string hexData);
    bool IsOpen();
    bool Open(std::string_view com,int bautrate);
    void Close();
    bool Init(bool EjectFrontOrRear = true); // 1 rear
    bool PermitMagCardOnly();
    int IdentifyCard();
    bool SelectCard();
    std::string generateIad28Bytes(std::string iadHex);
    bool ProhibitCardIn();
    bool Eject();
    bool Capture();
    bool Retrieve();
    int CardStatus();
    bool LEDBlinkGreen();
    bool LEDBlinkOrange();
    bool LEDBlinkRed();
    bool LEDOFF();
    
    void GetMagCardStatusBtn();
    std::string ReadData(uint8_t index=2);
    std::string EncodeMagDataStatus(uint8_t *status);
    CardData CheckMAgData();
    void OnICTestTypeBtn();
    CardData ReadCard(int timeout,int *status = nullptr);
    int RejectCardOrMoveBin(int timeout);
    void OnFrontEnterCardBySwitchBtn();
	void OnFrontNoCardEnterBtn();
    int gethexvalue(char p);
    int GetDECData(unsigned char *pData,std::string_view str);
    int OnCPURESETButton() ;
    void OnCPUAPDUSENDButton(std::string datass);
    int JenisKartu(JenisKartuResponse *datass);
	int GetDataKartu(int ammount_data, int flag,DataKartuResponse *datass);
    
    
    //util
    void ClearData();
    bool PaymentSystemEnvironment(unsigned char *ret);
    std::string GetResponse(int mode, unsigned char *SEt1, std::vector<uint8_t> *GenResp);
    std::string ReadRecord();
    std::string SelectAID();
    std::string GetProcessingOptions();
    std::string CommandData(std::string data,unsigned char *SEt0,
                                          unsigned char *SEt1);
    std::string CariTag88(std::string datas);   
    std::string ConvertBytetoString(std::vector<BYTE> data);    
    std::vector<unsigned char> hexToBytes(const std::string& hex);                       
    std::string bytesToHex(const std::vector<unsigned char>& bytes);
    bool findTagValue(const std::vector<unsigned char>& tlvData, size_t start, size_t end, unsigned int targetTag, std::vector<unsigned char>& outValue);
    std::vector<unsigned char> generateTerminalData(unsigned int tag, size_t length);
    std::string MulaiGPO(std::string data);
    std::string buildDynamicGpo(const std::vector<unsigned char>& pdolData);
    std::string toHexStr(unsigned char value);
    int CariAFL(std::string data);
    std::string GenerateAFL(std::string data);

    void parseEMVTLV(const std::vector<unsigned char>& data, int flag=0);
    void parsingGenerateAC(const std::string& hex);
    int GenerateACCommand(std::string formatCdol);
    std::string hexToAscii(const std::string& hex);
    std::string hexToAsciiString(const std::string& hex);
    /**
     * @brief Melakukan parsing data mentah menjadi daftar objek TLV
     * @param data Pointer ke buffer data
     * @param size Ukuran buffer
     */
    static std::vector<TLV> parse(const uint8_t* data, size_t size);

    /**
     * @brief Mencetak struktur TLV ke console (rekursif untuk tag constructed)
     */
    static void debugPrint(const std::vector<TLV>& items, int *tugas, int *perintah, int indent = 0);

private:
    std::string namaFile = "counttra.txt";
    std::string m_lastcom;
    int m_lastbautrate;
    bool is_open = false;
    std::string_view	m_APDUSENDSTRINGCMD;
    std::string_view	m_APDUBACKSTRING;
    static bool isConstructed(uint32_t tag);
    CardDetails emv;
    int StatusKartu =0;
    //DataKartuResponse DK_Buffer;
};
#endif