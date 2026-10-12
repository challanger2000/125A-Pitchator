#include "../src/CurveMailbox.h"
#include <cstdio>
using namespace pitchator;
#define TEST(x) do{if(!(x)){std::fprintf(stderr,"CurveMailbox fail %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
 Pattern original;TEST(original.setBars(16));
 TEST(original.upsert({960,3,Curve::Smooth}));
 TEST(original.upsert({57600,1,Curve::Step}));
 auto packet=packCurve(original);
 Pattern read;TEST(unpackCurve(packet,read));
 TEST(read.bars()==16&&read.size()==3&&read.at(2).tick==57600);
 packet.points[1].curve=99;TEST(!unpackCurve(packet,read));
 packet=packCurve(original);packet.points[2].tick=960;
 TEST(!unpackCurve(packet,read));
 packet=packCurve(original);
 CurveMailbox channel(original);
 uint64_t revision=0;Pattern snapshot;
 TEST(channel.snapshot(snapshot,revision));
 TEST(revision==2&&snapshot.size()==3);
 channel.publish(read); // retains prior valid state
 TEST(channel.snapshot(snapshot,revision));
 TEST(revision==4&&snapshot.size()==3);
 Pattern clear;clear.setBars(2);channel.publish(clear);
 TEST(channel.snapshot(snapshot,revision));
 TEST(snapshot.size()==1&&snapshot.bars()==2);
 std::puts("CurveMailbox snapshot and packet validation PASS");return 0;
}
