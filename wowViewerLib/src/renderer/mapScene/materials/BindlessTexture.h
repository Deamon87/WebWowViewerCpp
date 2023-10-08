//
// Created by Deamon on 10/8/2023.
//

#ifndef AWEBWOWVIEWERCPP_BINDLESSTEXTURE_H
#define AWEBWOWVIEWERCPP_BINDLESSTEXTURE_H

#include <functional>
#include <cstdint>

//Bindless stuff
class BindlessTexture {
public:
    BindlessTexture(uint32_t index, const std::function<void()> &onDestroy): m_index(index){};
    ~BindlessTexture() {
        if (onDestroy) onDestroy();
    }
    uint32_t getIndex() const {return m_index;};
private:
    std::function<void()> onDestroy;
    uint32_t m_index;
};


#endif //AWEBWOWVIEWERCPP_BINDLESSTEXTURE_H
