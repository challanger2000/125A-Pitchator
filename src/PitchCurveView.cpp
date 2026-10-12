#include "PitchCurveView.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/events.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace VSTGUI;
namespace pitchator {
namespace {
constexpr CColor kBackground(16,24,31,255);
constexpr CColor kPanel(22,33,41,255);
constexpr CColor kGrid(54,75,85,255);
constexpr CColor kBar(88,114,123,255);
constexpr CColor kLine(48,201,178,255);
constexpr CColor kText(211,225,226,255);
constexpr CColor kWarning(239,138,111,255);
}
PitchCurveView::PitchCurveView(const CRect& bounds,Pattern& pattern,std::function<bool()> publish,std::function<void()> selected)
:CView(bounds),pattern_(pattern),publish_(std::move(publish)),selectionChanged_(std::move(selected)) {setMouseEnabled(true);}
double PitchCurveView::left() const {return getViewSize().left+55;}
double PitchCurveView::top() const {return getViewSize().top+28;}
double PitchCurveView::width() const {return std::max(1.0,getWidth()-76.0);}
double PitchCurveView::height() const {return std::max(1.0,getHeight()-66.0);}
double PitchCurveView::tickToX(double t) const {
 return left()+(t-scrollBar_*kTicksPerBar)*width()/(viewBars_*kTicksPerBar);
}
double PitchCurveView::semitoneToY(double pitch) const {
 return top()+(pitchRange_-std::clamp(pitch,-static_cast<double>(pitchRange_),static_cast<double>(pitchRange_)))*height()/(2.0*pitchRange_);
}
int32_t PitchCurveView::xToTick(double x) const {
 double t=scrollBar_*kTicksPerBar+(x-left())*(viewBars_*kTicksPerBar)/width();
 t=std::clamp(t,0.0,static_cast<double>(pattern_.lengthTicks()-1));
 if(snap_) t=std::round(t/120.0)*120.0;
 return std::min(pattern_.lengthTicks()-1,static_cast<int32_t>(t));
}
double PitchCurveView::yToSemitone(double y) const {
 double p=pitchRange_-(y-top())*(2.0*pitchRange_)/height();
 // Vertical grid intentionally has integer semitone precision in V1.
 return std::round(std::clamp(p,-static_cast<double>(pitchRange_),static_cast<double>(pitchRange_)));
}
double PitchCurveView::pointerX(const CPoint& pos) const {
 CPoint p=pos;frameToLocal(p);
 return p.x+getViewSize().left;
}
double PitchCurveView::pointerY(const CPoint& pos) const {
 CPoint p=pos;frameToLocal(p);
 return p.y+getViewSize().top;
}
void PitchCurveView::draw(CDrawContext* dc){
 const auto rect=getViewSize();
 dc->setFillColor(kBackground);dc->drawRect(rect,kDrawFilled);
 dc->setFillColor(kPanel);
 dc->drawRect(CRect(left(),top(),left()+width(),top()+height()),kDrawFilled);
 dc->setLineWidth(1);
 const int pitchGridStep=pitchRange_<=12?1:pitchRange_<=24?2:4;
 for(int st=-pitchRange_;st<=pitchRange_;st+=pitchGridStep){
  const double y=semitoneToY(st);
  dc->setFrameColor(st==0?kBar:kGrid);
  dc->drawLine(CPoint(left(),y),CPoint(left()+width(),y));
  if(st==0||st%std::max(3,pitchRange_/4)==0){
   char label[32];std::snprintf(label,sizeof(label),"%+d",st);
   dc->setFontColor(kText);
   dc->drawString(label,CRect(rect.left+6,y-10,rect.left+48,y+10),kRightText);
  }
 }
 const int bars=pattern_.bars();
 const int last=std::min(bars,scrollBar_+viewBars_);
 for(int b=scrollBar_;b<=last;++b){
  double x=tickToX(b*kTicksPerBar);
  dc->setFrameColor(kBar);dc->drawLine(CPoint(x,top()),CPoint(x,top()+height()));
  if(b<bars){
   char n[24];std::snprintf(n,sizeof(n),"BAR %d",b+1);
   dc->setFontColor(kText);dc->drawString(n,CRect(x+4,rect.top+2,x+90,top()-2),kLeftText);
  }
  if(b<last)for(int beat=1;beat<4;++beat){
   x=tickToX(b*kTicksPerBar+beat*kTicksPerQuarter);
   dc->setFrameColor(kGrid);dc->drawLine(CPoint(x,top()),CPoint(x,top()+height()));
  }
 }
 dc->setFrameColor(kLine);dc->setLineWidth(2.2);
 // Fixed upper bound: 64 sampling segments per visible bar.
 const int samples=std::max(1,viewBars_*64);
 for(int i=1;i<=samples;++i){
  const double tick0=scrollBar_*kTicksPerBar+(i-1.0)*viewBars_*kTicksPerBar/samples;
  const double tick1=scrollBar_*kTicksPerBar+i*viewBars_*kTicksPerBar/samples;
  if(tick0>=pattern_.lengthTicks())break;
  dc->drawLine(CPoint(tickToX(tick0),semitoneToY(pattern_.value(tick0))),
               CPoint(tickToX(std::min(tick1,static_cast<double>(pattern_.lengthTicks()-1))),
                      semitoneToY(pattern_.value(std::min(tick1,static_cast<double>(pattern_.lengthTicks()-1))))));
 }
 for(size_t i=0;i<pattern_.size();++i){
  const auto& p=pattern_.at(i);
  if(p.tick<scrollBar_*kTicksPerBar||p.tick>(scrollBar_+viewBars_)*kTicksPerBar)continue;
  const double x=tickToX(p.tick),y=semitoneToY(p.semitones);
  dc->setFillColor(p.tick==selectedTick_?kText:kLine);
  dc->setFrameColor(kBackground);
  dc->drawEllipse(CRect(x-6,y-6,x+6,y+6),kDrawFilledAndStroked);
 }
 const Point* selected=&pattern_.at(0);
 for(size_t i=0;i<pattern_.size();++i)if(pattern_.at(i).tick==selectedTick_){selected=&pattern_.at(i);break;}
 char status[128];
 std::snprintf(status,sizeof(status),"POSITION %d.%d.%d  |  PITCH %+0.1f ST  |  %s  |  %s",
  selected->tick/kTicksPerBar+1,
  (selected->tick%kTicksPerBar)/kTicksPerQuarter+1,
  (selected->tick%kTicksPerQuarter)/120+1,
  selected->semitones,
  selected->toNext==Curve::Step?"STEP":selected->toNext==Curve::Smooth?"SMOOTH":"LINEAR",
  snap_?"SNAP ON":"SNAP OFF");
 dc->setFontColor(syncError_?kWarning:kText);
 dc->drawString(syncError_?"CURVE NOT SYNCED - CHECK PLUGIN CONNECTION":status,
                CRect(rect.left+10,rect.bottom-28,rect.right-10,rect.bottom-4),kLeftText);
}
void PitchCurveView::commit(){
 if(dirty_){syncError_=!publish_();dirty_=false;}
 invalid();
}
void PitchCurveView::onMouseDownEvent(MouseDownEvent& e){
 const double x=pointerX(e.mousePosition),y=pointerY(e.mousePosition);
 if(x<left()||x>left()+width()||y<top()-10||y>top()+height()+10)return;
 int32_t nearest=-1;double minDist=130.0;
 for(size_t i=0;i<pattern_.size();++i){
  const auto& p=pattern_.at(i);
  const double dx=x-tickToX(p.tick),dy=y-semitoneToY(p.semitones);
  const double d=dx*dx+dy*dy;if(d<minDist){minDist=d;nearest=p.tick;}
 }
 if(e.buttonState.isRight()){
  if(nearest>0&&pattern_.erase(nearest)){dirty_=true;selectedTick_=0;commit();if(selectionChanged_)selectionChanged_();}
  e.consumed=true;return;
 }
 if(!e.buttonState.isLeft())return;
 if(nearest>=0){selectedTick_=nearest;}
 else{
  auto tick=xToTick(x);
  auto value=yToSemitone(y);
  if(tick==0)value=0;
  if(pattern_.upsert({tick,value,selectedShape_})){selectedTick_=tick;dirty_=true;}
 }
 dragging_=true;if(selectionChanged_)selectionChanged_();invalid();e.consumed=true;
}
void PitchCurveView::onMouseMoveEvent(MouseMoveEvent& e){
 if(!dragging_)return;
 if(selectedTick_==0){e.consumed=true;return;}
 const double x=pointerX(e.mousePosition),y=pointerY(e.mousePosition);
 const int32_t tick=xToTick(std::clamp(x,left(),left()+width()));
 const double value=yToSemitone(std::clamp(y,top(),top()+height()));
 if(tick==0){e.consumed=true;return;}
 Curve style=selectedShape_;
 for(size_t i=0;i<pattern_.size();++i)if(pattern_.at(i).tick==selectedTick_){
  style=pattern_.at(i).toNext;break;
 }
 Pattern trial=pattern_;
 trial.erase(selectedTick_);
 if(trial.upsert({tick,value,style})){
  pattern_=trial;selectedTick_=tick;dirty_=true;invalid();
 }
 e.consumed=true;
}
void PitchCurveView::onMouseUpEvent(MouseUpEvent& e){
 if(!dragging_)return;
 dragging_=false;commit();if(selectionChanged_)selectionChanged_();e.consumed=true;
}
void PitchCurveView::onMouseCancelEvent(MouseCancelEvent& e){
 if(dragging_){dragging_=false;commit();e.consumed=true;}
}
void PitchCurveView::scrollBars(int delta){
 scrollBar_=std::clamp(scrollBar_+delta,0,std::max(0,pattern_.bars()-viewBars_));
 invalid();
}
void PitchCurveView::cycleVisibleBars(){
 viewBars_=viewBars_==1?2:viewBars_==2?4:viewBars_==4?8:viewBars_==8?16:1;
 scrollBar_=std::clamp(scrollBar_,0,std::max(0,pattern_.bars()-viewBars_));
 invalid();
}
void PitchCurveView::setCurve(Curve shape){
 selectedShape_=shape;
 for(size_t i=0;i<pattern_.size();++i)if(pattern_.at(i).tick==selectedTick_){
  Point p=pattern_.at(i);p.toNext=shape;
  if(pattern_.upsert(p)) {dirty_=true;commit();}
  break;
 }
 invalid();
}
bool PitchCurveView::eraseSelected(){
 if(pattern_.erase(selectedTick_)){selectedTick_=0;dirty_=true;commit();if(selectionChanged_)selectionChanged_();return true;}
 return false;
}
void PitchCurveView::clearCurve(){
 pattern_.clear();selectedTick_=0;dirty_=true;commit();
 if(selectionChanged_)selectionChanged_();
}
const Point* PitchCurveView::selectedPoint() const noexcept {
 for(size_t i=0;i<pattern_.size();++i)
  if(pattern_.at(i).tick==selectedTick_)return &pattern_.at(i);
 return nullptr;
}
bool PitchCurveView::editSelectedTick(int32_t tick){
 const auto* chosen=selectedPoint();
 if(!chosen||selectedTick_==0||tick<=0||tick>=pattern_.lengthTicks())return false;
 if(tick==selectedTick_)return true;
 for(size_t i=0;i<pattern_.size();++i)
  if(pattern_.at(i).tick==tick)return false; // refuse collision, no silent overwrite
 Pattern draft=pattern_;
 Point item=*chosen;
 if(!draft.erase(selectedTick_))return false;
 item.tick=tick;
 if(!draft.upsert(item))return false;
 pattern_=draft;selectedTick_=tick;dirty_=true;commit();
 if(selectionChanged_)selectionChanged_();
 return true;
}
bool PitchCurveView::editSelectedPitch(double semitones){
 const auto* chosen=selectedPoint();
 if(!chosen||!std::isfinite(semitones)||semitones< -48||semitones>48||
    (selectedTick_==0&&semitones!=0.0))return false;
 Point item=*chosen;item.semitones=semitones;
 if(!pattern_.upsert(item))return false;
 dirty_=true;commit();
 if(selectionChanged_)selectionChanged_();
 return true;
}
} // namespace pitchator
