// Time Management

#include "Types.h"

class Timeman{
    public:
        bool timeKept;
        std::chrono::steady_clock::time_point moment;

        int64_t hardTime, softTime; // hard and soft time limits (us)

        Timeman(){
            reset();
        }

        void reset(){
            timeKept = false;
            hardTime = ~0U;
            softTime = ~0U;
        }
        void start(){
            moment = std::chrono::steady_clock::now();
        }

        void setBounds(uint32_t base, uint32_t inc){ //inputs in ms, outputs in us
            timeKept = ~base;
            if (timeKept){
                softTime = 1000 * (base / 30 + inc / 2);
                hardTime = 1000 * (std::min(9 * base / 10, base / 10 + 9 * inc / 10));
            }
        }

        int64_t elapsed() const{
            return 1 + std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - moment).count();
        }

        bool exceedHard() const{ return elapsed() > hardTime; }
        bool exceedSoft() const{ return elapsed() > softTime; }
};