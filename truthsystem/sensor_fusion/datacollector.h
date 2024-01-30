#pragma once
#include "sensordata.hpp"
#include <vector>
#include "../singleton.h"
#include "sensormatch.h"

class DataCollector : public Singleton<DataCollector>{

public:
    explicit DataCollector()=default;

public:
    SensorMatch *mSensorMatch{nullptr};
    void setSensorMatch(SensorMatch*);
    void setChannel(const uint32_t& channel){
        mChannel = channel;
    };
    void add(const StructRadarDataPtr data);
    void add(const PointView&);
    void add(const char*,const uint32_t&,const uint64_t&,const uint32_t &);
private:
    uint32_t mChannel{0};
};