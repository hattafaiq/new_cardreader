

#include "serial_command_cr.h"
#include <random>
#include <fstream>
#include <map>

#define USE_DL 0

#define PSE1 "00A404000E315041592E5359532E4444463031"

#define RESP "00C00000"

#include "CRT_310_NR01.h"

#define SUCSESS_RESP 0
#define FAIL_RESP 1
#define DISCONNECT_RESP 2

#define COMSelectPSE "00A404000E315041592E5359532E4444463031" 

static HANDLE buf = NULL;

struct Application {
    std::string aid;
    std::string label;
    int priority = 99; // Default prioritas rendah jika tidak ditemukan
};

// Struktur data untuk menampung satu blok instruksi AFL (4 byte)
struct AflEntry {
    unsigned char sfi;          // Short File Identifier murni (sudah di-decode)
    unsigned char startRecord;  // Record pertama yang harus dibaca
    unsigned char endRecord;    // Record terakhir yang harus dibaca
    unsigned char odaBytes;     // Byte untuk kalkulasi otentikasi data (offline data authentication)
};

// Struktur data untuk menampung hasil generate perintah APDU
struct ApduCommand {
    std::string apduHex;
    unsigned char sfi;
    unsigned char recordNumber;
};

std::string CommandCardCRT::selectHighestPriorityAid(std::string hexData) {
    // Hilangkan status word "9000" di akhir jika ada
    if (hexData.length() >= 4 && hexData.substr(hexData.length() - 4) == "9000") {
        hexData = hexData.substr(0, hexData.length() - 4);
    }

    std::vector<Application> appList;
    size_t pos = 0;

    // Cari semua Tag 61 (Application Template)
    while ((pos = hexData.find("61", pos)) != std::string::npos) {
        if (pos + 4 > hexData.length()) break;

        // Ambil panjang data dari Tag 61
        std::string lenHex = hexData.substr(pos + 2, 2);
        int lenBytes = std::stoul(lenHex, nullptr, 16);
        int lenChars = lenBytes * 2;

        if (pos + 4 + lenChars > hexData.length()) break;
        std::string appContent = hexData.substr(pos + 4, lenChars);

        Application app;

        // 1. Ekstrak Tag 4F (AID)
        size_t aidPos = appContent.find("4F");
        if (aidPos != std::string::npos && aidPos + 4 <= appContent.length()) {
            int aidLen = std::stoul(appContent.substr(aidPos + 2, 2), nullptr, 16) * 2;
            app.aid = appContent.substr(aidPos + 4, aidLen);
        }

        // 2. Ekstrak Tag 50 (Application Label)
        size_t labelPos = appContent.find("50");
        if (labelPos != std::string::npos && labelPos + 4 <= appContent.length()) {
            int labelLen = std::stoul(appContent.substr(labelPos + 2, 2), nullptr, 16) * 2;
            std::string labelHex = appContent.substr(labelPos + 4, labelLen);
            app.label = hexToAscii(labelHex);
        }

        // 3. Ekstrak Tag 87 (Application Priority Indicator)
        size_t prioPos = appContent.find("8701");
        if (prioPos != std::string::npos && prioPos + 6 <= appContent.length()) {
            app.priority = std::stoul(appContent.substr(prioPos + 4, 2), nullptr, 16);
        }

        appList.push_back(app);
        pos += 4 + lenChars; // Pindah ke template berikutnya
    }

    if (appList.empty()) {
        std::cout << "Tidak ada aplikasi (AID) yang ditemukan." << std::endl;
        return "";
    }

    // Urutkan berdasarkan prioritas terkecil (01 adalah prioritas tertinggi)
    std::sort(appList.begin(), appList.end(), [](const Application& a, const Application& b) {
        return a.priority < b.priority;
    });

    // Ambil aplikasi terbaik
    Application bestApp = appList[0];

    // Format panjang AID ke dalam Hex 1 Byte (2 Karakter)
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << std::hex << (bestApp.aid.length() / 2);
    std::string aidLenHex = ss.str();

    // Susun APDU Select: CLA(00) INS(A4) P1(04) P2(00) + Lc + AID + Le(00)
    std::string apduSelect = "00A40400" + aidLenHex + bestApp.aid + "00";

    // Transformasi hasil ke Upper Case agar rapi
    std::transform(apduSelect.begin(), apduSelect.end(), apduSelect.begin(), ::toupper);

    // Cetak Hasil
    std::cout << "=== HASIL SELEKSI APLIKASI ===" << std::endl;
    std::cout << "Nama Aplikasi   : " << bestApp.label << std::endl;
    emv.DedicatedFile = bestApp.aid;
    std::cout << "AID Terpilih    : " << emv.DedicatedFile << std::endl;
    std::cout << "Nilai Prioritas : " << bestApp.priority << std::endl;
    std::cout << "👉 APDU SELECT  : " << apduSelect << std::endl;
    return apduSelect;
}


CommandCardCRT::CommandCardCRT() {

}

CommandCardCRT::~CommandCardCRT() {
  if(is_open){
    Close();
  }
}

std::string getTime(){
  std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    char buffer[80];
    // Format: YYYY-MM-DD (Contoh: 2026-05-19)
    std::strftime(buffer, sizeof(buffer), "%Y%m%d", now);
    std::string tanggal_str(buffer);
    return tanggal_str;
}

std::string NewgetTime(){
  std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    char buffer[80];
    // Format: YYYY-MM-DD (Contoh: 2026-05-19)
    std::strftime(buffer, sizeof(buffer), "%y%m%d", now);
    std::string tanggal_str(buffer);
    return tanggal_str;
}


std::string genRandom(){
  std::random_device rd;
    std::mt19937 gen(rd());

    // Kumpulan karakter: huruf kecil, huruf besar, dan angka
    std::string kumpulan_karakter = "ABCDEF0123456789";
    
    // Rentang indeks dari 0 sampai panjang string - 1
    std::uniform_int_distribution<int> distrib(0, kumpulan_karakter.length() - 1);

    std::string hasil_acak = "";
    int panjang_string = 8; // Atur panjang string sesuai kebutuhan

    for (int i = 0; i < panjang_string; ++i) {
        hasil_acak += kumpulan_karakter[distrib(gen)];
    }
    return hasil_acak;
}

bool CommandCardCRT::Open(std::string_view com, int bautrate) {
  unsigned int Baudrate = 115200;
  if(bautrate<0)bautrate=Baudrate;
  buf = CRT310NROpenWithBaut((char*)com.data(), bautrate);
  if (buf != NULL) {
    is_open = true;
  }
  if(is_open){
    m_lastcom = com;
  }
  return is_open;
}

void CommandCardCRT::Close() {
  if (buf) CRT310NRClose(buf);

  is_open = false;
}

std::string CommandCardCRT::EncodeMagDataStatus(uint8_t *status)
{
  unsigned char CmCode;
	unsigned char PmCode;
  int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x36;
	PmCode=0x37;
	CmDataLen=0;
  
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  
  std::string ret;
  if (rc == 0) {
    if (ReType == 0x50 &&
        status) {
      std::bitset<8> bit_stat;
      switch(ReData[0])
		   {
			   case '0':
				   ret +="ISO #1 is not encoded."; 
           bit_stat.set(0,false);
				   break;
			   case '1':
				   ret +="ISO #1 is encoded."; 
           bit_stat.set(0);
		 		   break;
		   }  
		   switch(ReData[1])
		   {
			   case '0':
				   ret+=",ISO #1 is not encoded.";
           bit_stat.set(1,false);
				   break;
			   case '1':
				   ret +=",ISO #1 is encoded.";
           bit_stat.set(1);
		 		   break;
		   }  
		   switch(ReData[2])
		   {
			   case '0':
				   ret +=",ISO #1 is not encoded.";
           bit_stat.set(2,false);
				   break;
			   case '1':
				   ret +=",ISO #1 is encoded.";
           bit_stat.set(2);
		 		   break;
		   }  
		   switch(ReData[3])
		   {
			   case '0':
				   ret +=",JIS II is not encoded.";
           bit_stat.set(3,false);
				   break;
			   case '1':
				   ret +=",JIS II is encoded.";
           bit_stat.set(3);
		 		   break;
		   }
       *status =(uint8_t) bit_stat.to_ulong(); 
    } else if (ReType == 0x4e) {
      std::cout << "\n0x4e";
    } else {
      std::cout << "\ncom err";
    }
  }
  return ret;
}

CardData CommandCardCRT::ReadCard(int timeout,int *status)
{
  CardData data;
  if(!is_open){
    Open(m_lastcom,m_lastbautrate);
    if(!is_open){
      if(status){
        *status = -1;
      }
      return data;
    }
  }
  bool ret = Init(true);
  if(!ret){
    if(status){
      *status = -2;
    }
    return data;
  }
  ret = PermitMagCardOnly();
  if(!ret){
    if(status){
      *status = -3;
    }
    return data;
  }
  ret = LEDBlinkGreen();
  if(!ret) {
    if(status){
      *status = -4;
    }
    return data;
  }
  int card = -1;
  int m_max_try = timeout > 0 ? timeout : 10;
  while(card != 2 &&
        m_max_try > 0){
    card = CardStatus(); 
    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_max_try -= 1;
  }
  ret = LEDOFF();

  uint8_t status_mag = 0;
  auto statdata = EncodeMagDataStatus(&status_mag);
  std::bitset<8> bit_mag = status_mag;
  if(bit_mag.test(0)){
    data.track1 = ReadData(1);
  }
  if(bit_mag.test(1)){
    data.track2 = ReadData(2);
  }
  if(bit_mag.test(2)){
    data.track3 = ReadData(3);
  }
  return data;
}

int CommandCardCRT::RejectCardOrMoveBin(int timeout) {
  int ret = -1;
  if (!is_open) {
    return ret;
  }
  ret = -2;
  int card = CardStatus();
  if (card == 2) {
    ret = 0;
    Eject();
    LEDBlinkOrange();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    int max_try = timeout > 0 ? timeout : 10;
    int expect = 2;
    while ((expect == 2 || expect == 1) && max_try > 0) {
      expect = CardStatus();
      std::this_thread::sleep_for(std::chrono::seconds(1));
      max_try -= 1;
    }

    if (expect == 1) {
      ret = 1;
      Retrieve();
      LEDOFF();

      int max_loop = 15;
      expect = 0;
      while (expect != 2 && max_loop > 0) {
        expect = CardStatus();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        max_loop -= 1;
      }

      if (expect == 2) {
        Capture();
      }
    }
  }
  ProhibitCardIn();
  LEDOFF();
  return ret;
}

std::string CommandCardCRT::ReadData(uint8_t index)
{
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x36;
  switch(index){
    case 1:
      PmCode = 0x31;
      break;
    case 2: 
      PmCode = 0x32;
      break;
    case 3: 
      PmCode = 0x33;
      break;
    default: 
      PmCode = 0x32;
      break;
  }
  CmDataLen = 0;
  std::string ret;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return std::string((const char*)ReData,ReDataLen);
    } else if (ReType == 0x4e) {
      std::cout << "\n0x4e";
    } else {
      std::cout << "\ncom err";
    }
  }
  return ret;
}

int CommandCardCRT::CardStatus()
{
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x31;
  PmCode = 0x30;
  CmDataLen = 0;
  int ret = -1;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    //  std::cout << "ReType:"<<static_cast<unsigned int>(ReType) <<"|"<<ReType<< std::endl;
    //  std::cout << "SEt1:"<<static_cast<unsigned int>(SEt0) <<"|"<<SEt0<< std::endl;
    //  std::cout << "SEt2:"<<static_cast<unsigned int>(SEt1) <<"|"<<SEt1<< std::endl;
    if (ReType == 0x50) {
      switch(SEt1)
		  {
			   case '0':
         // std::cout << "SEt1:"<<"0"<< std::endl;
				   //Str1="No card detected within ICRW (including card gate)";
           ret = 0;
         break;
			   case '1':
          //std::cout << "SEt1:"<<"1"<< std::endl;
           ret = 1;
				   //Str1="Card locates at card Gate";
         break;
			   case '2':
            //std::cout << "SEt1:"<<"2"<< std::endl;
           ret = 2;
				   //Str1="Card locates inside ICRW (Transport)";
         break;
		  }
      std::cout << "ret:"<<ret<< std::endl;
      return ret;
    } else if (ReType == 0x4e) {
      std::cout << "\n0x4e";
    } else {
      std::cout << "\ncom err";
    }
    std::cout << "ret end:"<<ret<< std::endl;
  }
  return ret;
}

// Fungsi pembantu untuk mengubah angka int menjadi string hex 2 karakter (contoh: 29 -> "1D")
std::string intToHexStr(int value) {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex << value;
    return ss.str();
}

// FUNGSI UTAMA: Menyusun DATA PAYLOAD CDOL dalam bentuk string secara dinamis
std::string buildDynamicCdolString(const std::string& cdolFormat, const std::map<std::string, std::string>& txData) {
    std::string cdolPayloadStr = "";
    size_t i = 0;

    while (i < cdolFormat.length()) {
        std::string tag = "";
        
        // 1. Identifikasi Panjang Tag EMV (1 byte atau 2 bytes)
        if ((cdolFormat[i + 1] == 'F' || cdolFormat[i + 1] == 'f') && (i + 4 <= cdolFormat.length())) {
            tag = cdolFormat.substr(i, 4);
            i += 4;
        } else {
            tag = cdolFormat.substr(i, 2);
            i += 2;
        }

        // 2. Ambil Panjang Data (Length) dalam satuan byte
        std::string lenStr = cdolFormat.substr(i, 2);
        int lengthInBytes = (int)strtol(lenStr.c_str(), nullptr, 16);
        i += 2;

        // 3. Ambil data dari database terminal berdasarkan Tag
        std::string dataHex = "";
        if (txData.find(tag) != txData.end()) {
            dataHex = txData.at(tag);
        }

        // 4. Validasi panjang (Auto padding nol jika kurang, potong jika lebih)
        int targetLengthChars = lengthInBytes * 2;
        if (dataHex.length() < targetLengthChars) {
            dataHex = std::string(targetLengthChars - dataHex.length(), '0') + dataHex;
        } else if (dataHex.length() > targetLengthChars) {
            dataHex = dataHex.substr(0, targetLengthChars);
        }

        // 5. Gabungkan ke payload
        cdolPayloadStr += dataHex;
    }
    return cdolPayloadStr;
}

