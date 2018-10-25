/*
comms.cpp
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

#include "comms.h"

/* class declaration, hardware serial bus and baudrate */
AircraftSocComms::AircraftSocComms(HardwareSerial& bus,uint32_t baud) {
  bus_ = &bus;
  baud_ =  baud;
}

/* begins communication over the hardware serial bus at the specified baudrate */
void AircraftSocComms::Begin() {
  Serial.print("Initializing communication with SOC...");
  bus_->begin(baud_);
  Serial.println("done!");
}

/* sends sensor data message */
void AircraftSocComms::SendSensorData(std::vector<uint8_t> &DataBuffer) {
  SendMessage(SensorData,DataBuffer);
}

/* returns mode command if a mode command message has been received */
bool AircraftSocComms::ReceiveModeCommand(AircraftMission::Mode *mode) {
  if (MessageReceived_) {
    if (ReceivedMessage_ == ModeCommand) {
      MessageReceived_ = false;
      if (ReceivedPayload_.size() == 1) {
        *mode = (AircraftMission::Mode)ReceivedPayload_[0];
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

/* returns configuration string if a configuration message has been received */
bool AircraftSocComms::ReceiveConfigMessage(std::vector<char> *ConfigString) {
  if (MessageReceived_) {
    if (ReceivedMessage_ == Configuration) {
      MessageReceived_ = false;
      ConfigString->resize(ReceivedPayload_.size());
      memcpy(ConfigString->data(),ReceivedPayload_.data(),ReceivedPayload_.size());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

/* returns effector command if a mode command message has been received */
bool AircraftSocComms::ReceiveEffectorCommand(std::vector<float> *EffectorCommands) {
  if (MessageReceived_) {
    if (ReceivedMessage_ == EffectorCommand) {
      MessageReceived_ = false;
      EffectorCommands->resize(ReceivedPayload_.size()/sizeof(float));
      memcpy(EffectorCommands->data(),ReceivedPayload_.data(),ReceivedPayload_.size());
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

/* checks for valid BFS messages received */
void AircraftSocComms::CheckMessages() {
  MessageReceived_ = ReceiveMessage(&ReceivedMessage_,&ReceivedPayload_);
}

/* builds and sends a BFS message given a message ID and payload */
void AircraftSocComms::SendMessage(Message message,std::vector<uint8_t> &Payload) {
  if (Payload.size() < (kUartBufferMaxSize-headerLength_-checksumLength_)) {
    // header
    Buffer_[0] = header_[0];
    Buffer_[1] = header_[1];
    // message ID
    Buffer_[2] = (uint8_t)message;
    // payload length
    Buffer_[3] = Payload.size() & 0xff;
    Buffer_[4] = Payload.size() >> 8;
    // payload
    memcpy(Buffer_+headerLength_,Payload.data(),Payload.size());
    // checksum
    CalcChecksum((size_t)(Payload.size()+headerLength_),Buffer_,Checksum_);
    Buffer_[Payload.size()+headerLength_] = Checksum_[0];
    Buffer_[Payload.size()+headerLength_+1] = Checksum_[1];
    // transmit
    bus_->write(Buffer_,Payload.size()+headerLength_+checksumLength_);
  }
}

/* parses BFS messages returning message ID and payload on success */
bool AircraftSocComms::ReceiveMessage(Message *message,std::vector<uint8_t> *Payload) {
  while(bus_->available()) {
    RxByte_ = bus_->read();
    // header
    if (ParserState_ < 2) {
      if (RxByte_ == header_[ParserState_]) {
        Buffer_[ParserState_] = RxByte_;
        ParserState_++;
      }
    } else if (ParserState_ == 3) {
      LengthBuffer_[0] = RxByte_;
      Buffer_[ParserState_] = RxByte_;
      ParserState_++;
    } else if (ParserState_ == 4) {
      LengthBuffer_[1] = RxByte_;
      Length_ = ((uint16_t)LengthBuffer_[1] << 8) | LengthBuffer_[0];
      if (Length_ > (kUartBufferMaxSize-headerLength_-checksumLength_)) {
        ParserState_ = 0;
        LengthBuffer_[0] = 0;
        LengthBuffer_[1] = 0;
        Length_ = 0;
        Checksum_[0] = 0;
        Checksum_[1] = 0;
        return false;
      }
      Buffer_[ParserState_] = RxByte_;
      ParserState_++;
    } else if (ParserState_ < (Length_ + headerLength_)) {
      Buffer_[ParserState_] = RxByte_;
      ParserState_++;
    } else if (ParserState_ == (Length_ + headerLength_)) {
      CalcChecksum(Length_ + headerLength_,Buffer_,Checksum_);
      if (RxByte_ == Checksum_[0]) {
        ParserState_++;
      } else {
        ParserState_ = 0;
        LengthBuffer_[0] = 0;
        LengthBuffer_[1] = 0;
        Length_ = 0;
        Checksum_[0] = 0;
        Checksum_[1] = 0;
        return false;
      }
    // checksum 1
    } else if (ParserState_ == (Length_ + headerLength_ + 1)) {
      if (RxByte_ == Checksum_[1]) {
        // message ID
        *message = (Message) Buffer_[2];
        // payload size
        Payload->resize(Length_);
        // payload
        memcpy(Payload->data(),Buffer_+headerLength_,Length_);
        ParserState_ = 0;
        LengthBuffer_[0] = 0;
        LengthBuffer_[1] = 0;
        Length_ = 0;
        Checksum_[0] = 0;
        Checksum_[1] = 0;
        return true;
      } else {
        ParserState_ = 0;
        LengthBuffer_[0] = 0;
        LengthBuffer_[1] = 0;
        Length_ = 0;
        Checksum_[0] = 0;
        Checksum_[1] = 0;
        return false;
      }
    }
  }
  return false;
}

/* computes a two byte checksum */
void AircraftSocComms::CalcChecksum(size_t ArraySize, uint8_t *ByteArray, uint8_t *Checksum) {
  Checksum[0] = 0;
  Checksum[1] = 0;
  for (size_t i = 0; i < ArraySize; i++) {
    Checksum[0] += ByteArray[i];
    Checksum[1] += Checksum[0];
  }
}