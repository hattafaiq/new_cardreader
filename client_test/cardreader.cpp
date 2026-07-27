#include <chrono>
#include <memory>
#include <sstream>

#include "card.grpc.pb.h"
#include "card.pb.h"
#include "cardreader.h"
#include "grpcpp/channel.h"
#include "grpcpp/client_context.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/support/channel_arguments.h"


class CardService {
 public:
  explicit CardService(std::shared_ptr<grpc::Channel> channel)
      : _stub(card_reader_crt::CardReader::NewStub(channel)) {}
  ~CardService() {}

  void setup(std::string_view com, int port) {
    m_com = com;
    m_port = port;
  }

  bool PermitMagCardOnly()
  {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::PermitMagCardOnlyRequest req;
    card_reader_crt::PermitMagCardOnlyResponse resp;
    auto m_status = _stub->PermitMagCardOnly(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

  bool ProhibitCardIn(){
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::ProhibitCardInRequest req;
    card_reader_crt::ProhibitCardInResponse resp;
    auto m_status = _stub->ProhibitCardIn(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

  bool LEDOFF(){
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::LEDOFFRequest req;
    card_reader_crt::LEDOFFResponse resp;
    auto m_status = _stub->LEDOFF(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

  bool LEDBlinkGreen(){
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::LEDBlinkGreenRequest req;
    card_reader_crt::LEDBlinkGreenResponse resp;
    auto m_status = _stub->LEDBlinkGreen(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }
  
  std::string WaitInputReadCard() {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::WaitInputReadCardRequest req;
    card_reader_crt::WaitInputReadCardResponse resp;
    auto m_status = _stub->WaitInputReadCard(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.SerializeAsString();
    }
    return "";
  }

  bool Eject(){
     grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::EjectRequest req;
    card_reader_crt::EjectResponse resp;
    auto m_status = _stub->Eject(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

  bool Retrieve()
  {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::RetrieveRequest req;
    card_reader_crt::RetrieveResponse resp;
    auto m_status = _stub->Retrieve(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

  bool Capture(){
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::CaptureRequest req;
    card_reader_crt::CaptureResponse resp;
    auto m_status = _stub->Capture(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

   std::string ReadData()
  {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);
    card_reader_crt::ReadDataRequest req;
    card_reader_crt::ReadDataResponse resp;
    auto m_status = _stub->ReadData(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.record2(); 
    }
    return "";
  }

  std::string Open() {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);
    card_reader_crt::OpenRequest req;
    req.set_com(m_com);
    card_reader_crt::OpenResp resp;
    auto status = _stub->Open(&ctx, req, &resp);
    if (status.ok()) {
      if (resp.status()) {
        return "1";
      }
      return "0";
    }
    return "";
  }

  std::string Close() {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::CloseRequest req;
    card_reader_crt::CloseResp resp;
    auto status = _stub->Close(&ctx, req, &resp);
    if (status.ok()) {
      return "1";
    }
    return "";
  }

  std::string IsOpen() {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::IsOpenRequest req;
    card_reader_crt::IsOpenResp resp;
    auto status = _stub->IsOpen(&ctx, req, &resp);
    if (status.ok()) {
      if (resp.status()) {
        return "1";
      }
      return "0";
    }
    return "";
  }

  int CardStatus() {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::CardStatusRequest req;
    card_reader_crt::CardStatusResponse resp;
    auto status = _stub->CardStatus(&ctx, req, &resp);
    if (status.ok()) {
      return resp.status();
    }
    return -1;
  }

  int RejectCardOrMoveToBin() {
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::RejectCardOrMoveBinRequest req;
    card_reader_crt::RejectCardOrMoveBinResponse resp;
    auto m_status = _stub->RejectCardOrMoveBin(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    }
    return -1;
  }

  bool Init(){
    grpc::ClientContext ctx;
    auto max = std::chrono::system_clock::now() + std::chrono::seconds(5);
    ctx.set_deadline(max);

    card_reader_crt::InitRequest req;
    req.set_ejectfrontorrear(true);
    card_reader_crt::InitResponse resp;
    auto m_status = _stub->Init(&ctx, req, &resp);
    if (m_status.ok()) {
      return resp.status();
    } 
    return false;
  }

 private:
  std::unique_ptr<card_reader_crt::CardReader::Stub> _stub;
  std::string m_com;
  int m_port;
};

void CardReader::Init()
{
   grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.Init();
  Q_EMIT OnInit(stat);
}

CardReader::CardReader() {}

CardReader::~CardReader() {}

void CardReader::setup(const QString &com, int port) {
  m_com = com;
  m_port = port;
}

void CardReader::Open() {
  grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.Open();

  Q_EMIT OnOpen(stat == "1" ? true : false);
}

void CardReader::Close() {
  grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.Close();
  Q_EMIT OnClose(stat == "1" ? true : false);
}

void CardReader::IsOpen() {
  grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.IsOpen();
  Q_EMIT OnIsOpen(stat == "1" ? true : false);
}

void CardReader::CardStatus() {
  grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.CardStatus();
  Q_EMIT OnCardStatus(stat);
}

void CardReader::WaitInputReadCard() {
  grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.WaitInputReadCard();
  card_reader_crt::WaitInputReadCardResponse resp;
  resp.ParseFromString(stat);
  Q_EMIT OnWaitInputReadCard(resp.status(), resp.record1().c_str(),
                             resp.record2().c_str(), resp.record3().c_str());
}

void CardReader::RejectCardOrMoveToBin() {
  grpc::ChannelArguments args;
  args.SetMaxSendMessageSize(-1);
  std::string target_str = "127.0.0.1";
  target_str += ":";
  target_str += std::to_string(this->m_port);
  CardService serv(grpc::CreateCustomChannel(
      target_str, grpc::InsecureChannelCredentials(), args));

  serv.setup(this->m_com.toStdString(), this->m_port);
  auto stat = serv.RejectCardOrMoveToBin();
  Q_EMIT OnRejectCardOrMoveToBin(stat);
}

CardReaderMgr::CardReaderMgr() : QObject() {
  m_worker.reset(new CardReader);
  connect(m_worker.get(), &CardReader::OnOpen, this, &CardReaderMgr::OnOpen);
  connect(m_worker.get(), &CardReader::OnIsOpen, this,
          &CardReaderMgr::OnIsOpen);
  connect(m_worker.get(), &CardReader::OnClose, this, &CardReaderMgr::OnClose);
  connect(m_worker.get(), &CardReader::OnCardStatus, this,
          &CardReaderMgr::OnCardStatus);
  connect(m_worker.get(), &CardReader::OnWaitInputReadCard, this,
          &CardReaderMgr::OnWaitInputReadCard);
  connect(m_worker.get(), &CardReader::OnInit,this,
          &CardReaderMgr::OnInit);
  connect(m_worker.get(), &CardReader::OnPermitMagCardOnly,this,
          &CardReaderMgr::OnPermitMagCardOnly);
  connect(m_worker.get(), &CardReader::OnProhibitCardIn,  this,
          &CardReaderMgr::OnProhibitCardIn);
  connect(m_worker.get(),&CardReader::OnLEDOFF, this,
              &CardReaderMgr::OnLEDOFF);
  connect(m_worker.get(), &CardReader::OnLEDBlinkGreen, this,
          &CardReaderMgr::OnLEDBlinkGreen);
  connect(m_worker.get(), &CardReader::OnReadData, this, 
    &CardReaderMgr::OnReadData);
  connect(m_worker.get(), &CardReader::OnEject, this, 
    &CardReaderMgr::OnEject);
  connect(m_worker.get(), &CardReader::OnRetrieve, this, 
    &CardReaderMgr::OnRetrieve);
  connect(m_worker.get(), &CardReader::OnCapture, this, 
    &CardReaderMgr::OnCapture);
  connect(m_worker.get(),&CardReader::OnRejectCardOrMoveToBin,this,
          &CardReaderMgr::OnRejectCardOrMoveToBin);

  connect(this, &CardReaderMgr::TriggerOpen,m_worker.get(),&CardReader::Open);
  connect(this, &CardReaderMgr::TriggerIsOpen,m_worker.get(), &CardReader::IsOpen);
  connect(this, &CardReaderMgr::TriggerClose, m_worker.get(), &CardReader::Close);
  connect(this, &CardReaderMgr::TriggerCardStatus, m_worker.get(), &CardReader::CardStatus);
  connect(this, &CardReaderMgr::TriggerWaitInputReadCard, m_worker.get(), &CardReader::WaitInputReadCard);
  connect(this, &CardReaderMgr::TriggerRejectCardOrMoveToBin, m_worker.get(), &CardReader::RejectCardOrMoveToBin);
  connect(this, &CardReaderMgr::TriggerInit,m_worker.get(), &CardReader::Init);
  connect(this, &CardReaderMgr::TriggerPermitMagCardOnly, m_worker.get(), &CardReader::PermitMagCardOnly);
  connect(this, &CardReaderMgr::TriggerProhibitCardIn, m_worker.get(), &CardReader::ProhibitCardIn );
  connect(this, &CardReaderMgr::TriggerLEDOFF, m_worker.get(), &CardReader::LEDOFF);
  connect(this, &CardReaderMgr::TriggerLEDBlinkGreen, m_worker.get(), &CardReader::LEDBlinkGreen);
  connect(this, &CardReaderMgr::TriggerReadData, m_worker.get(), &CardReader::ReadData);
  connect(this, &CardReaderMgr::TriggerEject, m_worker.get(), &CardReader::Eject);
  connect(this, &CardReaderMgr::TriggerRetrieve,m_worker.get(), &CardReader::Retrieve );
  connect(this,&CardReaderMgr::TriggerCapture,m_worker.get(), &CardReader::Capture);

  m_worker->moveToThread(&m_thread);
}

CardReaderMgr::~CardReaderMgr() { EndThread(); }

void CardReaderMgr::EndThread() {
  if (m_thread.isRunning()) {
    m_thread.exit();
    m_thread.wait();
  }
}

void CardReaderMgr::setup(const QString &com, int port) {
  m_worker->setup(com, port);
}

void CardReaderMgr::Open() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerOpen(QPrivateSignal());
}

void CardReaderMgr::Close() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerClose(QPrivateSignal());
}

void CardReaderMgr::IsOpen() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerIsOpen(QPrivateSignal());
}

void CardReaderMgr::CardStatus() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerCardStatus(QPrivateSignal());
}

void CardReaderMgr::WaitInputReadCard() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerWaitInputReadCard(QPrivateSignal());
}

void CardReaderMgr::RejectCardOrMoveToBin() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerRejectCardOrMoveToBin(QPrivateSignal());
}

void CardReaderMgr::Init(){
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerInit(QPrivateSignal());
}

void CardReaderMgr::PermitMagCardOnly(){
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerPermitMagCardOnly(QPrivateSignal());
}

void CardReaderMgr::ProhibitCardIn()
{
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerProhibitCardIn(QPrivateSignal());
}

void CardReaderMgr::LEDOFF()
{
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerLEDOFF(QPrivateSignal());
}

void CardReaderMgr::LEDBlinkGreen()
{
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerLEDBlinkGreen(QPrivateSignal());
}

void CardReaderMgr::ReadData() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerReadData(QPrivateSignal());
}

void CardReaderMgr::Eject() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerEject(QPrivateSignal());
}
void CardReaderMgr::Retrieve() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerRetrieve(QPrivateSignal());
}
void CardReaderMgr::Capture() {
  if (!m_thread.isRunning()) {
    m_thread.start();
  }
  Q_EMIT TriggerCapture(QPrivateSignal());
}