// Fungsi helper konversi Hex String ke Byte Array
std::vector<unsigned char> CommandCardCRT::hexToBytes(const std::string& hex) {
    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        bytes.push_back(static_cast<unsigned char>(strtol(hex.substr(i, 2).c_str(), nullptr, 16)));
    }
    return bytes;
}

// Fungsi untuk menghitung panjang dan membuat format TLV
std::string generateLengEMVTLV(const std::string& tag, std::string valueHex) {
    // Hapus spasi jika ada input yang mengandung spasi
    valueHex.erase(std::remove(valueHex.begin(), valueHex.end(), ' '), valueHex.end());

    // Hitung jumlah byte (2 karakter hex = 1 byte)
    size_t byteLength = valueHex.length() / 2;

    // Konversi panjang byte ke format hex string 2 digit (Uppercase)
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << byteLength;
    std::string lengthHex = ss.str();

    // Gabungkan menjadi format TLV
    std::string fullTLV = tag + " " + lengthHex +" "+ valueHex;

    // // Tampilkan hasil analisis di konsol
    // std::cout << "--- Hasil Analisis Tag " << tag << " ---\n";
    // std::cout << "Data Asli (Value) : " << valueHex << "\n";
    // std::cout << "Panjang (Decimal) : " << byteLength << " byte\n";
    // std::cout << "Panjang (Hex)     : " << lengthHex << "\n";
    // std::cout << "Format TLV Utuh   : " << fullTLV << "\n\n";

    return fullTLV;
}

// Fungsi untuk menghitung panjang dan membuat format TLV
std::string generateLengEMVTLVEND(const std::string& tag, std::string valueHex) {
    // Hapus spasi jika ada input yang mengandung spasi
    valueHex.erase(std::remove(valueHex.begin(), valueHex.end(), ' '), valueHex.end());

    // Hitung jumlah byte (2 karakter hex = 1 byte)
    size_t byteLength = valueHex.length() / 2;

    // Konversi panjang byte ke format hex string 2 digit (Uppercase)
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << byteLength;
    std::string lengthHex = ss.str();

    // Gabungkan menjadi format TLV
    std::string fullTLV = tag + lengthHex + valueHex;

    // // Tampilkan hasil analisis di konsol
    // std::cout << "--- Hasil Analisis Tag " << tag << " ---\n";
    // std::cout << "Data Asli (Value) : " << valueHex << "\n";
    // std::cout << "Panjang (Decimal) : " << byteLength << " byte\n";
    // std::cout << "Panjang (Hex)     : " << lengthHex << "\n";
    // std::cout << "Format TLV Utuh   : " << fullTLV << "\n\n";

    return fullTLV;
}

int CommandCardCRT::GenerateACCommand(std::string formatCdol){

    emv.unpredictableNo = genRandom();
    emv.transactionDate = NewgetTime();
    
    // if(emv.ApplicationCurrencyExponent != "0"){
    //   std::string depan = emv.ammountDeposit.substr(0, 2);
    //   // Mengambil sisa karakter setelahnya ("0000030000")
    //   std::string sisa = emv.ammountDeposit.substr(2);
    //   // Menggabungkan sisa karakter dengan bagian depan di akhir
    //   std::string hasil = sisa + depan;
    //   emv.ammountDeposit = hasil;
    //   std::cout <<" berhasil update ammount "<<emv.ammountDeposit <<std::endl;
    // }
  // 1. Gudang Data Terminal
    std::map<std::string, std::string> terminalData;
    terminalData["9F02"] = emv.ammountDeposit; // Amount Auth
    terminalData["9F03"] = emv.ammountOther; // Amount Other
    terminalData["9F1A"] = emv.terminalCountry;         // Country Code
    terminalData["95"]   = emv.TerminalVerificationResults;   // TVR
    terminalData["5F2A"] = emv.currencyCode;         // Currency Code
    terminalData["9A"]   = emv.transactionDate;       // Date (YYMMDD)
    terminalData["9C"]   = emv.transactionType;           // Trans Type
    terminalData["9F37"] = emv.unpredictableNo;     // Unpredictable Number

    // CDOL1 format yang kamu miliki
    std::string cdolFormat = formatCdol;//"9F02069F03069F1A0295055F2A029A039C019F3704";

    // 2. Dapatkan string isi data CDOL secara dinamis
    std::string cdolPayloadData = buildDynamicCdolString(cdolFormat, terminalData);

    // 3. Susun APDU Header (dalam bentuk string)
    std::string cla = "80";
    std::string ins = "AE";
    std::string p1  = "40"; // Meminta ARQC 80 offline(digunakan untuk tol parkir ofline) 40 online
    std::string p2  = "00";

    // P1 = 0x80 (ARQC): Meminta kriptogram untuk transaksi Online (Paling umum digunakan terminal Visa untuk meminta otentikasi ke bank penerbit).
    // P1 = 0x40 (TC): Meminta kriptogram untuk persetujuan transaksi Offline.
    // P1 = 0x00 (AAC): Menolak transaksi secara Offline.
    // P1 = 0x90 (ARQC + CDA) atau P1 = 0x50 (TC + CDA): Jika terminal mendukung Combined Data Authentication (CDA), bit ke-5 pada P1 diaktifkan (OR 0x10). Kartu Visa modern sering kali meminta kombinasi ini untuk aspek keamanan ekstra dari pemalsuan kartu fisik.
    
    // Hitung Lc (Panjang payload data dalam byte = jumlah karakter / 2)
    int lcValue = cdolPayloadData.length() / 2;
    std::string lc = intToHexStr(lcValue);
    
    std::string le = "00"; // Request all response bytes

    // 4. Gabungkan semua menjadi satu String APDU Command
    //std::string apduCmd = cla + ins + p1 + p2 + lc + cdolPayloadData + le;
    std::string apduCmd = cla + ins + p1 + p2 + lc + cdolPayloadData + le;

    // Supaya rapi saat dicetak (opsional: ubah ke uppercase)
    std::transform(apduCmd.begin(), apduCmd.end(), apduCmd.begin(), ::toupper);

    // Output hasil akhir
    std::cout << "CDOL Payload Data : " << cdolPayloadData << std::endl;
    std::cout << "Lc (Hex Length)   : " << lc << " (" << lcValue << " bytes)" << std::endl;
    std::cout << "Full APDU Command : " << apduCmd << std::endl;

    unsigned char sw1 = 0, sw2 = 0;
      // Panggil fungsi utama RS232 Anda
      std::string responseHex = CommandData(apduCmd, &sw1, &sw2); 
      unsigned int sw = (sw1 << 8) | sw2;

      if (sw == 0x9000 || sw1 == 0x61) {
          // Hilangkan status word 9000 di ekor string jika ada sebelum parsing murni TLV
          if (responseHex.length() >= 4 && responseHex.substr(responseHex.length() - 4) == "9000") {
              responseHex = responseHex.substr(0, responseHex.length() - 4);
          }

          std::vector<unsigned char> responseBytes = hexToBytes(responseHex);

          // Tembak ke parser rekursif untuk diekstrak secara otomatis
          parseEMVTLV(responseBytes);
          emv.transactionDate = NewgetTime();
          if(emv.ApplicationVersionNumber == "01"){
            emv.ApplicationVersionNumber = "0001";
          }
          emv.CardholderVerificationMethod = "020002";
          emv.TerminalCapabilities = "604021";
          emv.TerminalType = "21";

          // if(emv.iad.length() < 50){
          //   std::string transisi = emv.iad;
          //   emv.iad = generateIad28Bytes(transisi);
          //    std::cout << "\n=====>iad sukses jadi sepanjang " << emv.iad.length() << " | "<< emv.iad << std::endl;
          // }

          
            // if(emv.pan == "4616994800000178"){
            //     emv.iad = "0101A00000000023EA42F10000000000000000000000000000000000";
            // }

            // if(emv.pan == "4617006000000509"){
            //   emv.iad = "0101A000000000892A52BE0000000000000000000000000000000000";
            // }

                      // --- CETAK HASIL AKHIR PEMBACAAN KIOSK ---
          std::cout << "\n==========================================" << std::endl;
          std::cout << "        HASIL DATA KARTU EMV KIOSK        " << std::endl;
          std::cout << "==========================================" << std::endl;
          std::cout << "================TRACK2=====================" << std::endl;
          std::cout << "Nomor Kartu (PAN)  : " << emv.pan << std::endl;
          std::cout << "Expired Date (YYMM): " << emv.expiryDate << std::endl;
          std::cout << "card holder Name   : " << emv.cardholderName << std::endl;
          std::cout << "service Code       : " << emv.serviceCode << std::endl;
          std::cout << "discretionaryData  : " << emv.discretionaryData << std::endl;
          std::cout << "Final Post Track2  : " << emv.track2Data << std::endl;
          std::cout << "==========================================" << std::endl;
          std::cout << "ammountDeposit     :" << emv.ammountDeposit <<std::endl;
          std::cout << "ammountOther       :" << emv.ammountOther <<std::endl;
          std::cout << "Tag 9F1A terminalCo: " << emv.terminalCountry << std::endl;
          std::cout << "Tag 95 tvr         : " << emv.tvr << std::endl;
          std::cout << "Tag 5F2A currencyCo: " << emv.currencyCode << std::endl;
          std::cout << "Tag 9A trans Date  : " << emv.transactionDate << std::endl;
          std::cout << "Tag 9C trans Type  : " << emv.transactionType << std::endl;
          std::cout << "unpredictable No   : " << emv.unpredictableNo << std::endl;
          std::cout << "===================ICC PACKET=====================" << std::endl;
          std::cout << "arqc               :"<<generateLengEMVTLV("9F26",emv.arqc)<< /*emv.arqc << */std::endl;// Tag 9F26
          std::cout << "cid                :"<< generateLengEMVTLV("9F27",emv.cid)<</*emv.cid << */std::endl; // Tag 9F27
          std::cout << "iad                :"<<generateLengEMVTLV("9F10",emv.iad)<< /*emv.iad << */std::endl; // Tag 9F10
          std::cout << "atc                :"<< generateLengEMVTLV("9F36",emv.atc)<</*emv.atc << */std::endl; // Tag 9F36
          std::cout << "unpredictableNo    :"<< generateLengEMVTLV("9F37",emv.unpredictableNo)<</*emv.unpredictableNo << */std::endl;//generate dari kita 9F37
          std::cout << "TerminalVerificationResults:"<<  generateLengEMVTLV("95",emv.TerminalVerificationResults)<</*emv.TerminalVerificationResults << */std::endl; //Tag 95 hasil verivikasi
          std::cout << "transactionDate    :"<< generateLengEMVTLV("9A",emv.transactionDate)<</*emv.transactionDate << */std::endl; // tag 9A (YYMMDD)
          std::cout << "transactionType    :"<< generateLengEMVTLV("9C",emv.transactionType)<</*emv.transactionType << */std::endl; //00 asumsi purcase butuh info dari mandiri mau transaksi tipenya apa //tag 9C
          std::cout << "ammountDeposit     :"<< generateLengEMVTLV("9F02",emv.ammountDeposit)<</*emv.ammountDeposit << */std::endl; //Tag 9F02
          std::cout << "ammountOther       :"<< generateLengEMVTLV("9F03",emv.ammountOther)<</*emv.ammountOther << */std::endl; //Tag 9F03
          std::cout << "pan                :"<< generateLengEMVTLV("5A",emv.pan)<</*emv.pan << */std::endl; //5A
          std::cout << "currencyCode       :"<< generateLengEMVTLV("5F2A",emv.currencyCode)<</*emv.currencyCode << */std::endl; //tag 5F2A
          std::cout << "ApplicationInterchangeProfile:"<< generateLengEMVTLV("82",emv.ApplicationInterchangeProfile)<</*emv.ApplicationInterchangeProfile << */std::endl; //Tag 82
          std::cout << "terminalCountry    :"<< generateLengEMVTLV("9F1A",emv.terminalCountry)<</*emv.terminalCountry << */std::endl;//Tag 9F1A
          std::cout << "panSequenceNumber  :"<< generateLengEMVTLV("5F34",emv.panSequenceNumber)<</*emv.panSequenceNumber << */std::endl;        // Tag 5F34
          std::cout << "TerminalCapabilities:"<< generateLengEMVTLV("5F33",emv.TerminalCapabilities)<</*emv.TerminalCapabilities << */std::endl; //9F33
          std::cout << "CardholderVerificationMethod:"<< generateLengEMVTLV("9F34",emv.CardholderVerificationMethod)<</*emv.CardholderVerificationMethod << */std::endl;
          std::cout << "TerminalType       :"<< generateLengEMVTLV("9F35",emv.TerminalType)<</*emv.TerminalType << */std::endl;
          std::cout << "InterfaceDevice    :"<< generateLengEMVTLV("9F1E",emv.InterfaceDevice)<</*emv.InterfaceDevice << */std::endl;
          std::cout << "DedicatedFile      :"<< generateLengEMVTLV("84",emv.DedicatedFile)<</*emv.DedicatedFile << */std::endl;
          std::cout << "ApplicationVersionNumber:"<< generateLengEMVTLV("9F09",emv.ApplicationVersionNumber)<</*emv.ApplicationVersionNumber << */std::endl;
          std::cout << "TransactionSequenceCounter:"<< generateLengEMVTLV("9F41",emv.TransactionSequenceCounter)<</*emv.TransactionSequenceCounter <<*/ std::endl;
          std::cout << "Application Currency Exponent:"<< generateLengEMVTLV("9F44",emv.ApplicationCurrencyExponent)<<std::endl;
          std::cout << "==========================================" << std::endl;
          emv.IccFull = generateLengEMVTLVEND("9F26",emv.arqc);//ada
          emv.IccFull += generateLengEMVTLVEND("9F27",emv.cid);//ada
          
          if(emv.iad != "Tidak Ditemukan") {
            emv.IccFull += generateLengEMVTLVEND("9F10",emv.iad);//ada
          }
          
          emv.IccFull += generateLengEMVTLVEND("9F36",emv.atc);//ada
          emv.IccFull += generateLengEMVTLVEND("9F37",emv.unpredictableNo);//ada
          emv.IccFull += generateLengEMVTLVEND("95",emv.TerminalVerificationResults);//ada
          emv.IccFull += generateLengEMVTLVEND("9A",emv.transactionDate);//ada
          emv.IccFull += generateLengEMVTLVEND("9C",emv.transactionType);//ada
          emv.IccFull += generateLengEMVTLVEND("9F02",emv.ammountDeposit);//ada
          emv.IccFull += generateLengEMVTLVEND("9F03",emv.ammountOther);//ada
          emv.IccFull += generateLengEMVTLVEND("5A",emv.pan);//ini yang hilang
          emv.IccFull += generateLengEMVTLVEND("5F2A",emv.currencyCode);//ada
          emv.IccFull += generateLengEMVTLVEND("82",emv.ApplicationInterchangeProfile);//ada
          emv.IccFull += generateLengEMVTLVEND("9F1A",emv.terminalCountry);//ada
          emv.IccFull += generateLengEMVTLVEND("5F34",emv.panSequenceNumber);//ada
          emv.IccFull += generateLengEMVTLVEND("9F33",emv.TerminalCapabilities);//ada kosong
          emv.IccFull += generateLengEMVTLVEND("9F34",emv.CardholderVerificationMethod);//ada kosong
          emv.IccFull += generateLengEMVTLVEND("9F35",emv.TerminalType);
          emv.IccFull += generateLengEMVTLVEND("9F1E",emv.InterfaceDevice);//ini yang hilang
          emv.IccFull += generateLengEMVTLVEND("84",emv.DedicatedFile);//ini yang hilang
          emv.IccFull += generateLengEMVTLVEND("9F09",emv.ApplicationVersionNumber);
          emv.IccFull += generateLengEMVTLVEND("9F41",emv.TransactionSequenceCounter);//ini yang hilang
      } else {
          std::cerr << "[LEWAT] Record merespon error / kosong. SW: " << std::hex << sw << std::dec << std::endl;
      }
}

