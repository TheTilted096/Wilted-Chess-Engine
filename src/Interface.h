// Class Definition for Interface (UCI Handler)

#include "Engine.h"
#include "Performer.h"

class Interface{
    public:
        std::string command, param;
        bool minPrint;

        void loop(Engine&);
        void positionStartpos(Engine&);
        void positionFen(Engine&);
        void setoption(Engine&);

        //void go(Engine&);



};