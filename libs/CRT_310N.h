#define Bad_CommOpen -101//端口打开错.
#define Bad_CommClose -105//端口关闭错.

#define LPERR		-4  //数据包长度错误
#define CPERR		-3  //命令参数错误
#define CMERR		-2  //命令字错误
#define ERR		-1
#define OK		0

#define ENQ  0x05//请求连接通信线路(询问).
#define ACK  0x06//确认(握手).
#define NAK  0x15//通信忙.
#define EOT  0x04//通信结束(传送结束).
#define CAN  0x18//解除通信(取消).
#define STX  0x02//数据包起始符(起始字符).
#define ETX  0x03//数据包结束符(终结符).
#define US   0x1F//数据分隔符.


int APIENTRY GetSysVerion(HANDLE ComHandle, char *strVerion);
HANDLE APIENTRY CRT310NUOpen();
int APIENTRY CRT310NUClose(HANDLE ComHandle);
int APIENTRY GetDeviceCapabilities(HANDLE ComHandle, int *_InputReportByteLength, int *_OutputReportByteLength);
int APIENTRY ReadReport(HANDLE ComHandle,  BYTE _ReportData[],BYTE _ReportLen);
int APIENTRY WriteReport(HANDLE ComHandle,  BYTE _ReportData[],BYTE _ReportLen);
int APIENTRY USB_ExeCommand(HANDLE ComHandle,BYTE TxCmCode,BYTE TxPmCode,int TxDataLen,BYTE TxData[],BYTE *RxReplyType,BYTE *RxStCode1,BYTE *RxStCode0,int *RxDataLen,BYTE RxData[]);
int APIENTRY USB_ICCardTransmit(HANDLE ComHandle,BYTE TxAddr,BYTE TxCmCode,BYTE TxPmCode,int TxDataLen,BYTE TxData[],BYTE *RxReplyType,BYTE *RxCmCode,BYTE *RxPmCode,BYTE *RxStCode1,BYTE *RxStCode0,int *RxDataLen,BYTE RxData[]);

HANDLE APIENTRY CRT310NROpen(char *Port);
HANDLE APIENTRY CRT310NROpenWithBaut(char *Port, unsigned int Baudrate);
int APIENTRY CRT310NRClose(HANDLE ComHandle);
int APIENTRY RS232_ExeCommand(HANDLE ComHandle,BYTE TxCmCode,BYTE TxPmCode,int TxDataLen,BYTE TxData[],BYTE *RxReplyType,BYTE *RxStCode1,BYTE *RxStCode0,int *RxDataLen,BYTE RxData[]);
int APIENTRY RS232_ICCardTransmit(HANDLE ComHandle,BYTE TxAddr,BYTE TxCmCode,BYTE TxPmCode,int TxDataLen,BYTE TxData[],BYTE *RxReplyType,BYTE *RxCmCode,BYTE *RxPmCode,BYTE *RxStCode1,BYTE *RxStCode0,int *RxDataLen,BYTE RxData[]);
int APIENTRY CancelCommand(HANDLE ComHandle);