bool CommandCardCRT::IsOpen() { return is_open; }

bool CommandCardCRT::Init(bool EjectFrontOrRear) {
  bool ok = false;
  emv.cdol1Raw = "Tidak Ditemukan"; 
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x30;
  if(EjectFrontOrRear){ 
    PmCode = 0x31;//init with capture buang ke blkng
  }else{
    PmCode = 0x30; //buang ke depan
  }
  CmDataLen = 13;
  CmData[0] = 0x33;
  CmData[1] = 0x32;
  CmData[2] = 0x34;
  CmData[3] = 0x31;
  CmData[4] = 0x30;
  CmData[5] = 0x30;
  CmData[6] = 0x30;
  CmData[7] = 0x30;
  CmData[8] = 0x30;
  CmData[9] = 0x30;
  CmData[10] = 0x30;
  CmData[11] = 0x30;
  CmData[12] = 0x30;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      ok = true;
    } else if (ReType == 0x4e) {
      // todo log kode error set0/set1
      std::cout << "\ninit 0x4e";
    }
  } else {
    std::cout << "\nrc " << rc;
  }
  return ok;
}

bool CommandCardCRT::PermitMagCardOnly() {
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x3a;
  PmCode = 0x30;
  CmDataLen = 1;
  CmData[0] = 0x31;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
      std::cout << "\n0x4e";
    } else {
      std::cout << "\ncom err";
    }
  }
  return false;
}
bool CommandCardCRT::ProhibitCardIn() {
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x3a;
  PmCode = 0x31;
  CmDataLen = 0;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}

bool CommandCardCRT::LEDBlinkGreen()
{
unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x35;
  PmCode = 0x31;
  CmDataLen = 4;
  CmData[0] = 0x31;
  CmData[1] = 0x30;
  CmData[2] = 0x31;
  CmData[3] = 0x30;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}

bool CommandCardCRT::LEDBlinkOrange()
{
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x35;
  PmCode = 0x33;
  CmDataLen = 4;
  CmData[0] = 0x31;
  CmData[1] = 0x30;
  CmData[2] = 0x31;
  CmData[3] = 0x30;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}

bool CommandCardCRT::LEDBlinkRed()
{
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x35;
  PmCode = 0x32;
  CmDataLen = 4;
  CmData[0] = 0x31;
  CmData[1] = 0x30;
  CmData[2] = 0x31;
  CmData[3] = 0x30;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}

bool CommandCardCRT::LEDOFF()
{
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x35;
  PmCode = 0x30;
  CmDataLen = 0;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}

bool CommandCardCRT::Eject() {
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];

  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  CmCode = 0x33;
  PmCode = 0x30;
  CmDataLen = 0;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}
bool CommandCardCRT::Capture() {
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];
  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  memset(ReData, 0x00, sizeof(ReData));

  CmCode = 0x33;
  PmCode = 0x31;
  CmDataLen = 0;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false;
}
bool CommandCardCRT::Retrieve() { 
  unsigned char CmCode;
  unsigned char PmCode;
  int CmDataLen;
  unsigned char CmData[1024];
  unsigned char ReType;

  unsigned char SEt0;
  unsigned char SEt1;
  int ReDataLen;
  unsigned char ReData[1024];

  memset(CmData, 0x00, sizeof(CmData));
  memset(ReData, 0x00, sizeof(ReData));

  CmCode = 0x34;
  PmCode = 0x30;
  CmDataLen = 0;
  int rc = RS232_ExeCommand(buf, CmCode, PmCode, CmDataLen, CmData, &ReType,
                            &SEt0, &SEt1, &ReDataLen, ReData);
  if (rc == 0) {
    if (ReType == 0x50) {
      return true;
    } else if (ReType == 0x4e) {
    } else {
    }
  }
  return false; 
}

CardData CommandCardCRT::CheckMAgData(){
  CardData data;
  uint8_t status_mag = 0;
  auto statdata = EncodeMagDataStatus(&status_mag);
  std::bitset<8> bit_mag = status_mag;
  if(bit_mag.test(0)){
    data.track1 = ReadData(1);
    std::cout <<"TRACK1="<<data.track1 <<std::endl;
  }
  if(bit_mag.test(1)){
    data.track2 = ReadData(2);
    std::cout <<"TRACK2="<<data.track2<<std::endl;
  }
  if(bit_mag.test(2)){
    data.track3 = ReadData(3);
    std::cout <<"TRACK3="<<data.track3<<std::endl;
  }
  return data;
}

void CommandCardCRT::GetMagCardStatusBtn() 
{
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x36;
	PmCode=0x37;
	CmDataLen=0;

	int	rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt1,&ReDataLen,ReData);	
	if(rc==0)
	{
		if (ReType==0x50)
		{
		   std::string Str1,Str2,Str3,Str4,MsgBuf;
		   switch(ReData[0])
		   {
			   case '0':
				   Str1="ISO #1 is not encoded.";
				   break;
			   case '1':
				   Str1="ISO #1 is encoded.";
		 		   break;
		   }  
		   switch(ReData[1])
		   {
			   case '0':
				   Str2="ISO #1 is not encoded.";
				   break;
			   case '1':
				   Str2="ISO #1 is encoded.";
		 		   break;
		   }  
		   switch(ReData[2])
		   {
			   case '0':
				   Str3="ISO #1 is not encoded.";
				   break;
			   case '1':
				   Str3="ISO #1 is encoded.";
		 		   break;
		   }  
		   switch(ReData[3])
		   {
			   case '0':
				   Str4="JIS II is not encoded.";
				   break;
			   case '1':
				   Str4="JIS II is encoded.";
		 		   break;
		   }  
			
			MsgBuf = "Execute OK and status code: ";
			MsgBuf += "\n"+Str1+"\n"+Str2+"\n"+Str3+"\n"+Str4;
			std::cout << MsgBuf; 
		}
		else if  (ReType==0x4e)
		{
			std::cout << "ERROR"; 
		}
		else
		{
			std::cout << "Communication Error"; 
		}
	}
	else 
		std::cout << "Communication Error"; 	
}

void CommandCardCRT::OnICTestTypeBtn() 
{
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x90;
	PmCode=0x30;
	CmDataLen=0;

	int	rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt1,&ReDataLen,ReData);		
	if(rc==0)
	{
		if (ReType==0x50)
		{
			// CString t,MsgBuf;
			// t.Format("%c%c",ReData[0],ReData[1]);
			// MsgBuf="Card Code: ";
			// MsgBuf += t;
			switch(ReData[0])
			{
				 case '0':
					  switch(ReData[1])
					  {
						   case '0':
                std::cout <<"Unknow Type";
                  //MessageBox(MsgBuf+"\nUnknow Type", "Auto Test IC Card Type",MB_OK);
							   break;
					  }  
					  break;

				 case '1':
					  switch(ReData[1])
					  {
						   case '0':
               std::cout <<"T=0 CPU Card";
							   //MessageBox(MsgBuf+"\nT=0 CPU Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '1':
                std::cout <<"T=1 CPU Card";
							   //MessageBox(MsgBuf+"\nT=1 CPU Card", "Auto Test IC Card Type",MB_OK);
							   break;
					  }  
					  break;
				 case '2':
					  switch(ReData[1])
					  {
						   case '1':
               std::cout <<"24C01 Card";
							   //MessageBox(MsgBuf+"\n24C01 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '2':
               std::cout <<"24C02 Card";
							  //  MessageBox(MsgBuf+"\n24C02 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '3':
               std::cout <<"24C04 Card";
							  //  MessageBox(MsgBuf+"\n24C04 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '4':
               std::cout <<"24C08 Card";
							  //  MessageBox(MsgBuf+"\n24C08 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '5':
               std::cout <<"24C16 Card";
							  //  MessageBox(MsgBuf+"\n24C16 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '6':
               std::cout <<"24C32 Card";
							  //  MessageBox(MsgBuf+"\n24C32 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '7':
               std::cout <<"24C64 Card";
							  //  MessageBox(MsgBuf+"\n24C64 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '8':
               std::cout <<"24C128 Card";
							  //  MessageBox(MsgBuf+"\n24C128 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '9':
               std::cout <<"24C256 Card";
							  //  MessageBox(MsgBuf+"\n24C256 Card", "Auto Test IC Card Type",MB_OK);
							   break;
					  }  
					  break;
				 case '3':
					  switch(ReData[1])
					  {
						   case '1':
               std::cout <<"SLE4442 Card";
							  //  MessageBox(MsgBuf+"\nSLE4442 Card", "Auto Test IC Card Type",MB_OK);
							   break;
						   case '2':
               std::cout <<"SLE4428 Card";
							  //  MessageBox(MsgBuf+"\nSLE4428 Card", "Auto Test IC Card Type",MB_OK);
							   break;
					  }  
					  break;
				 default:
					 break;
			}
		}
		else if  (ReType==0x4e)
		{
      std::cout <<"Error Card";
			// CString t,MsgBuf;
			// t.Format("%c%c",SEt0,SEt1);
			// MsgBuf="Error Code: ";
			// MsgBuf += t;
			// MessageBox(MsgBuf,MB_OK,0); 
		}
		else
		{
      std::cout <<"Error Communication";
			//MessageBox("Communication Error"); 
		}
	}
	else 
       std::cout <<"Error Communication";
		// MessageBox("Communication Error"); 
}

void CommandCardCRT::OnFrontEnterCardBySwitchBtn() 
{
	// if (hCom==0)
	// {
	// 	MessageBox("Please Open Comm. port first");
	// 	return;
	// }
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x3a;
	PmCode=0x30;
	CmDataLen=1;
	CmData[0]=0x30;

  int	rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt1,&ReDataLen,ReData);	
	if(rc==0)
	{
		if (ReType==0x50)
		{
      std::cout <<"Execute Ok     ..";
		}
		else if  (ReType==0x4e)
		{
      std::cout <<"Error code"; 
		}
		else
		{
      std::cout <<"Error Communication";
		}
	}
	else 
    std::cout <<"Error Communication"; 
}

void CommandCardCRT::OnFrontNoCardEnterBtn() 
{
	// if (hCom==0)
	// {
	// 	MessageBox("Please Open Comm. port first");
	// 	return;
	// }
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x3a;
	PmCode=0x31;
	CmDataLen=0;

		int rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt1,&ReDataLen,ReData);	
	if(rc==0)
	{
		if (ReType==0x50)
		{
      std::cout <<"Execute Ok     ..";
		}
		else if  (ReType==0x4e)
		{
      std::cout <<"Error code"; 
		}
		else
		{
      std::cout <<"Error Communication";
		}
	}
	else 
    std::cout <<"Error Communication";
}


int CommandCardCRT::gethexvalue(char p)
{
	switch(p)
	{
	case '0': return 0; 
	case '1': return 1;
	case '2': return 2; 
	case '3': return 3;
	case '4': return 4; 
	case '5': return 5;
	case '6': return 6; 
	case '7': return 7;
	case '8': return 8; 
	case '9': return 9;
	case 'a': 
	case 'A': return 10;
	case 'b':  
	case 'B': return 11;
	case 'c': ; 
	case 'C': return 12;
	case 'd':  
	case 'D': return 13;
	case 'e': 
	case 'E': return 14;
	case 'f': 
	case 'F': return 15;
	};
	return -1;
}

int CommandCardCRT::GetDECData(uint8_t* pData, std::string_view str) {
    // Validasi panjang string: Minimal harus ada 32 karakter (16 byte * 2 char per hex)

    for (size_t i = 0; i < 16; ++i) {
        int high = gethexvalue(str[i * 2]);
        int low  = gethexvalue(str[i * 2 + 1]);

        if (high == -1 || low == -1) {
            return 2; // Error: Karakter bukan hex
        }

        // Operasi bitwise (|) lebih cepat daripada penjumlahan (+)
        pData[i] = static_cast<uint8_t>((high << 4) | low);
    }

    return 0; // Success
}

