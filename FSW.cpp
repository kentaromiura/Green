//
// Created by Carlesso, Cristian on 2018/10/20.
//

#include "FSW.h"
#include "UnixSocketClient.h"
#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <functional>
#include <iterator>
#include <algorithm>

FSW::FSW(std::string path, const std::function<void(void)> callback) {
    if (system("which watchman > /dev/null 2>&1") == 0) {
        useWatchman = true;
    } else {
        // either no POSIX or no watchman
        useWatchman = false;
    }

    if (!useWatchman) {
        std::cout << "Currently Green needs watchman installed. Exiting." << std::endl;
        exit(-1);
    }

    // TODO: IF Unix Socket not available ...
    auto usc = UnixSocketClient();
    if (usc.Send("[\"subscribe\", \"" + path + "\", \"green\", {\"fields\": [\"name\"]}]\n")) {
        std::string lastReceived = "";
        // TODO: use a "debug" compilation constant to enable logging

        // Skip first response (subscription clock)
        // std::cout << "First response: " << usc.Receive();

        do {
            lastReceived = "";
            lastReceived += usc.Receive();

            // std::cout << "Received: " << lastReceived << std::endl;

            if (lastReceived.size() != 0) {

                auto response = nlohmann::json::parse(
                        lastReceived
                );

                // std::cout << "Received files: " << response["files"] << std::endl;


                if (std::all_of(response["files"].cbegin(), response["files"].cend(),
                                [](std::string s) { return s.find(".git") == 0; })) {
                    // std::cout << "Only git folder changes\n";
                } else {
                    callback();
                }
            }
        } while (lastReceived.size() != 0);
    } else {
        std::cout << "Error sending message";
        exit(-1);
    }

}