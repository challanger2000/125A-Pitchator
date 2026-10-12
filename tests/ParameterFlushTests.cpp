#include "../src/PitchatorVST.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include <cstdio>
using namespace Steinberg;
using namespace Steinberg::Vst;

// Standalone EXE test harness only: the plugin DLL gets the real HINSTANCE
// from the SDK's Windows module entry. Never use this placeholder in the DLL.
void* moduleHandle=nullptr;

#define CHECK(v) do { if(!(v)){std::fprintf(stderr,"ParameterFlushTests line %d: %s\n",__LINE__,#v);return 1;}}while(0)
static bool flushBypass(PitchatorProcessor& processor,ParamValue value){
 ParameterChanges changes(1);
 int32 index=0;
 auto* queue=changes.addParameterData(kPitchBypass,index);
 if(!queue)return false;
 int32 point=0;
 if(queue->addPoint(0,value,point)!=kResultOk)return false;
 ProcessData data{};
 data.numSamples=0;
 data.numInputs=0;
 data.numOutputs=0;
 data.inputParameterChanges=&changes;
 data.outputEvents=nullptr;  // crucial: validator flushes with no event list
 return processor.process(data)==kResultOk;
}
int main(){
 PitchatorProcessor processor;
 PitchatorController controller;
 CHECK(controller.initialize(nullptr)==kResultOk);
 CHECK(flushBypass(processor,1));
 MemoryStream snapshot;
 CHECK(processor.getState(&snapshot)==kResultOk);
 CHECK(flushBypass(processor,0));
 MemoryStream zeroState;
 CHECK(processor.getState(&zeroState)==kResultOk);
 CHECK(zeroState.seek(0,IBStream::kIBSeekSet,nullptr)==kResultOk);
 CHECK(controller.setComponentState(&zeroState)==kResultOk);
 CHECK(controller.getParamNormalized(kPitchBypass)==0.0);
 CHECK(snapshot.seek(0,IBStream::kIBSeekSet,nullptr)==kResultOk);
 CHECK(controller.setComponentState(&snapshot)==kResultOk);
 CHECK(controller.getParamNormalized(kPitchBypass)==1.0);
 CHECK(snapshot.seek(0,IBStream::kIBSeekSet,nullptr)==kResultOk);
 PitchatorProcessor restored;
 CHECK(restored.setState(&snapshot)==kResultOk);
 MemoryStream roundtrip;
 CHECK(restored.getState(&roundtrip)==kResultOk);
 CHECK(roundtrip.seek(0,IBStream::kIBSeekSet,nullptr)==kResultOk);
 CHECK(controller.setComponentState(&roundtrip)==kResultOk);
 CHECK(controller.getParamNormalized(kPitchBypass)==1.0);
 CHECK(controller.terminate()==kResultOk);
 std::puts("VST3 zero-buffer parameter flush, bypass and preset restore PASS");
 return 0;
}