void CommandCardCRT::OnCPUAPDUSENDButton(std::string datass) 
{
	m_APDUSENDSTRINGCMD = datass;
	unsigned char buft[1024];
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	// 1. Validasi Input menggunakan std::string_view
if (m_APDUSENDSTRINGCMD.empty()) {
    std::cerr << "Error: APDU data is empty!" << std::endl;
    return;
}

// 2. Konversi Hex String ke Byte
// Menggunakan GetDECData versi Modern yang menerima std::string_view
// if (GetDECData(buft, m_APDUSENDSTRINGCMD) != 0) {
//     std::cerr << "Error: Invalid Hex Data!" << std::endl;
//     return;
// }

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x49;
	PmCode=0x39;
	CmDataLen=(m_APDUSENDSTRINGCMD.length())/2;

	// Gunakan std::copy: Lebih aman dan cepat daripada loop for manual
  // std::copy(buft, buft + CmDataLen, CmData);

  std::cout << "data len: " << CmDataLen << "data length:"<<m_APDUSENDSTRINGCMD.length() << std::endl;

  if (m_APDUSENDSTRINGCMD.length() % 2 != 0){ 
          std::cout << "data tidak genap";
          return; // Error: Panjang harus genap
    }

    for (size_t i = 0; i < m_APDUSENDSTRINGCMD.length(); i += 2) {
        std::string byteString = std::string(m_APDUSENDSTRINGCMD.substr(i, 2));
        buft[i / 2] = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
    }

  // for (int i=0;i<m_APDUSENDSTRINGCMD.length();i++)
	// {
	// 	CmData[i]=buft[i];
	// }
  std::copy(buft, buft + CmDataLen, CmData);

	int		rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt1,&ReDataLen,ReData);			
	
  if(rc==0)
	{
		if (ReType==0x50)
		{
			std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        
        for (int n = 0; n < ReDataLen; ++n) {
            ss << std::setw(2) << static_cast<int>(ReData[n]);
        }
        
        // Output akhir berupa std::string
        std::string resultHex = ss.str();
			// UpdateData(false);
			//MessageBox("Send APDU Successed");
      std::cout << "Send APDU Success: " << resultHex << std::endl;
		}
		else if  (ReType==0x4e)
		{
      // SEt0 = SW1, SEt1 = SW2
      if (SEt0 == 0x6A && SEt1 == 0x82) {
          std::cerr << "Error 6A82: File atau AID tidak ditemukan di kartu!" << std::endl;
      } else {
          std::printf("Res err: %02X %02X\n", SEt0, SEt1);
      }
		}
		else
		{
      std::cerr << "Communication Error: Unexpected ReType 0x" 
                  << std::hex << (int)ReType << std::endl;
		}
	}
	else {
    std::cerr << "Communication Error: 0x" 
                  << std::hex << (int)ReType << std::endl;
  }
		//MessageBox("Communication Error"); 
    m_APDUSENDSTRINGCMD = {};
}

#include <iostream>
#include <string>
#include <algorithm>

// Fungsi helper untuk mencari nilai berdasarkan Tag di dalam string TLV Hex
std::string findEMVTagValue(const std::string& tlvHex, const std::string& targetTag) {
    std::string target = targetTag;
    // Pastikan komparasi menggunakan huruf besar (uppercase)
    std::transform(target.begin(), target.end(), target.begin(), ::toupper);
    
    size_t i = 0;
    while (i < tlvHex.length()) {
        if (i + 2 > tlvHex.length()) break;
        
        // 1. Ambil byte pertama untuk cek Tag
        std::string tag = tlvHex.substr(i, 2);
        i += 2;
        
        // Cek apakah ini Tag 2 byte (akhiran byte pertama adalah 1F / berujung 'F' atau 'f')
        // Contoh: 9F, 5F, 7F
        if ((tag[1] == 'F' || tag[1] == 'f') && (tag[0] == '9' || tag[0] == '5' || tag[0] == '7' || tag[0] == 'D')) {
            if (i + 2 > tlvHex.length()) break;
            tag += tlvHex.substr(i, 2); // Tambah 1 byte lagi untuk jadi Tag 2 byte
            i += 2;
        }
        
        // 2. Ambil byte Length
        if (i + 2 > tlvHex.length()) break;
        std::string lenHex = tlvHex.substr(i, 2);
        i += 2;
        
        // Konversi Length dari Hex ke Integer (Jumlah Byte)
        int length = std::stoi(lenHex, nullptr, 16);
        int charLength = length * 2; // 1 byte = 2 karakter hex
        
        // 3. Ambil Value
        if (i + charLength > tlvHex.length()) break;
        std::string value = tlvHex.substr(i, charLength);
        
        // Jika Tag cocok dengan yang dicari, kembalikan nilainya
        std::transform(tag.begin(), tag.end(), tag.begin(), ::toupper);
        if (tag == target) {
            return value;
        }
        
        // Pindah ke Tag berikutnya
        i += charLength;
    }
    return ""; // Kembalikan string kosong jika tag tidak ditemukan
}

int CommandCardCRT::OnCPURESETButton() 
{
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt1;
    int  ReDataLen;
	unsigned char ReData[1024];

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x49;
	PmCode=0x30;
	CmDataLen=1;
	CmData[0]=0x30;
	// CmData[0]=0x33;
	// CmData[0]=0x35;
	int	rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt1,&ReDataLen,ReData);		
	if(rc==0)
	{
		if (ReType==0x50)
		{
			std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        
        for (int n = 0; n < ReDataLen; ++n) {
            ss << std::setw(2) << static_cast<int>(ReData[n]);
        }
        
        std::string resultHex = ss.str();
      std::cout << "CPU Card Activate Successed {Cold Reset}: " << resultHex << std::endl;

			if (ReData[0]==48)
			{
        std::cout << "CPU Card(T=0) Activate Successed" << std::endl;
			}
			if (ReData[0]==49)
			{
        std::cout << "CPU Card(T=1) Activate Successed" << std::endl;
			}
      return SUCSESS_RESP;
		}
		else if((ReType==0x4e) && (ReDataLen>0))
		{
			std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        
        for (int n = 0; n < ReDataLen; ++n) {
            ss << std::setw(2) << static_cast<int>(ReData[n]);
        }
        
        // Output akhir berupa std::string
        std::string resultHex = ss.str();
			// UpdateData(false);

     std::cout << "Send APDU Success: " << resultHex << std::endl;
			if (ReData[0]==48)
			{
        std::cout << "CPU Card(T=0) Activate Successed,But CPU card information does not meet the EMV mode ATR" << std::endl;
			}
      return FAIL_RESP;
		}
		else if((ReType==0x4e) && (ReDataLen==0))
		{
			 std::fprintf(stderr, "Response Error: %02X %02X\n", SEt0, SEt1);
       return FAIL_RESP;
		}
		else 
		{
			std::cerr << "Cold Reset Error: Unexpected ReType 0x" 
                  << std::hex << (int)ReType << std::endl;
      return FAIL_RESP;
		}
	}
	else 
	{
		std::cerr << "ICRW is not Connect or Comm. port is not Opened" << std::endl;
    return DISCONNECT_RESP;
	}	
}

bool CommandCardCRT::PaymentSystemEnvironment(unsigned char *SEt1){
m_APDUSENDSTRINGCMD = PSE1;
	unsigned char buft[1024];
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt11;
    int  ReDataLen;
	unsigned char ReData[1024];

	// 1. Validasi Input menggunakan std::string_view
if (m_APDUSENDSTRINGCMD.empty()) {
    std::cerr << "Error: APDU data is empty!" << std::endl;
    return 1;
}

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x49;
	PmCode=0x39;
	CmDataLen=(m_APDUSENDSTRINGCMD.length())/2;

	// Gunakan std::copy: Lebih aman dan cepat daripada loop for manual
  // std::copy(buft, buft + CmDataLen, CmData);

  std::cout << "data len: " << CmDataLen << "data length:"<<m_APDUSENDSTRINGCMD.length() << std::endl;

  if (m_APDUSENDSTRINGCMD.length() % 2 != 0){ 
          std::cout << "data tidak genap";
          return 1; // Error: Panjang harus genap
    }

    for (size_t i = 0; i < m_APDUSENDSTRINGCMD.length(); i += 2) {
        std::string byteString = std::string(m_APDUSENDSTRINGCMD.substr(i, 2));
        buft[i / 2] = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
    }

  std::copy(buft, buft + CmDataLen, CmData);

	int		rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt11,&ReDataLen,ReData);			
	*SEt1 = SEt11;
  if(rc==0)
	{
		if (ReType==0x50)
		{
			std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        
        for (int n = 0; n < ReDataLen; ++n) {
            ss << std::setw(2) << static_cast<int>(ReData[n]);
        }
        
        // Output akhir berupa std::string
        std::string resultHex = ss.str();
			// UpdateData(false);
			//MessageBox("Send APDU Successed");
      std::cout << "Send APDU Success: " << resultHex << std::endl;
      return 0;
		}
		else if  (ReType==0x4e)
		{
      // SEt0 = SW1, SEt1 = SW2
      if (SEt0 == 0x6A && SEt11 == 0x82) {
          std::cerr << "Error 6A82: File atau AID tidak ditemukan di kartu!" << std::endl;
      } else {
          std::printf("Res err: %02X %02X\n", SEt0, SEt11);
      }
      return 1;
		}
		else
		{
      std::cerr << "Communication Error: Unexpected ReType 0x" 
                  << std::hex << (int)ReType << std::endl;
                  return 1;
		}
	}
	else {
    std::cerr << "Communication Error: 0x" 
                  << std::hex << (int)ReType << std::endl;
                  return 1;
  }
		//MessageBox("Communication Error"); 
    m_APDUSENDSTRINGCMD = {};

    return 0;
 }

std::string CommandCardCRT::GetResponse(int mode, unsigned char *SEt1, std::vector<uint8_t> *GenResp){
  m_APDUSENDSTRINGCMD = RESP;
	unsigned char buft[1024];
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	unsigned char SEt0;
	unsigned char SEt11;
    int  ReDataLen;
	unsigned char ReData[1024];

	// 1. Validasi Input menggunakan std::string_view
if (m_APDUSENDSTRINGCMD.empty()) {
    std::cerr << "Error: APDU data is empty!" << std::endl;
    return "ERR";
}

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x49;
	PmCode=0x39;
	CmDataLen=(m_APDUSENDSTRINGCMD.length())/2;

	// Gunakan std::copy: Lebih aman dan cepat daripada loop for manual
  // std::copy(buft, buft + CmDataLen, CmData);

  std::cout << "data len: " << CmDataLen << "data length:"<<m_APDUSENDSTRINGCMD.length() << std::endl;

  if (m_APDUSENDSTRINGCMD.length() % 2 != 0){ 
          std::cout << "data tidak genap";
          return "ERR"; // Error: Panjang harus genap
    }

    int currentPos = 0;
    for (size_t i = 0; i < m_APDUSENDSTRINGCMD.length(); i += 2) {
        std::string byteString = std::string(m_APDUSENDSTRINGCMD.substr(i, 2));
        buft[currentPos++] = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
    }

    buft[currentPos++] = *SEt1;

  std::copy(buft, buft + CmDataLen, CmData);

	int		rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0,&SEt11,&ReDataLen,ReData);			
	*SEt1 = SEt11;
  if(rc==0)
	{
		if (ReType==0x50)
		{
			std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        
        for (int n = 0; n < ReDataLen; ++n) {
            ss << std::setw(2) << static_cast<int>(ReData[n]);
        }
        
        // Output akhir berupa std::string
        std::string resultHex = ss.str();
			// UpdateData(false);
			//MessageBox("Send APDU Successed");
      GenResp->assign(ReData, ReData + ReDataLen);
      std::cout << "Send APDU Success: " << resultHex << std::endl;
      return resultHex; 
		}
		else if  (ReType==0x4e)
		{
      // SEt0 = SW1, SEt1 = SW2
      if (SEt0 == 0x6A && SEt11 == 0x82) {
          std::cerr << "Error 6A82: File atau AID tidak ditemukan di kartu!" << std::endl;
      } else {
          std::printf("Res err: %02X %02X\n", SEt0, SEt11);
      }
      //return *SEt1; 
		}
		else
		{
      std::cerr << "Communication Error: Unexpected ReType 0x" 
                  << std::hex << (int)ReType << std::endl;
                  //return *SEt1; 
		}
	}
	else {
    std::cerr << "Communication Error: 0x" 
                  << std::hex << (int)ReType << std::endl;
                  //return *SEt1; 
  }
		//MessageBox("Communication Error"); 
    m_APDUSENDSTRINGCMD = {};

    return "ERR";

}

std::string CommandCardCRT::ReadRecord(){

  return "ERR";

}
std::string CommandCardCRT::SelectAID(){

  return "ERR";

}
std::string CommandCardCRT::GetProcessingOptions(){

  return "ERR";

}

