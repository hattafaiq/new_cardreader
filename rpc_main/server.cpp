#include <grpcpp/support/status.h>
#include <string>

#include "serial_command_cr.h"
#include "server.h"
#include "clienthttp.h"

Server::Server() : m_handle(new CommandCardCRT()) {}

Server::~Server() { m_handle->Close(); }

::grpc::Status Server::IsOpen(::grpc::ServerContext* context,
                              const ::card_reader_crt::IsOpenRequest* request,
                              ::card_reader_crt::IsOpenResp* response) {
  bool resp = m_handle->IsOpen();
  response->set_status(resp);
  return ::grpc::Status::OK;
}

::grpc::Status Server::Open(::grpc::ServerContext* context,
                            const ::card_reader_crt::OpenRequest* request,
                            ::card_reader_crt::OpenResp* response) {
  bool status = false;
  if (!request->com().empty()) {
    status = m_handle->Open(request->com(),m_bautrate_servis);
  } else {
    status = m_handle->Open(m_com_servis,m_bautrate_servis);
  }
  response->set_status(status);
  return ::grpc::Status::OK;
}

::grpc::Status Server::GetDataKartu(::grpc::ServerContext* context,
                            const ::card_reader_crt::GetDataKartuRequest* request,
                            ::card_reader_crt::GetDataKartuResponse* response) {
  bool status = false;
   DataKartuResponse dat;
   status = m_handle->GetDataKartu(request->ammount(), request->flag(), &dat);
   response->set_cardnumber(dat.cardnumber); //dat.cardnumber
   response->set_iccdata(dat.iccdata);
   response->set_modecard(dat.modecard);
   response->set_tag5f34(dat.tag5F34);
   response->set_track2data(dat.track2data);
   response->set_status(dat.status);
  return ::grpc::Status::OK;
}

::grpc::Status Server::Close(::grpc::ServerContext* context,
                             const ::card_reader_crt::CloseRequest* request,
                             ::card_reader_crt::CloseResp* response) {
  if (m_handle->IsOpen()) {
    m_handle->Close();
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::Init(::grpc::ServerContext* context,
                            const ::card_reader_crt::InitRequest* request,
                            ::card_reader_crt::InitResponse* response) {
  // dat.cardnumber = "";
	// dat.track2data = "";
	// dat.modecard = "";
	// dat.tag5F34 = "";
	// dat.iccdata = "";
	// dat.status = false;
   
  if (m_handle->IsOpen()) {
    bool resp = m_handle->Init(request->ejectfrontorrear());
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}
::grpc::Status Server::PermitMagCardOnly(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::PermitMagCardOnlyRequest* request,
    ::card_reader_crt::PermitMagCardOnlyResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->PermitMagCardOnly();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::ProhibitCardIn(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::ProhibitCardInRequest* request,
    ::card_reader_crt::ProhibitCardInResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->ProhibitCardIn();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::Eject(::grpc::ServerContext* context,
                             const ::card_reader_crt::EjectRequest* request,
                             ::card_reader_crt::EjectResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->Eject();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}
::grpc::Status Server::Capture(::grpc::ServerContext* context,
                               const ::card_reader_crt::CaptureRequest* request,
                               ::card_reader_crt::CaptureResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->Capture();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::Retrieve(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::RetrieveRequest* request,
    ::card_reader_crt::RetrieveResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->Retrieve();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::CardStatus(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::CardStatusRequest* request,
    ::card_reader_crt::CardStatusResponse* response) {
  if (m_handle->IsOpen()) {
    auto resp = m_handle->CardStatus();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::LEDBlinkGreen(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::LEDBlinkGreenRequest* request,
    ::card_reader_crt::LEDBlinkGreenResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->LEDBlinkGreen();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::LEDBlinkOrange(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::LEDBlinkOrangeRequest* request,
    ::card_reader_crt::LEDBlinkOrangeResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->LEDBlinkOrange();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::LEDBlinkRed(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::LEDBlinkRedRequest* request,
    ::card_reader_crt::LEDBlinkRedResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->LEDBlinkRed();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::LEDOFF(::grpc::ServerContext* context,
                              const ::card_reader_crt::LEDOFFRequest* request,
                              ::card_reader_crt::LEDOFFResponse* response) {
  if (m_handle->IsOpen()) {
    bool resp = m_handle->LEDOFF();
    response->set_status(resp);
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::ReadData(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::ReadDataRequest* request,
    ::card_reader_crt::ReadDataResponse* response) {
  if (m_handle->IsOpen()) {
    uint8_t mag = 0;

    auto data = m_handle->EncodeMagDataStatus(&mag);
    std::bitset<8> bitmag = mag;
    int status = m_handle->CardStatus();

    if (status == 2 && bitmag.test(0)) {
      auto data1 = m_handle->ReadData(1);
      response->set_record1(data1);
    }
    if (status == 2 && bitmag.test(1)) {
      auto data1 = m_handle->ReadData(2);
      response->set_record2(data1);
    }
    if (status == 2 && bitmag.test(2)) {
      auto data1 = m_handle->ReadData(3);
      response->set_record3(data1);
    }
  }
  return ::grpc::Status::OK;
}

::grpc::Status Server::WaitInputReadCard(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::WaitInputReadCardRequest* request,
    ::card_reader_crt::WaitInputReadCardResponse* response) {
  int status = 0;
  auto data = m_handle->ReadCard(request->timeout(), &status);
  response->set_status(status);
  response->set_record1(data.track1);
  response->set_record2(data.track2);
  response->set_record3(data.track3);
  return ::grpc::Status::OK;
}

::grpc::Status Server::RejectCardOrMoveBin(
    ::grpc::ServerContext* context,
    const ::card_reader_crt::RejectCardOrMoveBinRequest* request,
    ::card_reader_crt::RejectCardOrMoveBinResponse* response) {
  auto resp = m_handle->RejectCardOrMoveBin(request->timeout());
  switch (resp) {
    case -1: {
      response->set_status(
          ::card_reader_crt::RejectCardOrMoveBinStatus::NOT_OPEN_DEVICE);
    } break;
    case -2: {
      response->set_status(
          ::card_reader_crt::RejectCardOrMoveBinStatus::CARD_NOT_DETECTED);
    } break;
    case 1: {
      response->set_status(
          ::card_reader_crt::RejectCardOrMoveBinStatus::CARD_MOVE_TO_BIN);
    } break;
    case 0: {
      response->set_status(
          ::card_reader_crt::RejectCardOrMoveBinStatus::CARD_TAKEN);
    }break;
    default:
      break;
  }
  return ::grpc::Status::OK;
}

void Server::RunOpenPinpad(std::string_view host, int port)
{
  HttpClient cl;
  cl.timeout = 9000;
  cl.is_post_or_get = false;
  std::string url="http://";
  url += host;
  url +=":";
  url += std::to_string(port);
  url +="/OpenPinPad";
  cl.target = url;
  cl.is_post_or_get = true;
  cl.headers["Content-Type"] = "application/json";
  cl.runwt();
  printf("\nresp type %d with open pinpad: %s ",cl.res_type,cl.response.c_str());  
}