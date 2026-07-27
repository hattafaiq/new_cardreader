#ifndef COMMON_WS_H
#define COMMON_WS_H

namespace httplib{
enum class CMD_TYPE{
    UNDEFINED,
    OPEN_PINPAD,
    CLOSE_PINPAD,
    START_PININPUT,
    GETPINBLOCK,
    KEY_PRESSEDCOUNT,
    EKTP_OPEN,
    EKTP_CLOSE,
    EKTP_READ,
    EKTP_VERIFY,
    EKTP_INFO
};
}

#endif