std::string CommandCardCRT::CommandData(std::string datass, 
                                        unsigned char *SEt0,
                                          unsigned char *SEt1){
std::string datasret = "";

	m_APDUSENDSTRINGCMD = datass;
	unsigned char buft[1024];
	unsigned char CmCode;
	unsigned char PmCode;
    int  CmDataLen;
	unsigned char CmData[1024];

	unsigned char ReType;
	
	 unsigned char SEt0A;
	 unsigned char SEt1A;
    int  ReDataLen;
	unsigned char ReData[1024];

	// 1. Validasi Input menggunakan std::string_view
if (m_APDUSENDSTRINGCMD.empty()) {
    std::cerr << "Error: APDU data is empty!" << std::endl;
    return datasret;
}

	memset(CmData,0x00,sizeof(CmData));
	CmCode=0x49;
	PmCode=0x39;
	CmDataLen=(m_APDUSENDSTRINGCMD.length())/2;

	// Gunakan std::copy: Lebih aman dan cepat daripada loop for manual
  // std::copy(buft, buft + CmDataLen, CmData);

  std::cout << "data len: " << CmDataLen << "data length:"<<m_APDUSENDSTRINGCMD.length() << std::endl;

  if (m_APDUSENDSTRINGCMD.length() % 2 != 0){ 
          std::cout << "data tidak genap";
          return datasret; // Error: Panjang harus genap
    }

    for (size_t i = 0; i < m_APDUSENDSTRINGCMD.length(); i += 2) {
        std::string byteString = std::string(m_APDUSENDSTRINGCMD.substr(i, 2));
        buft[i / 2] = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
    }

  std::copy(buft, buft + CmDataLen, CmData);

	int		rc=RS232_ExeCommand(buf,CmCode,PmCode,CmDataLen,CmData,&ReType,&SEt0A,&SEt1A,&ReDataLen,ReData);
  
  // if (SEt0 != nullptr) {
  //       *SEt0 = SEt0A;
  //   }
  //   if (SEt1 != nullptr) {
  //       *SEt1 = SEt1A;
  //   }
	
  if(rc==0)
	{
		if (ReType==0x50)
		{
			std::stringstream ss;
        ss << std::hex << std::uppercase << std::setfill('0');
        
        for (int n = 0; n < ReDataLen; ++n) {
            ss << std::setw(2) << static_cast<int>(ReData[n]);
        }
        
        // Output akhir berupa std::string
        std::string resultHex = ss.str();
			// UpdateData(false);
			//MessageBox("Send APDU Successed");
      std::cout << "Send APDU Success: " << resultHex << std::endl;
      datasret = resultHex;
      if (ReDataLen >= 2) 
      {
          unsigned char sw1 = ReData[ReDataLen - 2];
          unsigned char sw2 = ReData[ReDataLen - 1];

          if (SEt0 != nullptr) *SEt0 = sw1;
          if (SEt1 != nullptr) *SEt1 = sw2;

          std::printf("Res SW: %02X %02X\n", sw1, sw2);
      }
      std::printf("Res SW: %02X %02X\n", *SEt0, *SEt1);
		}
		else if  (ReType==0x4e)
		{
      // SEt0 = SW1, SEt1 = SW2
      if (SEt0A == 0x6A && SEt1A == 0x82) {
          std::cerr << "Error 6A82: File atau AID tidak ditemukan di kartu!" << std::endl;
      } else {
          std::printf("Res err: %02X %02X\n", SEt0A, SEt1A);
      }
		}
		else
		{
      std::cerr << "Communication Error: Unexpected ReType 0x" 
                  << std::hex << (int)ReType << std::endl;
		}
	}
	else {
    std::cerr << "Communication Error: 0x" 
                  << std::hex << (int)ReType << std::endl;
  }
		//MessageBox("Communication Error"); 
    m_APDUSENDSTRINGCMD = {};

  return datasret;
}
//util
std::string TLV::getValueHex() const {
    std::stringstream ss;
    for (uint8_t b : value) {
        ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << (int)b;
    }
    return ss.str();
}

std::vector<TLV> CommandCardCRT::parse(const uint8_t* data, size_t size) {
    std::vector<TLV> results;
    size_t offset = 0;

    while (offset < size) {
        // Abaikan padding byte (0x00 atau 0xFF) yang sering muncul di Smart Card
        if (data[offset] == 0x00 || data[offset] == 0xFF) {
            offset++;
            continue;
        }

        TLV element;

        // 1. Parsing Tag (ISO 7816-4 multi-byte support)
        element.tag = data[offset++];
        if ((element.tag & 0x1F) == 0x1F) { 
            // Jika 5 bit terakhir 1, maka ada byte berikutnya
            while (offset < size) {
                element.tag = (element.tag << 8) | data[offset++];
                if (!(element.tag & 0x80)) break; // Bit 8 = 0 artinya byte terakhir tag
            }
        }

        // 2. Parsing Length
        if (offset >= size) break;
        uint8_t lenByte = data[offset++];
        if (lenByte & 0x80) { 
            // Extended format (0x81, 0x82, dst)
            int numBytes = lenByte & 0x7F;
            element.length = 0;
            for (int i = 0; i < numBytes && offset < size; ++i) {
                element.length = (element.length << 8) | data[offset++];
            }
        } else {
            element.length = lenByte;
        }

        // 3. Parsing Value
        if (offset + element.length <= size) {
            element.value.assign(data + offset, data + offset + element.length);
            offset += element.length;
        } else {
            // Data tidak lengkap/corrupt
            break;
        }

        results.push_back(element);
    }
    return results;
}

bool CommandCardCRT::isConstructed(uint32_t tag) {
    // Menurut ISO 7816, jika bit 6 dari byte pertama tag adalah 1, maka itu constructed
    // Kita ambil byte paling signifikan dari tag
    uint8_t firstByte = 0;
    if (tag > 0xFFFF) firstByte = (tag >> 16) & 0xFF;
    else if (tag > 0xFF) firstByte = (tag >> 8) & 0xFF;
    else firstByte = tag & 0xFF;

    return (firstByte & 0x20); 
}

void CommandCardCRT::debugPrint(const std::vector<TLV>& items, int *tugas, int *perintah, int indent) {
    std::string space(indent, ' ');
    bool sudah_94 = false;
    bool sudah_88 = false;
    //tugas 1 untuk mencari AFL jika tidak ada afl/ tag 94 maka yang dicari adalah tag 88 / lokasi file yang ada AFL nya
    //jika dalam tugas 1 ada afl maka lanjut ke perintah 2, jika tidak maka masuk ke perintah 1 dulu ke directori yang dimaksud tag 88
    for (const auto& item : items) {
        if(*tugas == 1){
          if(item.tag == 0x94 && sudah_94 == false){
            sudah_94=true;
          }
          if(item.tag == 0x88 && sudah_88 == false){
            sudah_88=true;
          }
        }
        std::cout << space << "Tag: [" << std::hex << std::uppercase << item.tag << "] "
                  << "Len: [" << std::dec << item.length << "] "
                  << "Val: " << item.getValueHex() << std::endl;

        if (isConstructed(item.tag)) {
            auto subItems = parse(item.value.data(), item.value.size());
            debugPrint(subItems,tugas,perintah, indent + 4);
        }
    }

    if(*tugas == 1 && sudah_94){
      *perintah = 2; 
    }
    
    if(*tugas == 1 && !sudah_94 && sudah_88){
      *perintah = 1; 
    }
}

int CommandCardCRT::IdentifyCard(){
  int resp = OnCPURESETButton();
  if (resp == 0)
    return SUCSESS_RESP;
  if (resp == 2)
    return DISCONNECT_RESP;

  bool loops = 1;
  int count = 0;
  while (loops)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    resp = OnCPURESETButton();
    if (resp == 0)
      return SUCSESS_RESP;
    else if (resp == 2)
      return DISCONNECT_RESP;
    if (count == 3)
    {
      return FAIL_RESP;
    }
    count += 1;
  }
}

std::vector<unsigned char> CommandCardCRT::generateTerminalData(unsigned int tag, size_t length) {
    std::vector<unsigned char> value(length, 0x00); // Default isi 0x00

    if (tag == 0x9F37) { // Unpredictable Number (Random Bytes)
        for (size_t i = 0; i < length; ++i) {
            value[i] = std::rand() % 256;
        }
    }
    else if (tag == 0x9F1A) { // Terminal Country Code (Indonesia: 03 60)
        if (length == 2) { value[0] = 0x03; value[1] = 0x60; }
    }
    else if (tag == 0x5F2A) { // Transaction Currency Code (IDR: 03 60)
        if (length == 2) { value[0] = 0x03; value[1] = 0x60; }
    }
    else if (tag == 0x9A03) { // Transaction Date (YYMMDD - Misal 260518 untuk 18 Mei 2026)
        if (length == 3) { value[0] = 0x26; value[1] = 0x05; value[2] = 0x18; }
    }
    else if (tag == 0x9F02) { // Amount, Authorised (Numeric) - Misal Rp 100.000 -> 000000010000
        if (length == 6) { value[4] = 0x01; } 
    }
    // Tag lain yang tidak didefinisikan akan otomatis terisi 0x00 (Aman untuk EMV)
    return value;
}

// Merakit APDU GPO secara dinamis dari data PDOL mentah
std::string CommandCardCRT::buildDynamicGpo(const std::vector<unsigned char>& pdolData) {
    std::vector<unsigned char> gpoPayload;
    size_t i = 0;

    // Ekstrak Tag dan Length dari dalam daftar PDOL
    while (i < pdolData.size()) {
        unsigned int tag = pdolData[i++];
        if ((tag & 0x1F) == 0x1F) { // Jika Tag berukuran 2-byte (misal 9F37)
            if (i >= pdolData.size()) break;
            tag = (tag << 8) | pdolData[i++];
        }
        
        if (i >= pdolData.size()) break;
        size_t length = pdolData[i++];

        // Ambil data terminal penunjang
        std::vector<unsigned char> token = generateTerminalData(tag, length);
        gpoPayload.insert(gpoPayload.end(), token.begin(), token.end());
    }

    unsigned char pdolLength = gpoPayload.size();
    unsigned char lcValue = pdolLength + 2; // Ditambah 2 byte untuk prefix '83' dan 'Length'

    std::stringstream ss;
    ss << "80A80000" 
       << std::hex << std::uppercase << std::setfill('0') 
       << std::setw(2) << (int)lcValue 
       << "83" 
       << std::setw(2) << (int)pdolLength;

    for (unsigned char b : gpoPayload) {
        ss << std::setw(2) << (int)b;
    }
    ss << "00";

    return ss.str();
}

// Fungsi helper konversi Byte Array ke Hex String
std::string CommandCardCRT::bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (unsigned char b : bytes) ss << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

std::string CommandCardCRT::hexToAscii(const std::string& hex) {
    std::string ascii = "";
    for (size_t i = 0; i < hex.length(); i += 2) {
        char part = (char)strtol(hex.substr(i, 2).c_str(), nullptr, 16);
        if (part >= 32 && part <= 126) { // Hanya ambil karakter printable
            ascii += part;
        }
    }
    return ascii;
}

std::string CommandCardCRT::hexToAsciiString(const std::string& hex) {
    std::string ascii = "";
    
    // Melompat 2 karakter sekaligus karena 1 karakter ASCII = 2 karakter Hex (1 Byte)
    for (size_t i = 0; i < hex.length(); i += 2) {
        // Ambil potongan 2 karakter Hex, misal "4A"
        std::string partStr = hex.substr(i, 2);
        
        // Konversi string Hex basis 16 menjadi bilangan Desimal (Long), lalu cast ke char
        char character = static_cast<char>(strtol(partStr.c_str(), nullptr, 16));
        
        // Filter Karakter Nyata (Printable Characters):
        // Hanya masukkan karakter mulai dari Spasi (32) sampai dengan karakter '~' (126).
        // Karakter data sampah/padding seperti 0x00 atau 0xFF akan otomatis dibuang.
        if (character >= 32 && character <= 126) {
            ascii += character;
        }
    }
    return ascii;
}

std::string CommandCardCRT::toHexStr(unsigned char value) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)value;
    return ss.str();
}

