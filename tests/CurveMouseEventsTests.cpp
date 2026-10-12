#include "../src/PitchCurveView.h"
#include "vstgui/lib/vstguiinit.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include "vstgui/lib/events.h"
#include <cstdio>
#include <cmath>
using namespace pitchator;
using namespace VSTGUI;

#define CHECK(condition) do { if(!(condition)){ \
 std::fprintf(stderr,"CurveMouseEvents line %d: %s\n",__LINE__,#condition); return 1;\
} } while(0)

static int runTests(){

 Pattern curve;CHECK(curve.setBars(2));
 CHECK(curve.upsert({960,3,Curve::Step}));
 CHECK(curve.upsert({1920,1,Curve::Linear}));
 CHECK(curve.upsert({2880,0,Curve::Linear}));
 int published=0,selectionChanged=0;
 // Real Pitchator UI placement, copied from pitchator.uidesc.
 // MouseDownEvent coordinates are PARENT-LOCAL, as passed by VSTGUI
 // CViewContainer::onMouseDownEvent (not translated again by child view).
 PitchCurveView view(CRect(30,248,1170,515),curve,
   [&]{++published;return true;},[&]{++selectionChanged;});
 CHECK(view.getMouseEnabled());
 auto outside=MouseDownEvent(CPoint(850,120),MouseButton::Left);
 view.onMouseDownEvent(outside);
 CHECK(!outside.consumed&&curve.size()==4);
 // Inside graph at y=330. Before the bugfix, pointerY returned 330+248,
 // which is OUTSIDE the canvas (top=276, bottom=477).
 auto down=MouseDownEvent(CPoint(850,330),MouseButton::Left);
 view.onMouseDownEvent(down);
 CHECK(down.consumed);
 CHECK(curve.size()==5);
 CHECK(view.selectedTick()>3840&&view.selectedTick()<7680);
 CHECK(view.selectedPoint()!=nullptr);
 CHECK(view.selectedPoint()->semitones>0);
 auto move=MouseMoveEvent(CPoint(950,430),MouseButton::Left);
 view.onMouseMoveEvent(move);
 CHECK(move.consumed);
 CHECK(view.selectedTick()>3840);
 CHECK(view.selectedPoint()->semitones<0);
 auto up=MouseUpEvent(CPoint(950,430),MouseButton::Left);
 view.onMouseUpEvent(up);
 CHECK(up.consumed);
 CHECK(published==1&&selectionChanged>=2);
 CHECK(curve.size()==5);
 // Selecting a drawn point must not accidentally add another.
 auto reselect=MouseDownEvent(CPoint(950,430),MouseButton::Left);
 view.onMouseDownEvent(reselect);
 CHECK(reselect.consumed&&curve.size()==5);
 auto reup=MouseUpEvent(CPoint(950,430),MouseButton::Left);
 view.onMouseUpEvent(reup);
 CHECK(reup.consumed);
 // Right-click removes the clicked point, not some offset neighbor.
 auto del=MouseDownEvent(CPoint(965,430),MouseButton::Right);
 view.onMouseDownEvent(del);
 CHECK(del.consumed&&curve.size()==4);
 CHECK(published==2);
 // Some hosts route right mouse UP separately. The fallback must delete
 // an intended point but must NOT delete any unrelated neighboring point.
 auto newDown=MouseDownEvent(CPoint(770,400),MouseButton::Left);
 view.onMouseDownEvent(newDown);
 auto newUp=MouseUpEvent(CPoint(770,400),MouseButton::Left);
 view.onMouseUpEvent(newUp);
 CHECK(curve.size()==5);
 auto deleteUp=MouseUpEvent(CPoint(785,400),MouseButton::Right);
 view.onMouseUpEvent(deleteUp);
 CHECK(deleteUp.consumed&&curve.size()==4);
 CHECK(published==4);
 // Point zero is immutable even when clicked or dragged.
 auto neutral=MouseDownEvent(CPoint(85,376.5),MouseButton::Left);
 view.onMouseDownEvent(neutral);
 CHECK(neutral.consumed&&view.selectedTick()==0);
 auto neutralDrag=MouseMoveEvent(CPoint(400,300),MouseButton::Left);
 view.onMouseMoveEvent(neutralDrag);
 CHECK(neutralDrag.consumed&&curve.at(0).tick==0&&curve.at(0).semitones==0);
 auto neutralUp=MouseUpEvent(CPoint(400,300),MouseButton::Left);
 view.onMouseUpEvent(neutralUp);
 CHECK(neutralUp.consumed);
 // Event coordinates are in logical VSTGUI units independent of OS DPI
 // and VST3Editor zoom: exact same graph control operates at 100/150%.
 CHECK(curve.size()==4);
 // Right-down + right-up must not delete TWO nearby points.
 Pattern dense;CHECK(dense.setBars(2));
 CHECK(dense.upsert({120,0,Curve::Linear}));
 CHECK(dense.upsert({240,0,Curve::Linear}));
 PitchCurveView denseView(CRect(30,248,1170,515),dense,
   []{return true;},[]{});
 const double nearX=85.0+120.0*1064.0/7680.0;
 auto downDense=MouseDownEvent(CPoint(nearX,376.5),MouseButton::Right);
 denseView.onMouseDownEvent(downDense);
 CHECK(downDense.consumed&&dense.size()==2);
 auto upDense=MouseUpEvent(CPoint(nearX,376.5),MouseButton::Right);
 denseView.onMouseUpEvent(upDense);
 CHECK(upDense.consumed&&dense.size()==2);
 // A new right-up-only gesture still deletes its nearest point.
 auto releaseDense=MouseUpEvent(CPoint(nearX+17,376.5),MouseButton::Right);
 denseView.onMouseUpEvent(releaseDense);
 CHECK(releaseDense.consumed&&dense.size()==1);
 std::puts("VSTGUI mouse coordinates, insert, drag, select and delete PASS");
 return 0;
}
int main(){
#ifdef _WIN32
 VSTGUI::init(GetModuleHandleW(nullptr));
#else
 VSTGUI::init(nullptr);
#endif
 const int result=runTests();
 VSTGUI::exit();
 return result;
}
