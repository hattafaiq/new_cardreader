#ifndef CARD_READER_H
#define CARD_READER_H

#include <QObject>
#include <QThread>
#include <memory>
#include <qobjectdefs.h>
#include <qthread.h>

class CardReader: public QObject 
{
Q_OBJECT
public:
    explicit CardReader();
    ~CardReader();
    void setup(const QString &com, int port);

public Q_SLOTS:
      void Open();
    void Close();
    void IsOpen();
    void CardStatus();
    void WaitInputReadCard();
    void RejectCardOrMoveToBin();
    void Init();
    void PermitMagCardOnly();
    void ProhibitCardIn();
    void LEDOFF();
    void LEDBlinkGreen();
    void ReadData();
    void Eject();
    void Retrieve();
    void Capture();

Q_SIGNALS:
    void OnCapture(bool status);
    void OnEject(bool status);
    void OnRetrieve(bool status);
    void OnReadData(const QString &data);
    void OnPermitMagCardOnly(bool status);
    void OnProhibitCardIn(bool status);
    void OnLEDOFF(bool status);
    void OnLEDBlinkGreen(bool status);
    void OnInit(bool status);
    void OnRejectCardOrMoveToBin(int status);
    void OnWaitInputReadCard(int status,
        const QString &record1, 
        const QString &record2,
        const QString &record3);
    void OnCardStatus(int status);
    void OnIsOpen(bool status);
    void OnClose(bool status);
    void OnOpen(bool status);
private:
  QString m_com;
  int m_port = 0;
};

class CardReaderMgr: public QObject {
Q_OBJECT
public:
    explicit CardReaderMgr();
    ~CardReaderMgr();

    void EndThread();
    void setup(const QString &com, int port);

    void Open();
    void Close();
    void IsOpen();
    void CardStatus();
    void WaitInputReadCard();
    void RejectCardOrMoveToBin();
     void Init();
    void PermitMagCardOnly();
    void ProhibitCardIn();
    void LEDOFF();
    void LEDBlinkGreen();
    void ReadData();
    void Eject();
    void Retrieve();
    void Capture();

Q_SIGNALS:
    void OnCapture(bool status);
    void OnEject(bool status);
    void OnRetrieve(bool status);
    void OnReadData(const QString &data);
    void OnPermitMagCardOnly(bool status);
    void OnProhibitCardIn(bool status);
    void OnLEDOFF(bool status);
    void OnLEDBlinkGreen(bool status);
    void OnInit(bool status);
    void OnRejectCardOrMoveToBin(int status);
    void OnWaitInputReadCard(int status,
        const QString &record1, 
        const QString &record2,
        const QString &record3);
    void OnCardStatus(int status);
    void OnIsOpen(bool status);
    void OnClose(bool status);
    void OnOpen(bool status);

    //priv trigger
    void TriggerInit(QPrivateSignal);
    void TriggerCapture(QPrivateSignal);
    void TriggerEject(QPrivateSignal);
    void TriggerRetrieve(QPrivateSignal);
    void TriggerReadData(QPrivateSignal);
    void TriggerPermitMagCardOnly(QPrivateSignal);
    void TriggerProhibitCardIn(QPrivateSignal);
    void TriggerLEDOFF(QPrivateSignal);
    void TriggerLEDBlinkGreen(QPrivateSignal);
    void TriggerOpen(QPrivateSignal);
    void TriggerClose(QPrivateSignal);
    void TriggerIsOpen(QPrivateSignal);
    void TriggerCardStatus(QPrivateSignal);
    void TriggerWaitInputReadCard(QPrivateSignal);
    void TriggerRejectCardOrMoveToBin(QPrivateSignal);
private:
    QThread m_thread;
    std::unique_ptr<CardReader> m_worker;
};

#endif