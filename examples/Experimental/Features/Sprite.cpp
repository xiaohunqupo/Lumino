﻿
#include <LuminoExample.hpp>

class App_Sprite : public Application
{
public:

    void onInit() override
    {
        auto sprite = Sprite::With(_TT("picture1.jpg")).buildInto();
    }

    void onUpdate() override
    {

    }
};

LUMINO_APP(App_Sprite);



