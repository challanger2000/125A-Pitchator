#include "../src/PitchScheduler.h"
#include <cstdio>
#include <vector>
using namespace pitchator;
#define CHECK(v) do{if(!(v)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#v);return 1;}}while(0)
int main(){
 Pattern p;CHECK(p.setBars(2));
 CHECK(p.upsert({960,3,Curve::Step}));
 CHECK(p.upsert({1920,1,Curve::Linear}));
 CHECK(p.upsert({2880,0,Curve::Linear}));
 Settings cfg;Clock h{true,true,0,120,48000};Scheduler s;
 std::vector<Bend> results;
 auto sink=[&](Bend b){results.push_back(b);return true;};
 s.noteOn(0,60,cfg,0);
 CHECK(s.render(0,h,p,cfg,true,sink)&&results.empty());
 CHECK(s.render(12000,h,p,cfg,true,sink));
 CHECK(results.back().value==bend14(1.5,12));
 CHECK(s.render(24000,h,p,cfg,true,sink));
 CHECK(results.back().value==bend14(3,12));
 CHECK(s.render(48000,h,p,cfg,true,sink));
 CHECK(results.back().value==bend14(1,12));
 cfg.bypass=true;CHECK(s.render(48000,h,p,cfg,true,sink));
 CHECK(results.back().value==8192);
 cfg.bypass=false;s.noteOff(0,60);CHECK(s.render(48000,h,p,cfg,true,sink));
 CHECK(results.back().value==8192);
 s.reset();cfg.trigger=TriggerMode::Note;results.clear();
 s.noteOn(2,61,cfg,0);
 CHECK(s.render(24000,h,p,cfg,true,sink));
 CHECK(results.back().channel==2&&results.back().value==bend14(3,12));
 cfg.loop=false;CHECK(s.render(200000,h,p,cfg,true,sink));
 CHECK(results.back().value==8192);
 s.reset();cfg.loop=true;cfg.trigger=TriggerMode::Host;
 s.noteOn(1,60,cfg,0);s.noteOn(1,64,cfg,0);
 results.clear();CHECK(s.render(24000,h,p,cfg,true,sink));
 CHECK(results.size()==1&&results.back().channel==1);
 s.noteOff(1,64);s.noteOff(1,60);
 CHECK(s.render(24000,h,p,cfg,true,sink));CHECK(results.back().value==8192);
 s.reset();s.noteOn(0,60,cfg,0);results.clear();
 CHECK(s.render(24000,h,p,cfg,true,sink));h.playing=false;
 CHECK(s.render(24000,h,p,cfg,true,sink));CHECK(results.back().value==8192);
 std::puts("PitchScheduler test PASS");
}
