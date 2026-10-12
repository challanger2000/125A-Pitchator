#pragma once
#include "PitchScheduler.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/vst/ivstevents.h"
#include <array>
namespace Steinberg::Vst {
enum PitchParam:ParamID {
 kPitchKey=100,kPitchScale=101,kPitchBars=102,kPitchRange=103,
 kPitchLock=104,kPitchTrigger=105,kPitchLoop=106,kPitchBypass=107
};
static const FUID kPitchProcessorUID(0x125A4501,0x50495443,0x4841544F,0x52010001);
static const FUID kPitchControllerUID(0x125A4502,0x50495443,0x4841544F,0x52010001);
constexpr uint32_t kPitchMagic=0x31544350u;
constexpr uint32_t kPitchStateVersion=1u;
class PitchatorProcessor final:public AudioEffect {
public:
 PitchatorProcessor();
 static FUnknown* createInstance(void*){return static_cast<IAudioProcessor*>(new PitchatorProcessor);}
 tresult PLUGIN_API initialize(FUnknown*) override;
 tresult PLUGIN_API setActive(TBool) override;
 tresult PLUGIN_API setProcessing(TBool) override;
 tresult PLUGIN_API process(ProcessData&) override;
 tresult PLUGIN_API getState(IBStream*) override;
 tresult PLUGIN_API setState(IBStream*) override;
private:
 struct Change {int32 offset=0;ParamID id=0;ParamValue value=0;};
 std::array<Change,1024> changes_{};
 std::array<Event,2048> inputs_{};
 pitchator::Pattern pattern_{};
 pitchator::Settings settings_{};
 pitchator::Scheduler scheduler_{};
 bool previouslyPlaying_=false;
 void changeParam(ParamID,ParamValue) noexcept;
};
class PitchatorController final:public EditControllerEx1 {
public:
 static FUnknown* createInstance(void*){return static_cast<IEditController*>(new PitchatorController);}
 tresult PLUGIN_API initialize(FUnknown*) override;
 tresult PLUGIN_API setComponentState(IBStream*) override;
};
}
