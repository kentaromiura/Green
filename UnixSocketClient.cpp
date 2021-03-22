//
// Created by Carlesso, Cristian on 2018/10/20.
//
#include "UnixSocketClient.h"
#include <string>

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

#include <sys/un.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define NO_FLAGS 0
const char NULLCHAR = '\0';

bool error(const char * message) {
    perror(message);
    return false;
}

void errorAndExit(const char * message) {
    perror(message);
    exit(-1);
}

bool UnixSocketClient::Send(std::string message) {
    // std::cout << "Sending: " << message << std::endl;

    const char* pointerToRemainingMessage = message.c_str();
    int remainingBytes = message.length();
    int sentBytes;

    while (remainingBytes) {
        sentBytes = send(serverId, pointerToRemainingMessage, remainingBytes, NO_FLAGS);
        if (sentBytes < 0) {
            if(errno == EINTR) continue; // re-try if interrupted...
            std::stringstream errorMessageSS;
            errorMessageSS << "Error while sending " << sentBytes << " / " << message.length() << " bytes. of: " << message;
            // purposely not chaining to avoid de-allocation of the string.
            auto errorMessage = errorMessageSS.str();
            const char* msg = errorMessage.c_str();
            return error(msg);
        } 
        
        if (sentBytes == 0) return false; // SOCKET is closed.
        
        remainingBytes -= sentBytes;
        pointerToRemainingMessage += sentBytes;
    }
    return true;
}

UnixSocketClient::UnixSocketClient() {
    const uint MAX_LINE_SIZE = 4096;
    auto outputFile = popen("watchman get-sockname", "r");
    memset(previousBuffer, NULLCHAR, previousBufferLength);

    char buf[MAX_LINE_SIZE];
    std::string jsonString;
    while (fgets(buf, MAX_LINE_SIZE, outputFile) != nullptr) {
        jsonString.append(buf);
    }

    auto response = nlohmann::json::parse(
            jsonString
    );

    sockname = response["sockname"];

    struct sockaddr_un watchmanAddress;

    // setup watchman unix address structure
    memset(&watchmanAddress, NULLCHAR, sizeof(watchmanAddress));
    watchmanAddress.sun_family = AF_UNIX;
    strncpy(watchmanAddress.sun_path, sockname.c_str(), sizeof(watchmanAddress.sun_path) - 1);

    serverId = socket(PF_UNIX, SOCK_STREAM, PF_UNSPEC);

    if (!serverId) errorAndExit("Could not find watchman's socket.");        

    if (
        connect(
            serverId, 
            (const struct sockaddr *) &watchmanAddress,
            sizeof(watchmanAddress)
        ) < 0
    ) errorAndExit("Can' t connect to watchman.");

}

bool UnixSocketClient::hasPreviousBuffer() {
    if (previousBuffer[0] != NULLCHAR) return true;
    return false;
}

bool UnixSocketClient::previousBufferHasNewline() {
    return strchr(previousBuffer, '\n') != nullptr;
}

std::string UnixSocketClient::previousLineFromBuffer() {
    char * pNewline = strchr(previousBuffer, '\n'); 
    std::string response = "";
    int size = pNewline - previousBuffer + 1 ; 
    // this is not ideal, but ok.
    char * tmpBuf = (char *) malloc(size + 1); 
    strncpy(tmpBuf, previousBuffer, size);
    tmpBuf[size] = NULLCHAR;
    response.append(tmpBuf);
    free(tmpBuf);
    
    strncpy(previousBuffer, pNewline + 1, previousBufferLength - size);

    memset(previousBuffer + previousBufferLength -1 - size, NULLCHAR, size);
    return response;
}

int UnixSocketClient::flushPreviousBuffer(char * buffer) {
    char * pZero = strchr(previousBuffer, NULLCHAR);
    int size = pZero - previousBuffer + 1;

    strncpy(buffer, previousBuffer, size);
    memset(previousBuffer, NULLCHAR, pZero - previousBuffer);

    return size;
}

std::string UnixSocketClient::Receive() {
    const int responseBufferLength = previousBufferLength;
    char responseBuffer[responseBufferLength];

    std::string response = "";
    int flushed = 0;
    
    // read until we get a newline
    if (hasPreviousBuffer()) {
        if (previousBufferHasNewline()) {
            response.append(previousLineFromBuffer());
            return response;
        } else {
            flushed = flushPreviousBuffer(responseBuffer);    
        }
    }
    
    while (response.find("\n") == std::string::npos) {
        
        int receivedBytes = recv(serverId, responseBuffer + flushed, responseBufferLength - flushed, NO_FLAGS);
        if (receivedBytes < 0) {
            if (errno == EINTR) continue; // re-try on interruption
            return "";
        }
        if (receivedBytes == 0) return "";


        const char * pNewline = strchr(responseBuffer, '\n');
        // got a newline, split result in result + buffer.
        if (pNewline != nullptr) {
            int size = pNewline - responseBuffer + 1;

            char * tmpBuf = (char *) malloc(size + 1);
            strncpy(tmpBuf, responseBuffer, size);
            tmpBuf[size] = NULLCHAR;
            response.append(tmpBuf);
            free(tmpBuf);

            // if we have remaining bytes they go to a buffer
            int remaining = receivedBytes - size;
            if(remaining > 0) {
                strncpy(previousBuffer, pNewline + 1, remaining);
                memset(previousBuffer + remaining, NULLCHAR, previousBufferLength - remaining);
            }
        } else {
            response.append(responseBuffer, receivedBytes);
        }

        // clear response buffer.
        memset(responseBuffer, NULLCHAR, responseBufferLength);
    }

    return response;
}