// --- CORE RECURSIVE TLV ENGINE ---
void CommandCardCRT::parseEMVTLV(const std::vector<unsigned char>& data, int flags) {
   if (data.empty()) return;

    unsigned char topTag = data[0];

    // =========================================================================
    // KONDISI A: Format Pendek / Tanpa Tag (Template 80) -> Khas GENERATE AC
    // =========================================================================
    if (topTag == 0x80 && flags == 0) {
        size_t length = data[1];
        // Minimal 11 byte payload (1 byte CID + 2 byte ATC + 8 byte AC)
        if (length >= 11 && data.size() >= (2 + length)) {
            emv.isAcSuccess = true;
            
            // Byte indeks ke-2 adalah CID (Cryptogram Information Data)
            emv.cid = toHexStr(data[2]);
            
            // 2 Byte berikutnya adalah ATC (Application Transaction Counter)
            std::vector<unsigned char> atcBytes(data.begin() + 3, data.begin() + 5);
            emv.atc = bytesToHex(atcBytes);
            
            // 8 Byte berikutnya adalah inti dari ARQC (Application Cryptogram)
            std::vector<unsigned char> arqcBytes(data.begin() + 5, data.begin() + 13);
            emv.arqc = bytesToHex(arqcBytes);
        }

            // === TAMBAHAN UNTUK MENCARI IAD (TAG 9F10) ===
        // Pastikan ada sisa data setelah ARQC (panjang payload > 11 byte)
        if (length > 11) {
            // IAD dimulai dari indeks 13 sampai akhir panjang data payload (2 + length)
            std::vector<unsigned char> iadBytes(data.begin() + 13, data.begin() + (2 + length));
            emv.iad = bytesToHex(iadBytes); // Pastikan variabel emv.iad sudah dideklarasikan
        } else {
            emv.iad = ""; // Kosong jika tidak ada data IAD bawaan
        }
    }
     // =========================================================================
    // KONDISI A: Format Pendek / Tanpa Tag (Template 80) -> Khas GPO
    // =========================================================================
    if (topTag == 0x80 && flags == 1) {
        std::cout << "Parse GPO format 80!!" <<std::endl;
        size_t length = data[1];
        // Minimal 11 byte payload (1 byte CID + 2 byte ATC + 8 byte AC)
        // data adalah Tag (0x80)

          // Validasi apakah ukuran data yang diterima cukup
          if (data.size() >= (2 + length) && length >= 2) {
              
              // 1. Ambil 2 Byte Pertama setelah Length sebagai AIP
              std::vector<unsigned char> aipBytes(data.begin() + 2, data.begin() + 4);
              emv.ApplicationInterchangeProfile = bytesToHex(aipBytes); // Hasil: "1800"
}
    }
    // =========================================================================
    // KONDISI B: Format TLV Berstruktur (Template 77, 70, 61, atau Root Lainnya)
    // =========================================================================
    else if (topTag == 0x77 || topTag == 0x70 || topTag == 0x61 || (topTag & 0x20)) {
        // Tentukan titik mulai pembacaan data setelah Tag dan Length utama
        size_t i = 2; 
        if (data[1] & 0x80) {
            // Mengantisipasi jika byte Length utama berukuran lebih dari 1 byte (Format ASN.1)
            i = 2 + (data[1] & 0x7F);
        }
        
        size_t end = data.size();
        
        while (i < end) {
            // 1. Ekstrak Tag (Mendukung 1-byte maupun 2-byte EMV Tag)
            unsigned int tag = data[i++];
            if ((tag & 0x1F) == 0x1F) {
                if (i >= end) break;
                tag = (tag << 8) | data[i++];
            }
            
            // 2. Ekstrak Length
            if (i >= end) break;
            size_t length = data[i++];
            if (length & 0x80) {
                size_t numBytes = length & 0x7F;
                length = 0;
                for (size_t l = 0; l < numBytes; ++l) {
                    if (i >= end) break;
                    length = (length << 8) | data[i++];
                }
            }
            
            if (i + length > end) break;
            
            // Ambil array value subset byte-nya
            std::vector<unsigned char> valBytes(data.begin() + i, data.begin() + i + length);
            std::string valHex = bytesToHex(valBytes);
            
            // 3. SWITCH CASE MULTI-TAG (Gabungan Data Kartu + Data AC)
             switch (tag) {
                case 0x9F08:
                    emv.ApplicationVersionNumber = valHex;
                    break;
                case 0x9F44:
                   emv.ApplicationCurrencyExponent = valHex; 
                    break;
                case 0x82: 
                    emv.ApplicationInterchangeProfile = valHex;
                    break;
                // --- KELOMPOK DATA GENERATE AC ---
                case 0x9F26: // Application Cryptogram / ARQC
                    emv.arqc = valHex;
                    emv.isAcSuccess = true;
                    break;
                case 0x9F27: // Cryptogram Information Data
                    emv.cid = valHex;
                    break;
                case 0x9F36: // Application Transaction Counter
                    emv.atc = valHex;
                    break;
                case 0x9F10:
                    emv.iad = valHex;
                    break;
                case 0x9F38:
                    std::cout << "ADA NIH PDOL = "<<valHex <<std::endl;
                    break;
                case 0x8C:
                    std::cout << "ADA NIH CDOL 1" <<valHex <<std::endl;
                    emv.cdol1Raw = valHex;
                    break;
                case 0x8D:
                    std::cout << "ADA NIH CDOL 2"<<valHex <<std::endl;
                    emv.cdol2Raw = valHex;
                    break;
               // --- KELOMPOK DATA TERMINAL / TRANSAKSI BARU ---
                case 0x9F1A: // Terminal Country Code
                    emv.terminalCountry = valHex;
                    break;
                case 0x5F2A: // Transaction Currency Code 9F42
                    emv.currencyCode = valHex;
                    break;
                case 0x5F28:
                    emv.countrycode = valHex;
                    break;
                case  0x87:
                    emv.priority = valHex;
                      std::cout << "ADA NIH priority!!"<<valHex <<std::endl;
                    break;
                case 0x9F0D:
                    emv.IACDefault = valHex;
                    break;
                case 0x9F0E:
                    emv.IACDenial = valHex;
                    break;
                case 0x9F0F:
                    emv.IACOnline = valHex;
                    break;
                case 0x84:
                    emv.DedicatedFile = valHex;
                    break;
                case 0x57: { // Track 2 Equivalent Data
                    size_t dPos = valHex.find('D');
                    if (dPos != std::string::npos) {
                      // 1. Ambil PAN (sebelum 'D')
                      emv.pan = valHex.substr(0, dPos);
                      
                      // 2. Ambil Expiry Date (4 digit setelah 'D')
                      if (valHex.length() >= dPos + 5) {
                          emv.expiryDate = valHex.substr(dPos + 1, 4); // Format: YYMM
                      }
                      
                      // 3. Ambil Service Code (3 digit setelah Expiry Date)
                      if (valHex.length() >= dPos + 8) {
                          emv.serviceCode = valHex.substr(dPos + 5, 3); // Format: 3 digit angka
                      }
                      
                      // 4. Ambil Discretionary Data (Sisa string setelah Service Code)
                      if (valHex.length() > dPos + 8) {
                          std::string discData = valHex.substr(dPos + 8);
                          
                          // Opsional: Hapus padding 'F' atau 'f' di bagian paling akhir string jika ada
                          if (!discData.empty() && (discData.back() == 'F' || discData.back() == 'f')) {
                              discData.pop_back();
                          }
                          emv.discretionaryData = discData;
                      }
                      emv.isPanFound = true;
                    }
                    break;
                }
                case 0x5A: // PAN Murni Terpisah
                    if (!emv.isPanFound) {
                        emv.pan = valHex;
                        emv.isPanFound = true;
                    }
                    break;
                case 0x5F20: // Cardholder Name
                    emv.cardholderName = hexToAsciiString(valHex);
                    break;
                case 0x5F24: // Expiration Date (YYMMDD)
                    if (emv.expiryDate == "Tidak Ditemukan") {
                        emv.expiryDate = valHex.substr(0, 4); // Ambil 4 digit depan (YYMM)
                    }
                    break;
                case 0x8E:
                    emv.cardholderverification = valHex;
                    break;
                case 0x5F34:
                    emv.panSequenceNumber = valHex;
                // --- PENANGANAN JIKA ADA SUB-TEMPLATE DI DALAM RECORD ---
                    break;
                case 0x70:
                    // Jika di dalam loop mendeteksi ada sub-template pembungkus lagi, 
                    // panggil fungsi ini secara rekursif untuk membongkar isinya.
                    parseEMVTLV(valBytes);
                    break;

                default:
                    // Tag lain diabaikan secara aman demi efisiensi memori kiosk
                    break;
            }
            
            i += length; // Lompat ke tag berikutnya di level yang sama
        }
    }
}

std::string CommandCardCRT::GenerateAFL(std::string data){

    std::cout << "\n--- Memulai Eksekusi & Parsing Massal SFI ---" << std::endl;

    // for (size_t idx = 0; idx < targetCommands.size(); ++idx) {
    //     std::string currentApdu = targetCommands[idx].apduHex;
    //     std::cout << "\nExecuting [" << idx << "] -> " << currentApdu << std::endl;

        unsigned char sw1 = 0, sw2 = 0;
        // Panggil fungsi utama RS232 Anda
        std::string responseHex = CommandData(data, &sw1, &sw2); 
        unsigned int sw = (sw1 << 8) | sw2;

        if (sw == 0x9000 || sw1 == 0x61) {
            // Hilangkan status word 9000 di ekor string jika ada sebelum parsing murni TLV
            if (responseHex.length() >= 4 && responseHex.substr(responseHex.length() - 4) == "9000") {
                responseHex = responseHex.substr(0, responseHex.length() - 4);
            }

            std::vector<unsigned char> responseBytes = hexToBytes(responseHex);

            // Tembak ke parser rekursif untuk diekstrak secara otomatis
            parseEMVTLV(responseBytes);
        } else {
            std::cerr << "[LEWAT] Record merespon error / kosong. SW: " << std::hex << sw << std::dec << std::endl;
        }
    // }
} 

std::string CommandCardCRT::generateIad28Bytes(std::string iadHex) {
    // 1. Bersihkan spasi dari string input jika ada
   // iadHex.erase(std::remove(iadHex.begin(), iadHex.end(), ' '), iadHex.end());
    
    // 2. Target 28 byte = 56 karakter heksadesimal
    size_t targetLength = 56;
    
    // 3. Tambahkan padding '0' di sebelah kanan jika panjangnya kurang
    if (iadHex.length() < targetLength) {
        iadHex.append(targetLength - iadHex.length(), '0');
    }
    
    return iadHex;
}

std::string CommandCardCRT::MulaiGPO(std::string data){
  std::vector<unsigned char> tlvData = hexToBytes(data);
    std::vector<unsigned char> pdolValue;
    std::string apduGPO = "";

    std::cout << "\n--- Inisialisasi Proses GPO ---" << std::endl;

    // 1. Cek apakah Tag 9F38 (PDOL) ada di dalam FCI Respon SELECT AID
    // Fungsi findTagValue adalah fungsi rekursif TLV parser yang kita buat sebelumnya
    if (findTagValue(tlvData, 0, tlvData.size(), 0x9F38, pdolValue)) 
    {
        // KONDISI A: PDOL Ditemukan!
        std::cout << "[INFO] Kartu meminta PDOL. Merakit GPO secara dinamis..." << std::endl;
        std::cout << "PDOL Raw Data: " << bytesToHex(pdolValue) << std::endl;
        
        apduGPO = buildDynamicGpo(pdolValue);
    } 
    else 
    {
        // KONDISI B: PDOL Tidak Ditemukan!
        std::cout << "[INFO] Kartu tidak meminta PDOL. Menggunakan GPO standar..." << std::endl;
        
        apduGPO = "80A8000002830000";
    }

    // 2. Eksekusi Pengiriman APDU GPO ke Kartu
    std::cout << "Mengirim APDU GPO: " << apduGPO << std::endl;

    unsigned char sw1 = 0x00;
    unsigned char sw2 = 0x00;
    
    // Panggil fungsi CommandData Anda yang sudah auto-handle respon 61xx
    std::string gpoResponse = CommandData(apduGPO, &sw1, &sw2);
    unsigned int statusWord = (sw1 << 8) | sw2;

    // if(sw1 == 0x61){

    // }

    if (statusWord == 0x9000) {
        std::cout << "[SUKSES] Perintah GPO Berhasil Direspon Kartu." << std::endl;
        std::cout << "Respon GPO Data: " << gpoResponse << std::endl;
        if (gpoResponse.length() >= 4 && gpoResponse.substr(gpoResponse.length() - 4) == "9000") {
                gpoResponse = gpoResponse.substr(0, gpoResponse.length() - 4);
            }

            std::vector<unsigned char> responseBytes = hexToBytes(gpoResponse);

            // Tembak ke parser rekursif untuk diekstrak secara otomatis
            parseEMVTLV(responseBytes,1);
        
       int hasilafl = CariAFL(gpoResponse); //belum ditambahkan logic lagi
        
        // Langkah selanjutnya: Memparsing 'gpoResponse' untuk mengambil AFL (Application File Locator)
    } else {
        std::cerr << "[ERROR] GPO Ditolak oleh kartu dengan SW: " 
                  << std::hex << std::uppercase << statusWord << std::dec << std::endl;
                 // return "GPO ditolak";
    }
}
std::string CommandCardCRT::ConvertBytetoString(std::vector<BYTE> data){
  std::stringstream ss;
    
        for (size_t i = 0; i < data.size(); ++i) {
            ss << std::hex          // Set format ke heksadesimal
              << std::setw(2)       // Pastikan panjangnya selalu 2 karakter (misal: 00, bukan 0)
              << std::setfill('0')  // Isi dengan angka '0' jika hanya 1 karakter
              << static_cast<int>(data[i]); // Cast ke int agar tidak terbaca sebagai karakter ASCII
        }
        
        std::string str = ss.str();
        
        // Opsional: Ubah menjadi huruf kapital (00c00000 -> 00C00000)
        for (char &c : str) c = std::toupper(c);
        return str;
}

// =========================================================================
// LOGIKA UTAMA: EKSTRAKSI AFL RAW DARI DUA TEMPLATE BERBEDA
// =========================================================================
std::vector<unsigned char> extractAflRawData(const std::vector<unsigned char>& fullResponse) {
    std::vector<unsigned char> aflRaw;

    if (fullResponse.empty()) return aflRaw;

    unsigned char topTag = fullResponse[0];

    // --- KONDISI KARTU JADUL / FORMAT PENDEK (Template 80) ---
    if (topTag == 0x80) {
        std::cout << "[GPO INFO] Terdeteksi GPO Format Pendek (Template 80)" << std::endl;
        
        if (fullResponse.size() < 6) return aflRaw; // Validasi minimum size
        
        size_t dataLength = fullResponse[1];
        size_t aflStartIndex = 4; // Skip Tag[1], Len[1], AIP[2]
        
        if (dataLength > 2 && fullResponse.size() >= (aflStartIndex + (dataLength - 2))) {
            size_t aflLength = dataLength - 2;
            aflRaw.assign(fullResponse.begin() + aflStartIndex, fullResponse.begin() + aflStartIndex + aflLength);
        }
    }
    // --- KONDISI KARTU BARU / FORMAT TLV STRUKTUR (Template 77) ---
    else if (topTag == 0x77) {
        std::cout << "[GPO INFO] Terdeteksi GPO Format TLV Panjang (Template 77)" << std::endl;
        
        size_t i = 2; // Lompat Tag 77 [1 byte] dan Length [1 byte]
        size_t end = fullResponse.size();
        
        // Loop mencari Tag 94 secara biner TLV
        while (i < end) {
            unsigned int tag = fullResponse[i++];
            // Antisipasi jika ada tag 2-byte (meski tag 94 hanya 1 byte)
            if ((tag & 0x1F) == 0x1F) {
                if (i >= end) break;
                tag = (tag << 8) | fullResponse[i++];
            }
            
            if (i >= end) break;
            size_t length = fullResponse[i++];
            if (length & 0x80) {
                size_t numBytes = length & 0x7F;
                length = 0;
                for (size_t l = 0; l < numBytes; ++l) {
                    if (i >= end) break;
                    length = (length << 8) | fullResponse[i++];
                }
            }
            
            if (i + length > end) break;
            
            // JIKA MENEMUKAN TAG AFL (0x94)
            if (tag == 0x94) {
                aflRaw.assign(fullResponse.begin() + i, fullResponse.begin() + i + length);
                break; // AFL sudah ketemu, stop pencarian
            }
            
            i += length; // Loncat ke tag berikutnya jika bukan tag 94
        }
    }
    
    return aflRaw;
}

