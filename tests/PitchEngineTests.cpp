#include "../src/PitchEngine.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
using namespace pitchator;
static bool near(double a,double b,double tol=1e-9){return std::abs(a-b)<tol;}
int main(){
 Pattern p;
 assert(p.size()==1&&p.bars()==1&&near(p.value(960),0));
 assert(p.setBars(16)&&p.lengthTicks()==61440);
 assert(!p.setBars(17)&&!p.setBars(0));
 assert(p.upsert({gridTick(1,2,0),3,Curve::Step}));
 assert(p.upsert({gridTick(1,3,0),1,Curve::Linear}));
 assert(p.upsert({gridTick(1,4,0),0,Curve::Smooth}));
 assert(near(p.value(960),3)&&near(p.value(1919),3));
 assert(near(p.value(1920),1)&&near(p.value(2400),0.5));
 assert(near(p.value(2880),0));
 assert(!p.upsert({0,2,Curve::Step}));
 assert(!p.upsert({400,-49,Curve::Linear}));
 assert(!p.upsert({400,NAN,Curve::Linear}));
 assert(!p.upsert({61440,2,Curve::Step}));
 assert(!p.setBars(0));
 assert(!p.erase(0));
 assert(p.erase(960)&&near(p.value(960),0.5));
 assert(!p.erase(960));
 p.clear();assert(p.size()==1&&near(p.value(960),0));
 assert(p.setBars(1));
 assert(p.upsert({960,3,Curve::Linear}));
 assert(near(p.value(480),1.5));
 assert(p.upsert({960,4,Curve::Smooth})&&near(p.value(480),2.0));
 assert(p.upsert({1920,0,Curve::Step}));
 assert(near(p.value(1440),2.0)); // smooth midpoint
 assert(near(p.value(3840),0)); // 1-bar loop
 // Twelve tone classes of each scale checked against independent contracts.
 assert(isInScale(Scale::Major,0)&&isInScale(Scale::Major,11));
 assert(!isInScale(Scale::Major,1));
 assert(isInScale(Scale::NaturalMinor,3)&&!isInScale(Scale::NaturalMinor,4));
 assert(isInScale(Scale::Dorian,9)&&!isInScale(Scale::Dorian,8));
 assert(isInScale(Scale::Lydian,6)&&!isInScale(Scale::Lydian,5));
 assert(isInScale(Scale::Mixolydian,10)&&!isInScale(Scale::Mixolydian,11));
 assert(isInScale(Scale::Locrian,6)&&!isInScale(Scale::Locrian,7));
 assert(isInScale(Scale::HarmonicMinor,11)&&!isInScale(Scale::HarmonicMinor,10));
 // Exact half-tone +3 and -2 from current destination for a chromatic pattern.
 assert(near(value(p,960,60,0,Scale::Chromatic,true),4));
 assert(near(snapTarget(3,60,0,Scale::NaturalMinor),3));
 assert(near(snapTarget(1,60,0,Scale::NaturalMinor),2)); // C# -> D
 assert(near(snapTarget(-1,60,0,Scale::NaturalMinor),-2)); // B -> Bb
 assert(near(snapTarget(0,61,0,Scale::NaturalMinor),0)); // neutral is neutral
 assert(near(value(p,480,60,0,Scale::Major,false),2)); // unlocked glide
 // MIDI 14-bit encoding, exact central + both endpoints, symmetry contracts.
 assert(bend14(0,12)==8192);
 assert(bend14(-12,12)==0&&bend14(12,12)==16383);
 assert(bend14(3,12)==10240);
 assert(bend14(-3,12)==6144);
 assert(bend14(99,12)==16383&&bend14(-99,12)==0);
 assert(bend14(NAN,12)==8192);
 assert(near(hostTick(0,120,48000,24000,1),960));
 assert(near(hostTick(4,120,48000,0,1),0));
 assert(near(hostTick(-1,120,48000,0,1),2880));
 assert(gridTick(16,4,3)==(16*3840-240));
 assert(gridTick(0,1,0)==-1&&gridTick(1,5,0)==-1);
 std::cout<<"PitchEngine: all deterministic contracts PASS\n";
}
