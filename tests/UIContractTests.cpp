#include <array>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
static std::string load(const char* name){
 std::ifstream in(name,std::ios::binary);
 return std::string(std::istreambuf_iterator<char>(in),
                    std::istreambuf_iterator<char>());
}
int main(){
 const auto xml=load(PITCHATOR_UIDESC_PATH);
 const auto processor=load(PITCHATOR_SRC_PATH);
 const auto graph=load(PITCHATOR_GRAPH_PATH);
 if(xml.empty()||processor.empty()||graph.empty())return 1;
 for(int tag=100;tag<=107;++tag){
  const std::string needle="tag=\""+std::to_string(tag)+"\"";
  if(xml.find(needle)==std::string::npos){std::printf("missing param %d\n",tag);return 2;}
 }
 for(int tag=900;tag<=909;++tag){
  const std::string needle="control-tag=\""+
     std::string(tag==900?"UiZoom":tag==901?"Clear":tag==902?"Delete":
     tag==903?"Shape":tag==904?"Snap":tag==905?"Previous":
     tag==906?"Next":tag==907?"GraphZoom":tag==908?"PointPosition":"PointPitch")+"\"";
  if(xml.find(needle)==std::string::npos){std::printf("missing UI control %d\n",tag);return 3;}
 }
 const std::array<const char*,7> routes={"PitchatorController::valueChanged",
  "PitchatorController::verifyView","PitchatorController::createView",
  "PitchatorController::publishPattern","PitchatorProcessor::notify",
  "PitchatorProcessor::getState","PitchatorProcessor::setState"};
 for(const auto* route:routes)if(processor.find(route)==std::string::npos)return 4;
 const std::array<const char*,7> gestures={"onMouseDownEvent","onMouseMoveEvent",
  "onMouseUpEvent","onMouseCancelEvent","clearCurve","eraseSelected","setCurve"};
 for(const auto* gesture:gestures)if(graph.find(gesture)==std::string::npos)return 5;
 if(xml.find("pitchator-id=\"curve\"")==std::string::npos)return 6;
 if(processor.find("PitchatorController::syncPointEditors")==std::string::npos)return 8;
 if(graph.find("PitchCurveView::editSelectedTick")==std::string::npos)return 9;
 if(graph.find("PitchCurveView::editSelectedPitch")==std::string::npos)return 10;

 if(processor.find("publishedCurve_.publish(candidate)")==std::string::npos)return 7;
 std::puts("Pitchator static GUI contract paths present; host clicks NOT VERIFIED");
}