void CommandCardCRT::parsingGenerateAC(const std::string& hex) {
    // Validasi minimum panjang data (harus di atas 4 karakter / 2 byte status)
    if (hex.length() < 4) {
        std::cout << "Data terlalu pendek!" << std::endl;
        return;
    }

    // Pisahkan status SW1SW2 di akhir string (2 byte terakhir = 4 karakter hex)
    std::string dataBagian = hex.substr(0, hex.length() - 4);
    std::string statusWord = hex.substr(hex.length() - 4, 4);

    std::cout << "=== HASIL PARSING GENERATE AC ===" << std::endl;
    std::cout << "Status Perintah (SW1SW2) : " << statusWord << " (Sukses)" << std::endl;

    // Cek apakah menggunakan Template 80 (Format 1)
    if (dataBagian.rfind("80", 0) == 0) { 
        std::cout << "Jenis Format              : Template 80 (Format 1)" << std::endl;
        
        // Ambil Length (Karakter indeks 2 sampai 4)
        std::string lenHex = dataBagian.substr(2, 2);
        int lenVal = std::stoi(lenHex, nullptr, 16);
        std::cout << "Panjang Data (Length)     : " << lenVal << " Byte" << std::endl;

        // Pemotongan data berdasarkan posisi tetap (Fixed Position)
        std::string cid    = dataBagian.substr(4, 2);   // 1 Byte
        std::string atc    = dataBagian.substr(6, 4);   // 2 Byte
        std::string ac     = dataBagian.substr(10, 16); // 8 Byte
        std::string iad    = dataBagian.substr(26);     // Sisa data adalah IAD (Tag 9F10)

        std::cout << "Cryptogram Info (CID)     : " << cid << std::endl;
        std::cout << "Transaction Counter (ATC) : " << atc << std::endl;
        std::cout << "Application Cryptogram    : " << ac << std::endl;
        std::cout << "Issuer App Data (Tag 9F10): " << iad << std::endl;
    } else {
        std::cout << "Format bukan Template 80. Perlu parser BER-TLV standar." << std::endl;
    }
}

// Fungsi mendapatkan increment dari file dalam format string 8 karakter (Left-Padding '0')
std::string getIncrementStringFromFile(const std::string& filename) {
    int counter = 0;

    // 1. Baca angka terakhir dari file
    std::ifstream inputFile(filename);
    if (inputFile.is_open()) {
        inputFile >> counter;
        inputFile.close();
    } else {
        counter = 0; // Mulai dari 0 jika file belum ada
    }

    // 2. Increment nilai angkanya
    counter++;

    // 3. Tulis kembali angka murni (integer) ke file untuk pelacakan berikutnya
    std::ofstream outputFile(filename);
    if (outputFile.is_open()) {
        outputFile << counter;
        outputFile.close();
    }

    // 4. Ubah integer menjadi string
    std::string counterStr = std::to_string(counter);

    // 5. Proses Padding: Jika kurang dari 8 karakter, sisipkan '0' di sebelah KIRI
    if (counterStr.length() < 8) {
        counterStr = std::string(8 - counterStr.length(), '0') + counterStr;
    }

    return counterStr;
}



int CommandCardCRT::CariAFL(std::string data){
   // Potong status word 9000 di ekor string jika ada
    std::string cleanHex = data;
    if (cleanHex.length() >= 4 && cleanHex.substr(cleanHex.length() - 4) == "9000") {
        cleanHex = cleanHex.substr(0, cleanHex.length() - 4);
    }

    std::vector<unsigned char> fullResponseBytes = hexToBytes(cleanHex);
    
    // PANGGIL SANG EKSTRAKTOR ADAPTIF DI SINI
    std::vector<unsigned char> aflRawData = extractAflRawData(fullResponseBytes);

    if (aflRawData.empty()) {
        std::cerr << "Error: Tidak dapat menemukan atau mengekstrak data AFL dari respon GPO." << std::endl;
        return -1;
    }

    // --- LOGIKA SISA PROGRAM ANDA (TIDAK PERLU DIUBAH) ---
    if (aflRawData.size() % 4 != 0) {
        std::cerr << "Error: Data AFL tidak valid (bukan kelipatan 4 byte)." << std::endl;
        return -1;
    }

    std::vector<AflEntry> aflList;
    for (size_t i = 0; i < aflRawData.size(); i += 4) {
        AflEntry entry;
        entry.sfi = aflRawData[i] >> 3; 
        entry.startRecord = aflRawData[i + 1];
        entry.endRecord = aflRawData[i + 2];
        entry.odaBytes = aflRawData[i + 3];
        aflList.push_back(entry);
    }

    std::vector<ApduCommand> targetCommands;
    std::cout << "--- Hasil Parsing Peta AFL ---" << std::endl;
    for (const auto& afl : aflList) {
        std::cout << "SFI: " << (int)afl.sfi 
                  << " | Record: " << (int)afl.startRecord << " sampai " << (int)afl.endRecord << std::endl;

        for (unsigned char rec = afl.startRecord; rec <= afl.endRecord; ++rec) {
            unsigned char p2 = (afl.sfi << 3) | 0x04;
            std::string apdu = "00B2" + toHexStr(rec) + toHexStr(p2) + "00";

            ApduCommand cmd;
            cmd.apduHex = apdu;
            cmd.sfi = afl.sfi;
            cmd.recordNumber = rec;
            targetCommands.push_back(cmd);
        }
    }

    std::cout << "\n--- Array APDU Sukses Terbentuk ---" << std::endl;
    for (size_t idx = 0; idx < targetCommands.size(); ++idx) {
        std::cout << "Index [" << idx << "] -> APDU: " << targetCommands[idx].apduHex << std::endl;
        
        // Panggil fungsi eksekusi card reader Anda
        GenerateAFL(targetCommands[idx].apduHex); 
    }
    emv.track2Data ="";
    emv.track2Data += emv.pan;
    emv.track2Data += "=";
    emv.track2Data += emv.expiryDate;
    emv.track2Data += emv.serviceCode;
    emv.track2Data += emv.discretionaryData;

    emv.TransactionSequenceCounter = getIncrementStringFromFile(namaFile);

    if(emv.countrycode == "Tidak Ditemukan"){
      emv.terminalCountry ="0360";
    }

    emv.currencyCode = "0360";

    emv.posEntryMode ="051";

    emv.terminalCountry = emv.countrycode;
    
    // mulai gen AC

    bool isPanFound = false;
    
    return 0;
}

bool CommandCardCRT::SelectCard(){

  int resp = IdentifyCard();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::string respbuf="";

  if(resp != 0){
    emv.track2Data = ReadData(2);
    emv.posEntryMode ="021";
    return SUCSESS_RESP;
  }
  // Langkah 1: Select PSE (Payment System Environment)
  // Kirim perintah untuk melihat daftar aplikasi yang didukung kartu.
  // •	Command: 00 A4 04 00 0E 31 50 41 59 2E 53 59 53 2E 44 44 46 30 31
  // •	Logic:
  // •	Jika respon 90 00, baca filenya. Anda akan mendapatkan daftar AID (misal GPN dan VISA dalam satu kartu).
  // •	Jika 6A 82 (File not found), Anda harus melakukan Direct Selection (mencoba satu per satu AID yang Anda dukung).
  // Langkah 2: Seleksi Berdasarkan Prioritas
  // Aplikasi Anda harus memiliki daftar prioritas AID di kode C++.
  // •	Contoh Daftar AID:
  // •	GPN (Indonesia): A0 00 00 06 02 10 10
  // •	VISA: A0 00 00 00 03 10 10
  // •	Mastercard: A0 00 00 00 04 10 10
  // Logic Flow:
  // 1.	Coba Select AID GPN. Sukses? Lanjut alur GPN.
  // 2.	Gagal? Coba Select AID VISA. Sukses? Lanjut alur VISA.
  // 3.	Semua gagal? Berikan pesan "Kartu tidak didukung".
  //kalau responnya 61 panjang leng harus diparsing dulu
  unsigned char SEt0;
	unsigned char SEt1;
  std::cout << "resp=>"<<resp;
  if(resp == 0){
    //cek kartu apakah gpn
    respbuf = CommandData(COMSelectPSE,&SEt0,&SEt1);
    std::printf("Res SW CommandData: %02X %02X\n", SEt0, SEt1);

    if(SEt0 == 0x61){
       std::cout << "data respon panjang lagi!";

       BYTE extraBytesLength = SEt1;
       std::vector<BYTE> currentCmd = { 0x00, 0xC0, 0x00, 0x00, extraBytesLength };
      std::string str = ConvertBytetoString(currentCmd);
       // std::stringstream ss

        // std::cout << str << std::endl; // Output: 00C00000
       respbuf = CommandData(str,&SEt0,&SEt1);
       std::string data_buffer = respbuf;
       respbuf = CariTag88(respbuf);//select AID 
       
       std::vector<unsigned char> tlvData = hexToBytes(respbuf);
        std::vector<unsigned char> aidValue;

        // Cari Tag 4F (AID) secara otomatis
        if (findTagValue(tlvData, 0, tlvData.size(), 0x4F, aidValue)) {
            std::string aidHex = "A0000006021010";//bytesToHex(aidValue);
            std::cout << "[SUKSES] AID Ditemukan: " << aidHex << std::endl;

            // --- GENERATE APDU SELECT AID ---
            unsigned char aidLen = aidValue.size();
            std::stringstream ss;
            ss << "00A40400" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)aidLen << aidHex;
            
            std::string nextApduCmd = ss.str();
            std::cout << "Kirim APDU Berikutnya (SELECT AID): " << nextApduCmd << std::endl;
          //  std::string nextApduCmd_new = selectHighestPriorityAid(data_buffer);
            respbuf = CommandData(nextApduCmd,&SEt0,&SEt1);
             if(SEt0 == 0x61){
                std::cout << "data respon panjang lagi!";

                BYTE extraBytesLength = SEt1;
                std::vector<BYTE> currentCmd = { 0x00, 0xC0, 0x00, 0x00, extraBytesLength };
                std::string str = ConvertBytetoString(currentCmd);
                respbuf = CommandData(str,&SEt0,&SEt1);
                 if (respbuf.length() >= 4 && respbuf.substr(respbuf.length() - 4) == "9000") {
                      respbuf = respbuf.substr(0, respbuf.length() - 4);
                  }

                  std::vector<unsigned char> responseBytes = hexToBytes(respbuf);

                  // Tembak ke parser rekursif untuk diekstrak secara otomatis
                  parseEMVTLV(responseBytes);

                MulaiGPO(respbuf);
             }

             else if(SEt0 == 0x90){
                std::cout<<" CariTag88 resp 90";
            }


            // Tembak nextApduCmd ini menggunakan fungsi CommandData(nextApduCmd, &sw1, &sw2) Anda!
        } else {
            std::cerr << "[ERROR] Tag 4F (AID) tidak ditemukan dalam record." << std::endl;
        }
    }
    else if(SEt0 == 0x90){
        std::cout<<"COMSelectPSE resp 90";
    }
  }

  // switch(resp){
  //   case 0:{
  //       // OnCPUAPDUSENDButton();
  //   }
  // }
  //00A404000E315041592E5359532E4444463031
  return SUCSESS_RESP;
}


std::string  CommandCardCRT::CariTag88(std::string datas){
  std::vector<unsigned char> tlvData = hexToBytes(datas);
    std::vector<unsigned char> sfiValue;

    std::cout << "--- Memulai Parsing Respon FCI ---" << std::endl;

    // Cari Tag 0x88 (SFI) di dalam data TLV
    if (findTagValue(tlvData, 0, tlvData.size(), 0x88, sfiValue)) {
        if (!sfiValue.empty()) {
            unsigned char sfi = sfiValue[0];
            std::cout << "[SUKSES] SFI Berhasil Ditemukan: " << std::hex << std::uppercase 
                      << "0x" << (int)sfi << std::dec << std::endl;

            // --- LOGIKA GENERATE COMMAND READ RECORD ---
            // Rumus P2 untuk READ RECORD: (SFI << 3) | 0x04
            unsigned char p2 = (sfi << 3) | 0x04;
            unsigned char recordNumber = 0x01; // Mulai dari record ke-1

            std::vector<unsigned char> readRecordCmd = {
                0x00,          // CLA
                0xB2,          // INS (READ RECORD)
                recordNumber,  // P1 (Record Number)
                p2,            // P2 (SFI Formatted)
                0x00           // Le (Ambil semua data)
            };

            std::cout << "\n--- Next Command (READ RECORD) ---" << std::endl;
            std::cout << "Target SFI    : " << (int)sfi << std::endl;
            std::cout << "Record Number : " << (int)recordNumber << std::endl;
            std::cout << "APDU Generated: " << bytesToHex(readRecordCmd) << std::endl;
            std::cout << "Kirim APDU di atas menggunakan fungsi RS232_ExeCommand Anda." << std::endl;
            unsigned char SEt0;
	          unsigned char SEt1;
            std::string hasil_data;
            hasil_data = CommandData(bytesToHex(readRecordCmd),&SEt0,&SEt1);

            unsigned int statusWord = (SEt0 << 8) | SEt1;

            if(SEt0==0x90){
              return hasil_data;
            }
            else if(SEt0 ==0x61){
                BYTE extraBytesLength = SEt1;
                std::vector<BYTE> currentCmd = { 0x00, 0xC0, 0x00, 0x00, extraBytesLength };
                std::string str = ConvertBytetoString(currentCmd);
                hasil_data = CommandData(str,&SEt0,&SEt1);
                return hasil_data;
            }
        //   switch (statusWord) 
        //   {
        //       case 0x9000:
        //           std::cout << "[SUKSES] Record " << (int)recordNumber << " ditemukan!" << std::endl;
        //          // std::cout << "Data Payload: " << responseHex << std::endl;
                  
        //           // TODO: Di sini Anda bisa memparsing data 'responseHex' 
        //           // untuk mencari AID aplikasi (Tag 4F / 61)
                  
        //           recordNumber++; // Naikkan ke record berikutnya (misal dari 1 ke 2)
        //           break;

        //       case 0x6A83: // Record Not Found
        //           std::cout << "[SELESAI] SW 6A83: Record " << (int)recordNumber 
        //                     << " tidak ditemukan. Semua record pada SFI ini sudah habis." << std::endl;
        //          // keepReading = false; // Keluar dari loop secara normal
        //           break;

        //       case 0x6A82: // File/SFI Not Found
        //           std::cout << "[ERROR] SW 6A82: SFI " << (int)sfi << " tidak valid pada kartu ini." << std::endl;
        //           //keepReading = false;
        //           break;

        //       default:
        //           // Jika ada kasus di mana kartu merespon 61xx (Get Response dibutuhkan)
        //           // Harusnya ini sudah di-handle otomatis di dalam fungsi CommandData Anda.
        //           // Tapi jika mendadak dapet error aneh lain (seperti 6E00, 6D00):
        //           std::cout << "[PERINGATAN] Mendapat SW tidak dikenal: " 
        //                     << std::hex << std::uppercase << statusWord 
        //                     << ". Menghentikan scan." << std::dec << std::endl;
        //           //keepReading = false;
        //           break;
        //   }
         }
    } else {
        std::cerr << "[GAGAL] Tag 88 (SFI) tidak ditemukan di dalam struktur data." << std::endl;
    }
}

