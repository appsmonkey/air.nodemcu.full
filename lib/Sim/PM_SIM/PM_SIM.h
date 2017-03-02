#ifndef PM_SIM_H
#define PM_SIM_H

#include <CityOS.h>

class PM_SIM: public CityOS {
public:

    PM_SIM();
    PM_SIM(int set);

private:

    int pm;
    void loop();

    void setPM2_5Range();
    void setPM10Range();
    void setWorstRange();
};

#endif /* ifndef PM_SIM_H */
