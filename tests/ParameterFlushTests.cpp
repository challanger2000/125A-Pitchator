#include "../src/PitchatorVST.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
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
 // Integration probe: GUI-shaped CurvePacket -> IMessage -> processor
 // mailbox -> real ProcessData MIDI output list, *not* just a curve-unit test.
 PitchatorProcessor routed;
 pitchator::Pattern modified;
 CHECK(modified.setBars(2));
 CHECK(modified.upsert({960,-3.0,pitchator::Curve::Step}));
 HostMessage message;
 message.setMessageID("125A.Pitchator.Curve");
 auto packed=pitchator::packCurve(modified);
 CHECK(message.getAttributes()->setBinary("Curve",&packed,
       static_cast<uint32>(sizeof(packed)))==kResultOk);
 CHECK(routed.notify(&message)==kResultOk);
 // Invalid GUI messages must not overwrite the last accepted snapshot.
 packed.magic=0;
 HostMessage invalid;
 invalid.setMessageID("125A.Pitchator.Curve");
 CHECK(invalid.getAttributes()->setBinary("Curve",&packed,
       static_cast<uint32>(sizeof(packed)))==kResultOk);
 CHECK(routed.notify(&invalid)==kResultFalse);
 EventList midiIn(32),midiOut(128);
 Event on{};
 on.busIndex=0;on.type=Event::kNoteOnEvent;on.sampleOffset=12;
 on.noteOn.channel=3;on.noteOn.pitch=60;on.noteOn.velocity=0.75f;
 on.noteOn.noteId=42;on.noteOn.length=0;
 CHECK(midiIn.addEvent(on)==kResultOk);
 Event off{};
 off.busIndex=0;off.type=Event::kNoteOffEvent;off.sampleOffset=128;
 off.noteOff.channel=3;off.noteOff.pitch=60;off.noteOff.velocity=0.25f;
 off.noteOff.noteId=42;
 CHECK(midiIn.addEvent(off)==kResultOk);
 ProcessContext clock{};
 clock.state=ProcessContext::kPlaying |
             ProcessContext::kProjectTimeMusicValid |
             ProcessContext::kTempoValid;
 clock.projectTimeMusic=1.0; // beat 2: curve target -3 st
 clock.tempo=120.0;clock.sampleRate=48000.0;
 ProcessData block{};
 block.numSamples=160;
 block.numInputs=0;block.numOutputs=0;
 block.inputEvents=&midiIn;block.outputEvents=&midiOut;
 block.processContext=&clock;
 CHECK(routed.process(block)==kResultOk);
 // Expect bend first (the NoteOn at 12 must already sound transposed),
 // then original NoteOn, NoteOff and precise channel pitch center.
 CHECK(midiOut.getEventCount()==4);
 Event bendBefore{},noteOut{},noteOffOut{},bendReset{};
 CHECK(midiOut.getEvent(0,bendBefore)==kResultOk);
 CHECK(midiOut.getEvent(1,noteOut)==kResultOk);
 CHECK(midiOut.getEvent(2,noteOffOut)==kResultOk);
 CHECK(midiOut.getEvent(3,bendReset)==kResultOk);
 CHECK(bendBefore.type==Event::kLegacyMIDICCOutEvent);
 CHECK(bendBefore.sampleOffset==12&&bendBefore.midiCCOut.channel==3);
 CHECK(bendBefore.midiCCOut.controlNumber==kPitchBend);
 CHECK((static_cast<int>(bendBefore.midiCCOut.value)&127)+
       ((static_cast<int>(bendBefore.midiCCOut.value2)&127)<<7)==
       pitchator::bend14(-3,12));
 CHECK(noteOut.type==Event::kNoteOnEvent&&noteOut.sampleOffset==12);
 CHECK(noteOut.noteOn.noteId==42&&noteOut.noteOn.pitch==60);
 CHECK(noteOut.noteOn.channel==3&&noteOut.noteOn.velocity==0.75f);
 CHECK(noteOffOut.type==Event::kNoteOffEvent&&noteOffOut.sampleOffset==128);
 CHECK(noteOffOut.noteOff.noteId==42&&noteOffOut.noteOff.pitch==60);
 CHECK(bendReset.type==Event::kLegacyMIDICCOutEvent);
 CHECK(bendReset.sampleOffset==128&&bendReset.midiCCOut.channel==3);
 CHECK((static_cast<int>(bendReset.midiCCOut.value)&127)+
       ((static_cast<int>(bendReset.midiCCOut.value2)&127)<<7)==8192);
 MemoryStream routedSaved;
 CHECK(routed.getState(&routedSaved)==kResultOk);
 CHECK(routedSaved.seek(0,IBStream::kIBSeekSet,nullptr)==kResultOk);
 PitchatorProcessor reopened;
 CHECK(reopened.setState(&routedSaved)==kResultOk);
 CHECK(controller.terminate()==kResultOk);
 std::puts("VST3 zero-buffer parameter flush, bypass and preset restore PASS");
 return 0;
}