// Fungsi utama untuk mencari nilai Tag di dalam struktur TLV EMV secara rekursif
bool CommandCardCRT::findTagValue(const std::vector<unsigned char>& tlvData, size_t start, size_t end, unsigned int targetTag, std::vector<unsigned char>& outValue) {
    size_t i = start;
    while (i < end) {
        if (i + 1 > end) break;

        // 1. Parsing Tag (Bisa 1 byte atau 2 byte seperti 5F2D)
        unsigned int tag = tlvData[i++];
        if ((tag & 0x1F) == 0x1F) { // Jika 5 bit terakhir berstatus 1, berarti tag 2-byte
            if (i >= end) break;
            tag = (tag << 8) | tlvData[i++];
        }

        // 2. Parsing Length (EMV Menggunakan aturan ASN.1)
        if (i >= end) break;
        size_t length = tlvData[i++];
        if (length & 0x80) { // Jika bit paling kiri aktif, berarti multibyte length descriptor
            size_t numLengthBytes = length & 0x7F;
            length = 0;
            for (size_t l = 0; l < numLengthBytes; ++l) {
                if (i >= end) return false;
                length = (length << 8) | tlvData[i++];
            }
        }

        if (i + length > end) break; // Validasi batas memori

        // 3. Evaluasi Tag
        if (tag == targetTag) {
            outValue.assign(tlvData.begin() + i, tlvData.begin() + i + length);
            return true;
        }

        // Jika tag ini adalah Constructed Tag (mengandung sub-tag di dalamnya) seperti 6F atau A5
        // Di EMV, tag bertipe constructed ditandai dengan bit ke-6 bernilai 1 (tag & 0x20)
        if (tag == 0x6F || tag == 0xA5 || (tag & 0x20)) {
            if (findTagValue(tlvData, i, i + length, targetTag, outValue)) {
                return true;
            }
        }

        i += length; // Loncat ke tag berikutnya
    }
    return false;
}

//buat parsing Select PSE (Payment System Environment) untuk dapetid AID
void CommandCardCRT::ClearData(){
    emv.expiryDate = "";
    emv.cardholderName = "";
    emv.serviceCode = "";
    emv.discretionaryData = "";
    emv.isTrack2Found = false;
 
    emv.tvr = "0000000000";// aku masih pusing kalo membatasi TVR //Tag 95
    emv.cardholderverification = "";
    emv.countrycode = ""; //Tag 5F1A sama dengan curency code
    emv.IACDefault = "";
    emv.IACDenial = "";
    emv.IACOnline = "";
    // emv.cdol1Raw = "Tidak Ditemukan";    // Aturan pengisian 29 byte transaksi
    emv.cdol2Raw = "";    // Aturan pengisian 29 byte transaksi
    emv.isPanFound = false;

    // Data Kriptogram (Dari GENERATE AC)
    emv.arqc = "";        // Tag 9F26
    emv.cid = "";         // Tag 9F27
    emv.iad = "";        // Tag 9F10
    emv.atc = "";         // Tag 9F36
    emv.unpredictableNo = "";//generate dari kita 9F37
    //emv.TerminalVerificationResults = "8080048000"; //Tag 95 hasil verivikasi //hardcode karena belum tau rumus dan kesepakatanya
     emv.TerminalVerificationResults = "0000000000"; //Tag 95 hasil verivikasi //hardcode karena belum tau rumus dan kesepakatanya
    emv.transactionDate = ""; // tag 9A (YYMMDD)
    emv.transactionType = "00"; //00 asumsi purcase butuh info dari mandiri mau transaksi tipenya apa //tag 9C //hardcode karena belum tau rumus dan kesepakatanya
    emv.ammountDeposit = "000000010000"; //Tag 9F02
    emv.ammountOther = "000000000000"; //Tag 9F03
    emv.pan = ""; //5A
    emv.currencyCode = ""; //tag 5F2A
    emv.ApplicationInterchangeProfile = ""; //Tag 82
    emv.terminalCountry = "";//Tag 9F1A
    emv.panSequenceNumber = "";        // Tag 5F34
    emv.TerminalCapabilities = ""; //9F33 //hardcode karena belum tau rumus dan kesepakatanya
    emv.CardholderVerificationMethod = ""; //9F34 //hardcode karena belum tau rumus dan kesepakatanya
    emv.TerminalType = "14"; //9F35 //hardcode karena belum tau rumus dan kesepakatanya
    emv.InterfaceDevice = "5331444D30303032"; 
    emv.DedicatedFile = "";
    emv.ApplicationVersionNumber = "01"; //hardcode karena belum tau rumus dan kesepakatanya
    emv.TransactionSequenceCounter = "0";
    emv.IccFull = "";
    emv.track2Data = "";
    emv.posEntryMode ="";
    emv.isAcSuccess = false;
    emv.ApplicationCurrencyExponent = "0";//0x9F44
    StatusKartu =0;
    emv.priority="0";
}

int CommandCardCRT::JenisKartu(JenisKartuResponse *datass){

ClearData();//normalisasi data

int resp = IdentifyCard();
if(resp == SUCSESS_RESP){

  datass->jenis = "051";
  datass->status = SUCSESS_RESP;
  StatusKartu =1;
}
else if(resp == FAIL_RESP ){

  //syaratnya kalo data track 2 sudah dapat
  datass->jenis = "021";
  datass->status = SUCSESS_RESP;
  StatusKartu =2;
}
else{//card reader gagal
  datass->jenis = "";
  datass->status = FAIL_RESP;
  StatusKartu =-1;
}
std::this_thread::sleep_for(std::chrono::seconds(1));

//ini cukup nanti dilanjut sama high level buat ambil datanya;

  return resp;
}

std::string intToDecimal12Char(int value) {
    std::stringstream stream;
    stream << std::dec          // Format Desimal biasa
           << std::setw(12)     // Lebar 12 karakter
           << std::setfill('0') // Padding nol di depan
           << value;
    return stream.str();
}


int CommandCardCRT::GetDataKartu(int ammount_data, int flag, DataKartuResponse *datass){
  ClearData();
  emv.ammountDeposit = intToDecimal12Char(ammount_data);
  std::cout << "==========set deposit>>"<< emv.ammountDeposit << std::endl;
  int resp = 0;
  if(flag == 0){
      resp = IdentifyCard();
  }
  else if(flag == 1){
      resp = 1;
  }
  else if(flag == 2){
    resp = 0;
  }
  else if(flag == 3){
    resp = 3;
  }
  std::this_thread::sleep_for(std::chrono::seconds(2));

  if(resp == SUCSESS_RESP){
    //normalisasi data
    std::string respbuf="";
    unsigned char SEt0;
    unsigned char SEt1;
    //cek kartu apakah gpn
    respbuf = CommandData(COMSelectPSE,&SEt0,&SEt1);
    std::printf("Res SW CommandData: %02X %02X\n", SEt0, SEt1);

    if(SEt0 == 0x61){
       std::cout << "data respon panjang lagi!";

       BYTE extraBytesLength = SEt1;
       std::vector<BYTE> currentCmd = { 0x00, 0xC0, 0x00, 0x00, extraBytesLength };
       std::string str = ConvertBytetoString(currentCmd);
       // std::stringstream ss

        // std::cout << str << std::endl; // Output: 00C00000
       respbuf = CommandData(str,&SEt0,&SEt1);
      //  std::string datanya_apa = selectHighestPriorityAid(respbuf);
      //  bool pakai_def = true;
      //  if(datanya_apa.size() > 10 && datanya_apa != ""){
      //     pakai_def=false;
      //     std::cout << "pakai_def: " << pakai_def <<"|size: " << datanya_apa.size()<< std::endl;
      //  }
       //tambahkan logika sukses dan fail

       respbuf = CariTag88(respbuf);//select AID 

       std::vector<unsigned char> tlvData = hexToBytes(respbuf);
       std::vector<unsigned char> aidValue;

        // Cari Tag 4F (AID) secara otomatis
        if (findTagValue(tlvData, 0, tlvData.size(), 0x4F, aidValue)) {
          //  if(!pakai_def){
          //       respbuf = CommandData(datanya_apa,&SEt0,&SEt1);
          //       // emv.DedicatedFile = "A0000006021010";
          //       std::cout << "masuk pakai_def: " << pakai_def << std::endl;
          //  }else{
                //std::cout << "pakai_def: " << pakai_def << std::endl;
                std::string aidHex = bytesToHex(aidValue);
               // if(emv.priority == "02"){
                  aidHex = "A0000006021010";
                  emv.DedicatedFile = "A0000006021010";
              //  }
                std::cout << "[SUKSES] AID Ditemukan: " << aidHex << std::endl;

                // --- GENERATE APDU SELECT AID ---
                unsigned char aidLen = aidValue.size();
                std::stringstream ss;
                ss << "00A40400" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)aidLen << aidHex;
                
                std::string nextApduCmd = ss.str();
                std::cout << "Kirim APDU Berikutnya (SELECT AID): " << nextApduCmd << std::endl;
                respbuf = CommandData(nextApduCmd,&SEt0,&SEt1);
          // }
             if(SEt0 == 0x61){
                std::cout << "data respon panjang lagi!";

                BYTE extraBytesLength = SEt1;
                std::vector<BYTE> currentCmd = { 0x00, 0xC0, 0x00, 0x00, extraBytesLength };
                std::string str = ConvertBytetoString(currentCmd);
                respbuf = CommandData(str,&SEt0,&SEt1);
                 if (respbuf.length() >= 4 && respbuf.substr(respbuf.length() - 4) == "9000") {
                      respbuf = respbuf.substr(0, respbuf.length() - 4);
                  }

                  std::vector<unsigned char> responseBytes = hexToBytes(respbuf);

                  // Tembak ke parser rekursif untuk diekstrak secara otomatis
                  parseEMVTLV(responseBytes);

                  MulaiGPO(respbuf);

                  if(emv.cdol1Raw != "Tidak Ditemukan"){
                  std::cout << "mulai gen ac!" <<std::endl;
                    GenerateACCommand(emv.cdol1Raw);
                  }
                  else{
                    std::cout << "cdol1 kosong" <<std::endl;
                  }
             }

             else if(SEt0 == 0x90){
                std::cout<<" CariTag88 resp 90 -->>>> harusnya ini gak ada soalnya commandnya di belakang nggak ada tambahan 00";
            }


            // Tembak nextApduCmd ini menggunakan fungsi CommandData(nextApduCmd, &sw1, &sw2) Anda!
        } else {
            std::cerr << "[ERROR] Tag 4F (AID) tidak ditemukan dalam record." << std::endl;
            return FAIL_RESP;
        }
    }
    else if(SEt0 == 0x90){
        std::cout<<"COMSelectPSE resp 90";//harusnya nggak masuk sini soalnya command nya untuk 0x61 ya
    }
      // tambahkan respon gagal
    datass->cardnumber = emv.pan;
    datass->track2data = emv.track2Data;
    datass->modecard = "051";
    datass->tag5F34 = emv.panSequenceNumber;
    datass->iccdata = emv.IccFull;
    datass->status = SUCSESS_RESP;

      std::cout << "================final data sukses ICC=====================" << std::endl;
          std::cout << "cardnumber  : " << datass->cardnumber << std::endl;
          std::cout << "track2data  : " << datass->track2data << std::endl;
          std::cout << "modecard    : " << datass->modecard << std::endl;
          std::cout << "tag5F34     : " << datass->tag5F34 << std::endl;
          std::cout << "iccdata     : " << datass->iccdata << std::endl;
          std::cout << "status      : " << datass->status << std::endl;
    
    if(SEt0 == 0x6A && SEt1 == 0x82){
      std::cout << "================final data cek magnitude=====================" << std::endl;
      CardData datsa;
    datsa = CheckMAgData();
      datass->cardnumber = datsa.track2;//emv.pan;
    datass->track2data = datsa.track2;//emv.track2Data;
    datass->modecard = "021";
    datass->tag5F34 = "";
    datass->iccdata = "";
    datass->status = SUCSESS_RESP;

    }

    return SUCSESS_RESP;
  }
  else if(resp == FAIL_RESP){
    CardData datsa;
    datsa = CheckMAgData();
    datass->cardnumber = datsa.track2;//emv.pan;
    datass->track2data = datsa.track2;//emv.track2Data;
    datass->modecard = "021";
    datass->tag5F34 = "";
    datass->iccdata = "";
    datass->status = FAIL_RESP;

          std::cout << "================final data sukses data magnitude=====================" << std::endl;
          std::cout << "cardnumber  : " << datass->cardnumber << std::endl;
          std::cout << "track2data  : " << datass->track2data << std::endl;
          std::cout << "modecard    : " << datass->modecard << std::endl;
          std::cout << "tag5F34     : " << datass->tag5F34 << std::endl;
          std::cout << "iccdata     : " << datass->iccdata << std::endl;
          std::cout << "status      : " << datass->status << std::endl;

    return SUCSESS_RESP;
  }
  else if(resp == 3){
    if(emv.cdol1Raw != "Tidak Ditemukan"){
      std::cout << "mulai gen ac!" <<std::endl;
        GenerateACCommand(emv.cdol1Raw);
      }
      else{
        std::cout << "cdol1 kosong" <<std::endl;
      }
      datass->iccdata = emv.IccFull;
      datass->status = FAIL_RESP;

            std::cout << "================final data tidk ditemukan=====================" << std::endl;
          std::cout << "cardnumber  : " << datass->cardnumber << std::endl;
          std::cout << "track2data  : " << datass->track2data << std::endl;
          std::cout << "modecard    : " << datass->modecard << std::endl;
          std::cout << "tag5F34     : " << datass->tag5F34 << std::endl;
          std::cout << "iccdata     : " << datass->iccdata << std::endl;
          std::cout << "status      : " << datass->status << std::endl;
          
      return SUCSESS_RESP;
  }
  else{

    return FAIL_RESP;
  }

  return SUCSESS_RESP;

}

