#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
namespace pitchator {
constexpr int32_t kTicksPerQuarter=960;
constexpr int32_t kTicksPerBar=3840; // V1 contract: 4/4
constexpr int kMaxBars=16;
constexpr size_t kMaxPoints=512;
enum class Curve:uint8_t { Step, Linear, Smooth };
enum class Scale:uint8_t {
 Chromatic, Major, NaturalMinor, HarmonicMinor, MajorPentatonic, MinorPentatonic,
 Dorian, Phrygian, Lydian, Mixolydian, Locrian, Count
};
struct Point {int32_t tick=0; double semitones=0; Curve toNext=Curve::Linear;};
inline bool validCurve(Curve c) noexcept {return c==Curve::Step||c==Curve::Linear||c==Curve::Smooth;}
inline bool validScale(Scale s) noexcept {return static_cast<unsigned>(s)<static_cast<unsigned>(Scale::Count);}
inline int mod12(int x) noexcept {int r=x%12;return r<0?r+12:r;}
inline bool isInScale(Scale s,int offset) noexcept {
 // Masks are in pitch-class order C=bit0 through B=bit11.
 constexpr uint16_t masks[]={
 0x0FFF,0x0AB5,0x05AD,0x09AD,0x0295,0x04A9,
 0x06AD,0x05AB,0x0ACD,0x06B5,0x055B};
 return validScale(s) && (masks[static_cast<unsigned>(s)] & (1u<<mod12(offset)))!=0;
}
inline double interpolate(double a,double b,double fraction,Curve shape) noexcept {
 if(shape==Curve::Step) return a;
 double u=std::clamp(fraction,0.0,1.0);
 if(shape==Curve::Smooth) u=u*u*(3.0-2.0*u);
 return a+(b-a)*u;
}
class Pattern {
public:
 Pattern() noexcept {clear();}
 int bars() const noexcept {return bars_;}
 int32_t lengthTicks() const noexcept {return bars_*kTicksPerBar;}
 size_t size() const noexcept {return count_;}
 const Point& at(size_t index) const noexcept {return points_[index];}
 void clear() noexcept {count_=1;points_[0]={0,0.0,Curve::Linear};}
 bool setBars(int bars) noexcept {
  if(bars<1||bars>kMaxBars) return false;
  for(size_t i=0;i<count_;++i) if(points_[i].tick>=bars*kTicksPerBar) return false;
  bars_=bars;return true;
 }
 bool upsert(Point p) noexcept {
  if(p.tick<0||p.tick>=lengthTicks()||!std::isfinite(p.semitones)||
     p.semitones < -48||p.semitones>48||!validCurve(p.toNext)||
     (p.tick==0&&p.semitones!=0)) return false;
  size_t i=0; while(i<count_&&points_[i].tick<p.tick) ++i;
  if(i<count_&&points_[i].tick==p.tick) {points_[i]=p;return true;}
  if(count_>=kMaxPoints) return false;
  for(size_t j=count_;j>i;--j) points_[j]=points_[j-1];
  points_[i]=p;++count_;return true;
 }
 bool erase(int32_t tick) noexcept {
  if(tick==0) return false;
  size_t i=1;while(i<count_&&points_[i].tick!=tick) ++i;
  if(i==count_)return false;
  for(size_t j=i+1;j<count_;++j)points_[j-1]=points_[j];
  --count_;return true;
 }
 double value(double tick)const noexcept {
  if(!std::isfinite(tick))return 0;
  double t=std::fmod(tick,static_cast<double>(lengthTicks()));
  if(t<0)t+=lengthTicks();
  size_t i=0;while(i+1<count_&&points_[i+1].tick<=t)++i;
  if(i+1==count_)return points_[i].semitones;
  const auto& a=points_[i];const auto& b=points_[i+1];
  return interpolate(a.semitones,b.semitones,(t-a.tick)/(b.tick-a.tick),a.toNext);
 }
private:
 int bars_=1;size_t count_=1;std::array<Point,kMaxPoints> points_{};
};
inline double snapTarget(double offset,int inputMidiPitch,int key,Scale scale) noexcept {
 if(!std::isfinite(offset))return 0;
 if(offset==0||scale==Scale::Chromatic||!validScale(scale))return offset;
 double destination=inputMidiPitch+offset;
 int middle=static_cast<int>(std::round(destination)),best=middle;
 double distance=std::numeric_limits<double>::infinity();
 int direction=offset>0?1:-1;
 for(int n=middle-12;n<=middle+12;++n) {
  if(!isInScale(scale,n-mod12(key)))continue;
  double d=std::abs(n-destination);
  if(d<distance-1e-9||(std::abs(d-distance)<1e-9&&
       (n-middle)*direction>(best-middle)*direction)) {distance=d;best=n;}
 }
 return best-inputMidiPitch;
}
// Scale-lock snaps the knot targets, NOT every sample of a continuous glide.
inline double value(const Pattern& p,double tick,int inputPitch,int key,Scale scale,bool locked) noexcept {
 if(!locked||scale==Scale::Chromatic||!validScale(scale))return p.value(tick);
 if(!std::isfinite(tick))return 0;
 double t=std::fmod(tick,static_cast<double>(p.lengthTicks()));
 if(t<0)t+=p.lengthTicks();
 size_t i=0;while(i+1<p.size()&&p.at(i+1).tick<=t)++i;
 const auto& a=p.at(i);
 double start=snapTarget(a.semitones,inputPitch,key,scale);
 if(i+1==p.size())return start;
 const auto& b=p.at(i+1);
 double end=snapTarget(b.semitones,inputPitch,key,scale);
 return interpolate(start,end,(t-a.tick)/(b.tick-a.tick),a.toNext);
}
// Exact MIDI center 8192; negative full-scale 0; positive full-scale 16383.
inline uint16_t bend14(double semitones,int range) noexcept {
 if(!std::isfinite(semitones)||range<1||range>48)return 8192;
 double x=std::clamp(semitones/range,-1.0,1.0);
 return static_cast<uint16_t>(std::lround(8192.0+x*(x>=0?8191.0:8192.0)));
}
// The host provides the start PPQ, sample rate and tempo for each block.
inline double hostTick(double quarterNotes,double tempo,double sampleRate,int offset,int bars) noexcept {
 if(!std::isfinite(quarterNotes)||!std::isfinite(tempo)||tempo<=0||
    !std::isfinite(sampleRate)||sampleRate<=0||offset<0||bars<1||bars>16)return 0;
 double ticks=(quarterNotes+offset*tempo/(60.0*sampleRate))*kTicksPerQuarter;
 double duration=bars*kTicksPerBar;
 double t=std::fmod(ticks,duration);return t<0?t+duration:t;
}
inline int32_t gridTick(int bar,int beat,int sixteenth) noexcept {
 if(bar<1||bar>16||beat<1||beat>4||sixteenth<0||sixteenth>3)return -1;
 return (bar-1)*kTicksPerBar+(beat-1)*kTicksPerQuarter+sixteenth*240;
}
} // namespace pitchator
