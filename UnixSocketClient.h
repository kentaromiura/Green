//
// Created by Carlesso, Cristian on 2018/10/20.
//

#ifndef GREEN_UNIXSOCKETCLIENT_H
#define GREEN_UNIXSOCKETCLIENT_H

#include <string>

class UnixSocketClient {
public:
    constexpr static const int previousBufferLength = 1024;

private:
    std::string sockname;
    int serverId = 0;
    char previousBuffer[previousBufferLength];

    bool hasPreviousBuffer();
    bool previousBufferHasNewline();
    std::string previousLineFromBuffer();
    int flushPreviousBuffer(char * responseBuffer);
public:
    UnixSocketClient();

    bool Send(std::string message);
    std::string Receive();
};


#endif //GREEN_UNIXSOCKETCLIENT_H
