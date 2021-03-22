//
// Created by Carlesso, Cristian on 2018/10/20.
//

#ifndef GREEN_FSW_H
#define GREEN_FSW_H

#include <string>
#include <functional>

class FSW {
private:
    bool useWatchman = false;

public:
    FSW(std::string path, const std::function<void(void)> callback);
};


#endif //GREEN_FSW_H
