#include "time_stamp.hh"

int main(int argc, char *argv[])
{
    
    Time* time= new Time();
    time->setTimeTobeNow();

    printf("%s", time->getNowTime().c_str());
    printf("%s", time->getTimeSimp().c_str());
    printf("%s", time->getTimeFull().c_str());

    return 0;
}
