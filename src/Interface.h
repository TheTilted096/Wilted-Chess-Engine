// Class Definition for Interface (UCI Handler)

#include "Engine.h"
#include "Performer.h"

class Interface{
    public:
        std::string command, param;
        bool minPrint = false;
        bool asSoftNodes = false;
        std::string evalfilestr = "wilted-net-1-0.bin";

        void loop(Engine&);
        void positionStartpos(Engine&);
        void positionFen(Engine&);
        void setoption(Engine&);

        //void go(Engine&);
};