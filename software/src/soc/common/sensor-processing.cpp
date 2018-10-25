/*
sensor-processing.cc
Brian R Taylor
brian.taylor@bolderflight.com

Copyright (c) 2018 Bolder Flight Systems
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "sensor-processing.h"

/* configures sensor processing given a JSON value and registers data with global defs */
void SensorProcessing::Configure(const rapidjson::Value& Config) {
  std::map<std::string,std::string> OutputKeysMap;
  // configuring baseline sensor processing
  if (Config.HasMember("Baseline")) {
    // path for the baseline functions /Sensor-Processing/Baseline
    std::string PathName = RootPath_+"/"+"Baseline";
    // iterate over each and check the "Type" key to make the correct function pointer
    const rapidjson::Value& BaselineConfig = Config["Baseline"];
    for (auto &Func : BaselineConfig.GetArray()) {
      if (Func.HasMember("Type")) {
        if (Func["Type"] == "Constant") {
          BaselineSensorProcessing_.push_back(std::make_shared<ConstantClass>());
        } else if (Func["Type"] == "Gain") {
          BaselineSensorProcessing_.push_back(std::make_shared<GainClass>());
        } else if (Func["Type"] == "Sum") {
          BaselineSensorProcessing_.push_back(std::make_shared<SumClass>());
        } else if (Func["Type"] == "IAS") {
          BaselineSensorProcessing_.push_back(std::make_shared<IndicatedAirspeed>());
        } else if (Func["Type"] == "AGL") {
          BaselineSensorProcessing_.push_back(std::make_shared<AglAltitude>());
        } else if (Func["Type"] == "PitotStatic") {
          BaselineSensorProcessing_.push_back(std::make_shared<PitotStatic>());
        } else if (Func["Type"] == "FiveHole") {
          BaselineSensorProcessing_.push_back(std::make_shared<FiveHole>());
        } else if (Func["Type"] == "EKF15StateINS") {
          BaselineSensorProcessing_.push_back(std::make_shared<Ekf15StateIns>());
        } else if (Func["Type"] == "Filter") {
          BaselineSensorProcessing_.push_back(std::make_shared<GeneralFilter>());
        } else if (Func["Type"] == "If") {
          BaselineSensorProcessing_.push_back(std::make_shared<If>());
        } else if (Func["Type"] == "MinCellVolt") {
          BaselineSensorProcessing_.push_back(std::make_shared<MinCellVolt>());
        } else {
          throw std::runtime_error(std::string("ERROR")+PathName+std::string(": Type specified is not a defined type"));
        }

        // configure the function
        BaselineSensorProcessing_.back()->Configure(Func,PathName);

      } else {
        throw std::runtime_error(std::string("ERROR")+PathName+std::string(": Type not specified in configuration."));
      }
    }
    // getting a list of all baseline keys and adding to superset of output keys
    // modify the key to remove the intermediate path
    // (i.e. /Sensor-Processing/Baseline/Ias --> /Sensor-Processing/Ias)
    deftree.GetKeys(PathName,&BaselineDataKeys_);
    for (auto Key : BaselineDataKeys_) {
      std::string MemberName = RootPath_+Key.substr(Key.rfind("/"));
      if (Key.substr(Key.rfind("/"))!="/Mode") {
        OutputKeysMap[MemberName] = MemberName;
      }
    }
  } else {
    throw std::runtime_error(std::string("ERROR")+RootPath_+std::string(": Baseline not specified in configuration."));
  }

  // configuring research sensor processing groups
  if (Config.HasMember("Research")) {
    const rapidjson::Value& ResearchConfig = Config["Research"];
    for (auto &Group : ResearchConfig.GetArray()) {
      if (Group.HasMember("Group-Name")&&Group.HasMember("Components")) {
        // vector of group names
        ResearchGroupKeys_.push_back(Group["Group-Name"].GetString());
        // path for the research functions /Sensor-Processing/"Group-Name"
        std::string PathName = RootPath_+"/"+Group["Group-Name"].GetString();
        for (auto &Func : Group["Components"].GetArray()) {
          if (Func.HasMember("Type")) {
            if (Func["Type"] == "Constant") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<ConstantClass>());
            } else if (Func["Type"] == "Gain") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<GainClass>());
            } else if (Func["Type"] == "Sum") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<SumClass>());
            } else if (Func["Type"] == "IAS") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<IndicatedAirspeed>());
            } else if (Func["Type"] == "AGL") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<AglAltitude>());
            } else if (Func["Type"] == "PitotStatic") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<PitotStatic>());
            } else if (Func["Type"] == "FiveHole") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<FiveHole>());
            } else if (Func["Type"] == "EKF15StateINS") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<Ekf15StateIns>());
            } else if (Func["Type"] == "Filter") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<GeneralFilter>());
            } else if (Func["Type"] == "If") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<If>());
            } else if (Func["Type"] == "MinCellVolt") {
              ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].push_back(std::make_shared<MinCellVolt>());
            } else {
              throw std::runtime_error(std::string("ERROR")+PathName+std::string(": Type specified is not a defined type"));
            }

            // configure the function
            ResearchSensorProcessingGroups_[ResearchGroupKeys_.back()].back()->Configure(Func,PathName);

          } else {
            throw std::runtime_error(std::string("ERROR")+PathName+std::string(": Type not specified in configuration."));
          }
        }
        // getting a list of all research keys and adding to superset of output keys
        // modify the key to remove the intermediate path
        // (i.e. /Sensor-Processing/GroupName/Ias --> /Sensor-Processing/Ias)
        deftree.GetKeys(PathName,&ResearchDataKeys_[ResearchGroupKeys_.back()]);
        for (auto Key : ResearchDataKeys_[ResearchGroupKeys_.back()]) {
          std::string MemberName = RootPath_+Key.substr(Key.rfind("/"));
          if (Key.substr(Key.rfind("/"))!="/Mode") {
            OutputKeysMap[MemberName] = MemberName;
          }
        }
      } else {
        throw std::runtime_error(std::string("ERROR")+RootPath_+std::string(": Group name or components not specified in configuration."));
      }
    }
  }
  /* map baseline and research outputs to superset of outputs */
  // iterate through output keys and check for matching keys in baseline or research
  for (auto OutputElem : OutputKeysMap) {
    // current output key
    std::string OutputKey = OutputElem.second;
    // iterate through baseline keys
    for (auto BaselineKey : BaselineDataKeys_) {
      // check for a match with output keys
      if (BaselineKey.substr(BaselineKey.rfind("/"))==OutputKey.substr(OutputKey.rfind("/"))) {
        std::string KeyName = BaselineKey.substr(BaselineKey.rfind("/"));
        // setup baseline data pointer
        Element *ele = deftree.getElement(BaselineKey);
        BaselineDataPtr_[KeyName] = ele;
        if (ele) {
            OutputData_[KeyName] = ele;
            deftree.makeAlias(BaselineKey, OutputKey);
        }
      }
    }
    // iterate through research keys
    for (auto GroupKey : ResearchGroupKeys_) {
      for (auto ResearchKey : ResearchDataKeys_[GroupKey]) {
        // check for a match with output keys
        if (ResearchKey.substr(ResearchKey.rfind("/"))==OutputKey.substr(OutputKey.rfind("/"))) {
          std::string KeyName = ResearchKey.substr(ResearchKey.rfind("/"));
          // setup research data pointer
          Element *ele = deftree.getElement(ResearchKey);
          ResearchDataPtr_[GroupKey][KeyName] = ele;
          if (ele) {
              OutputData_[KeyName] = ele;
              deftree.makeAlias(ResearchKey, OutputKey);
          }
        }
      }
    }
  }
  Configured_ = true;
}

