#pragma once
#include "PitchEngine.h"
#include "vstgui/lib/cview.h"
#include <functional>

namespace pitchator {
class PitchCurveView final : public VSTGUI::CView {
public:
 PitchCurveView(const VSTGUI::CRect& bounds, Pattern& pattern,
                std::function<bool()> publish);
 void draw(VSTGUI::CDrawContext* ctx) override;
 void onMouseDownEvent(VSTGUI::MouseDownEvent& event) override;
 void onMouseMoveEvent(VSTGUI::MouseMoveEvent& event) override;
 void onMouseUpEvent(VSTGUI::MouseUpEvent& event) override;
 void onMouseCancelEvent(VSTGUI::MouseCancelEvent& event) override;
 void scrollBars(int delta);
 void cycleVisibleBars();
 void setSnap(bool enabled) {snap_=enabled;invalid();}
 bool snap() const {return snap_;}
 void setCurve(Curve shape);
 Curve curve() const {return selectedShape_;}
 bool eraseSelected();
 void clearCurve();
 void refresh() {invalid();}
private:
 Pattern& pattern_;
 std::function<bool()> publish_;
 bool snap_=true;
 bool dragging_=false;
 bool dirty_=false;
 bool syncError_=false;
 int viewBars_=2,scrollBar_=0;
 int32_t selectedTick_=0;
 Curve selectedShape_=Curve::Linear;
 double left() const;
 double top() const;
 double width() const;
 double height() const;
 int32_t xToTick(double x) const;
 double tickToX(double tick) const;
 double yToSemitone(double y) const;
 double semitoneToY(double value) const;
 double pointerX(const VSTGUI::CPoint& p) const;
 double pointerY(const VSTGUI::CPoint& p) const;
 void commit();
};
} // namespace pitchator
