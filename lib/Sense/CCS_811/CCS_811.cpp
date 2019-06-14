#include <CCS_811.h>

CCS_811::CCS_811(int scl, int sda, int i2c)
{
    setup(scl, sda, i2c);
};

CCS_811::CCS_811(int scl, int sda)
{
    setup(scl, sda, 0x5A);
};

void CCS_811::setup(int scl = D3, int sda = D4, int i2c = 0x5A)
{
    _ccs  = new CCS811(i2c);
    _wire = new TwoWire();
    _wire->begin(sda, scl);
    _ccs->begin(*_wire);

    in.eco2 = 0;
    in.tvoc = 0;

    sense(config["AIR_ECO_2"]);
    sense(config["AIR_TVOC"]);

    addToInterval(this);
}

CCS_811::~CCS_811()
{
    delete _ccs;
}

void CCS_811::interval()
{
    _ccs->readAlgorithmResults();

    in.eco2 = _ccs->getCO2();
    in.tvoc = _ccs->getTVOC();
    // Check if any reads failed and exit early (will try again).
    if (isnan(in.eco2) || isnan(in.tvoc)) {
        if (debug.errors) Serial
                << "ERROR| Failed to read from CCS sensor (eco2 & tvoc set to 0)!" << endl;
        return;
    }

    setSense(config["AIR_ECO_2"], in.eco2);
    setSense(config["AIR_TVOC"], in.tvoc);

} // CCS_811::interval