/* returns whether sensor processing has been configured */
bool SensorProcessing::Configured() {
  return Configured_;
}

/* initializes sensor processing */
bool SensorProcessing::Initialized() {
  if (InitializedLatch_) {
    return true;
  } else {
    bool initialized = true;
    // initializing baseline sensor processing
    for (auto Func : BaselineSensorProcessing_) {
      Func->Initialize();
      if (!Func->Initialized()) {
        initialized = false;
      }
    }
    // initializing research sensor processing
    for (auto Group : ResearchGroupKeys_) {
      for (auto Func : ResearchSensorProcessingGroups_[Group]) {
        Func->Initialize();
        if (!Func->Initialized()) {
          initialized = false;
        }
      }
    }
    if (initialized) {
      InitializedLatch_ = true;
    }
    return initialized;
  }
}

/* sets the sensor processing group to output */
void SensorProcessing::SetEngagedSensorProcessing(std::string EngagedSensorProcessing) {
  EngagedGroup_ = EngagedSensorProcessing;
}

/* computes sensor processing data */
void SensorProcessing::Run() {
  if (EngagedGroup_ == "Baseline") {
    // running baseline sensor processing
    for (auto Func : BaselineSensorProcessing_) {
      Func->Run(GenericFunction::kEngage);
    }
    // running research sensor processing
    for (auto Group : ResearchGroupKeys_) {
      for (auto Func : ResearchSensorProcessingGroups_[Group]) {
        Func->Run(GenericFunction::kArm);
      }
    }
  } else {
    // running baseline sensor processing
    for (auto Func : BaselineSensorProcessing_) {
      Func->Run(GenericFunction::kArm);
    }
    // running research sensor processing
    for (auto Group : ResearchGroupKeys_) {
      for (auto Func : ResearchSensorProcessingGroups_[Group]) {
        if (Group == EngagedGroup_) {
          Func->Run(GenericFunction::kEngage);
        } else {
          Func->Run(GenericFunction::kArm);
        }
      }
    }
  }
}