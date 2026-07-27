#ifndef SERVER_CARD_H
#define SERVER_CARD_H

#include "card.grpc.pb.h"
#include "card.pb.h"
#include <grpc++/server.h>
#include <memory>

class CommandCardCRT; 
class Server: public card_reader_crt::CardReader::Service{
public:
    explicit Server();
    ~Server();

    virtual ::grpc::Status IsOpen(::grpc::ServerContext* context, const ::card_reader_crt::IsOpenRequest* request, ::card_reader_crt::IsOpenResp* response);
    virtual ::grpc::Status Open(::grpc::ServerContext* context, const ::card_reader_crt::OpenRequest* request, ::card_reader_crt::OpenResp* response);
    virtual ::grpc::Status Close(::grpc::ServerContext* context, const ::card_reader_crt::CloseRequest* request, ::card_reader_crt::CloseResp* response);
    virtual ::grpc::Status Init(::grpc::ServerContext* context, const ::card_reader_crt::InitRequest* request, ::card_reader_crt::InitResponse* response);
    virtual ::grpc::Status PermitMagCardOnly(::grpc::ServerContext* context, const ::card_reader_crt::PermitMagCardOnlyRequest* request, ::card_reader_crt::PermitMagCardOnlyResponse* response);
    virtual ::grpc::Status ProhibitCardIn(::grpc::ServerContext* context, const ::card_reader_crt::ProhibitCardInRequest* request, ::card_reader_crt::ProhibitCardInResponse* response);
    virtual ::grpc::Status Eject(::grpc::ServerContext* context, const ::card_reader_crt::EjectRequest* request, ::card_reader_crt::EjectResponse* response);
    virtual ::grpc::Status Capture(::grpc::ServerContext* context, const ::card_reader_crt::CaptureRequest* request, ::card_reader_crt::CaptureResponse* response);
    virtual ::grpc::Status Retrieve(::grpc::ServerContext* context, const ::card_reader_crt::RetrieveRequest* request, ::card_reader_crt::RetrieveResponse* response);
    virtual ::grpc::Status CardStatus(::grpc::ServerContext* context, const ::card_reader_crt::CardStatusRequest* request, ::card_reader_crt::CardStatusResponse* response);
    virtual ::grpc::Status LEDBlinkGreen(::grpc::ServerContext* context, const ::card_reader_crt::LEDBlinkGreenRequest* request, ::card_reader_crt::LEDBlinkGreenResponse* response);
    virtual ::grpc::Status LEDBlinkOrange(::grpc::ServerContext* context, const ::card_reader_crt::LEDBlinkOrangeRequest* request, ::card_reader_crt::LEDBlinkOrangeResponse* response);
    virtual ::grpc::Status LEDBlinkRed(::grpc::ServerContext* context, const ::card_reader_crt::LEDBlinkRedRequest* request, ::card_reader_crt::LEDBlinkRedResponse* response);
    virtual ::grpc::Status LEDOFF(::grpc::ServerContext* context, const ::card_reader_crt::LEDOFFRequest* request, ::card_reader_crt::LEDOFFResponse* response);
    virtual ::grpc::Status ReadData(::grpc::ServerContext* context, const ::card_reader_crt::ReadDataRequest* request, ::card_reader_crt::ReadDataResponse* response);
    virtual ::grpc::Status WaitInputReadCard(::grpc::ServerContext* context, const ::card_reader_crt::WaitInputReadCardRequest* request, ::card_reader_crt::WaitInputReadCardResponse* response);
    virtual ::grpc::Status RejectCardOrMoveBin(::grpc::ServerContext* context, const ::card_reader_crt::RejectCardOrMoveBinRequest* request, ::card_reader_crt::RejectCardOrMoveBinResponse* response);
     virtual ::grpc::Status GetDataKartu(::grpc::ServerContext* context, const ::card_reader_crt::GetDataKartuRequest* request, ::card_reader_crt::GetDataKartuResponse* response);

    std::string m_com_servis;
    int m_bautrate_servis;
    void RunOpenPinpad(std::string_view host, int port);
private:
    std::unique_ptr<CommandCardCRT> m_handle;
    
};

#endif