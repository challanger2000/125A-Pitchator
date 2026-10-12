#pragma once
#include "PitchEngine.h"
#include <array>
#include <cmath>
#include <cstdint>
namespace pitchator {
enum class TriggerMode : int {Host,Note};
struct Settings {
 int key=0; Scale scale=Scale::Chromatic; int range=12;
 bool lock=false; TriggerMode trigger=TriggerMode::Host;
 bool loop=true,bypass=false;
};
struct Clock {
 bool valid=false,playing=false;
 double ppq=0,tempo=120,rate=48000;
};
struct Bend {int offset=0,channel=0;uint16_t value=8192;};
class Scheduler {
public:
 void reset() noexcept {channels_={};absoluteSample_=0;}
 int64_t sample()const noexcept{return absoluteSample_;}
 void endBlock(int count) noexcept {if(count>0)absoluteSample_+=count;}
 void noteOn(int ch,int pitch,const Settings& cfg,int offset) noexcept {
  if(ch<0||ch>=16||pitch<0||pitch>=128)return;
  auto& c=channels_[ch];
  if(c.held[pitch]<255){++c.held[pitch];++c.total;}
  c.pitch=pitch;
  if(cfg.trigger==TriggerMode::Note)c.anchor=absoluteSample_+offset;
 }
 void noteOff(int ch,int pitch) noexcept {
  if(ch<0||ch>=16||pitch<0||pitch>=128)return;
  auto& c=channels_[ch];
  if(c.held[pitch]){--c.held[pitch];--c.total;}
  if(c.total==0){c.pitch=-1;return;}
  if(c.pitch==pitch&&c.held[pitch]==0){
   for(int i=127;i>=0;--i)if(c.held[i]){c.pitch=i;break;}
  }
 }
 template<class Sink> bool center(int offset,Sink sink) noexcept {
  for(int ch=0;ch<16;++ch){
   auto& c=channels_[ch];
   if(c.last!=8192){
    if(!sink(Bend{offset,ch,8192}))return false;
    c.last=8192;
   }
  }
  return true;
 }
 template<class Sink> bool render(int offset,const Clock& clock,const Pattern& pattern,
                                  const Settings& cfg,bool force,Sink sink) noexcept {
  for(int ch=0;ch<16;++ch){
   auto& c=channels_[ch];uint16_t next=8192;
   if(c.total&&c.pitch>=0&&!cfg.bypass&&std::isfinite(clock.tempo)&&
      clock.tempo>0&&std::isfinite(clock.rate)&&clock.rate>0){
    double ticks=0;bool running=false;
    if(cfg.trigger==TriggerMode::Host){
     if(clock.playing&&clock.valid&&std::isfinite(clock.ppq)){
      ticks=(clock.ppq+offset*clock.tempo/(60.0*clock.rate))*kTicksPerQuarter;
      running=true;
     }
    }else if(c.anchor>=0){
     ticks=(absoluteSample_+offset-c.anchor)*clock.tempo*kTicksPerQuarter/(60.0*clock.rate);
     running=ticks>=0;
    }
    if(running&&std::isfinite(ticks)&&(cfg.loop||ticks<pattern.lengthTicks())){
     next=bend14(value(pattern,ticks,c.pitch,cfg.key,cfg.scale,cfg.lock),cfg.range);
    }
   }
   if(next!=c.last&&(force||c.total==0||offset%64==0)){
    if(!sink(Bend{offset,ch,next}))return false;
    c.last=next;
   }
  }
  return true;
 }
private:
 struct Channel {
  std::array<uint8_t,128> held{};
  uint32_t total=0;
  int pitch=-1;
  int64_t anchor=-1;
  uint16_t last=8192;
 };
 std::array<Channel,16> channels_{};
 int64_t absoluteSample_=0;
};
}
