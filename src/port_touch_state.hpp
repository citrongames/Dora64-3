#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace dora3::touch {
enum class Control { Stick, A, B, Z, Start, L, R, Menu, ZoomIn, ZoomOut, Count };
constexpr int count = static_cast<int>(Control::Count);
struct Point { float x = 0, y = 0; };
struct Rect {
    float x, y, w, h;
    bool contains(Point p) const { return p.x >= x && p.x <= x+w && p.y >= y && p.y <= y+h; }
};
struct Layout {
    std::array<Point, count> positions{{
        {0.13f,0.74f}, {0.91f,0.79f}, {0.80f,0.79f}, {0.90f,0.49f},
        {0.48f,0.11f}, {0.60f,0.11f}, {0.68f,0.11f}, {0.36f,0.11f},
        {0.95f,0.30f}, {0.95f,0.41f}
    }};
    float scale = 1.0f, opacity = 0.50f, dialogueOpacity = 0.18f, sensitivity = 1.0f;
};
struct GameInput { uint16_t buttons = 0; float x = 0, y = 0; bool stickOwned = false; };
struct CameraInput { float dx = 0, dy = 0, zoom = 0; };

// Event, game and draw threads use the same model through the adapter's mutex.
// Coordinates are normalized to the window, never the N64 render resolution.
class State {
public:
    Layout layout;
    bool visible = true, editing = false, modern = true, dialogue = false, blocked = false;
    float width = 1280, height = 720, density = 1;
    Rect editorToolbar{0,0,0,0};
    bool menuRequested = false;

    static uint16_t mask(Control c) {
        switch (c) {
        case Control::A: return 0x8000;
        case Control::B: return 0x4000;
        case Control::Z: return 0x2000;
        case Control::Start: return 0x1000;
        case Control::L: return 0x0020;
        case Control::R: return 0x0010;
        default: return 0;
        }
    }
    bool shown(Control c) const { return editing || modern || (c != Control::ZoomIn && c != Control::ZoomOut); }
    Point halfSize(Control c) const {
        float dp = 56;
        if (c == Control::Stick) dp = 132;
        if (c == Control::A) dp = 72;
        if (c == Control::B) dp = 64;
        // Fit one common baseline to the screen, preserving control proportions.
        // Apply the user's scale afterwards so the height cap cannot swallow it.
        const float fittedDensity = std::min(density, height * std::min(.21f / 132.0f, .115f / 72.0f));
        const float pixels = dp * fittedDensity * layout.scale;
        return {pixels * (c == Control::Start ? .85f : .5f) / width, pixels * .5f / height};
    }
    void constrain(Control c) {
        auto& p = layout.positions[static_cast<int>(c)];
        const auto r = halfSize(c);
        p.x = std::clamp(std::isfinite(p.x) ? p.x : .5f, r.x+.02f, 1-r.x-.02f);
        p.y = std::clamp(std::isfinite(p.y) ? p.y : .5f, r.y+.025f, 1-r.y-.025f);
    }
    void resize(float w, float h, float dp) {
        if (w <= 0 || h <= 0) return;
        if (w != width || h != height) clear();
        width=w; height=h; density=std::clamp(dp, .5f, 4.0f);
        for (int i=0;i<count;++i) constrain(static_cast<Control>(i));
    }
    bool hit(Control c, Point p, float expansion = 1) const {
        const auto center=layout.positions[static_cast<int>(c)], r=halfSize(c);
        float x=(p.x-center.x)/(r.x*expansion), y=(p.y-center.y)/(r.y*expansion);
        if (c==Control::Start || c==Control::Menu || c==Control::L || c==Control::R)
            return std::abs(x)<=1 && std::abs(y)<=1;
        return x*x+y*y<=1;
    }
    bool held(Control c) const {
        for (const auto& f:fingers) if(f.used && f.control==static_cast<int>(c) && f.inside) return true;
        return false;
    }
    void clear() { fingers={}; pending=0; camera={}; menuRequested=false; }
    void set_context(bool useModern, bool inDialogue, bool menuBlocked) {
        if (modern!=useModern || blocked!=menuBlocked) clear();
        if (dialogue!=inDialogue) {
            // Never deliver a queued camera drag when a scripted camera returns.
            camera={};
            for (auto& f:fingers) if(f.used && f.control==Camera) f.used=false;
        }
        modern=useModern; dialogue=inDialogue; blocked=menuBlocked;
    }
    void gamepad_activity() { if (!editing) { clear(); visible=false; } }
    void disconnected() { clear(); visible=true; }
    void begin_edit() { clear(); backup=layout; editing=true; visible=true; blocked=false; editorToolbar={0,.83f,1,.17f}; }
    void finish_edit(bool save) { clear(); if(!save) layout=backup; editing=false; editorToolbar={0,0,0,0}; }
    void down(int64_t device, int64_t id, Point p) {
        if (blocked || find(device,id)) return;
        if (!visible) { visible=true; return; } // Reveal without firing an unseen button.
        if (editing && editorToolbar.contains(p)) return;
        Finger* f=nullptr;
        for(auto& candidate:fingers) if(!candidate.used) { f=&candidate; break; }
        if(!f) return;
        int control=Ignored;
        for(int i=count-1;i>=0;--i) if(shown(static_cast<Control>(i)) && hit(static_cast<Control>(i),p)) { control=i; break; }
        if(control==Ignored && !editing && !dialogue && p.x>.40f && p.y>.20f && p.y<.73f) control=Camera;
        if(control==Ignored) return;
        // One owner per control. A second finger cannot steal the stick or drag.
        for(const auto& other:fingers) if(other.used && other.control==control) return;
        *f={true,device,id,control,p,p,{},true};
        if(editing) {
            const auto pos=layout.positions[control];
            f->offset={pos.x-p.x,pos.y-p.y};
        }
        else if(control==static_cast<int>(Control::Menu)) { menuRequested=true; }
        else if(control>=0) pending |= mask(static_cast<Control>(control));
    }
    void move(int64_t device, int64_t id, Point p) {
        auto* f=find(device,id); if(!f) return;
        if(editing) {
            layout.positions[f->control]={p.x+f->offset.x,p.y+f->offset.y};
            constrain(static_cast<Control>(f->control));
        }
        else if(f->control==Camera && !dialogue) {
            if(modern) {
                // Normalize to a 720-unit short edge; resolution-independent sensitivity.
                camera.dx+=(p.x-f->pos.x)*width/height*720*layout.sensitivity;
                camera.dy+=(p.y-f->pos.y)*720*layout.sensitivity;
            }
            else {
                const float dx=(p.x-f->origin.x)*width/height, dy=p.y-f->origin.y;
                if(std::max(std::abs(dx),std::abs(dy)) >= .045f) {
                    pending |= std::abs(dx)>std::abs(dy) ? (dx>0 ? 0x0001 : 0x0002) : (dy>0 ? 0x0004 : 0x0008);
                    f->origin=p;
                }
            }
        }
        else if(f->control>=0) f->inside=hit(static_cast<Control>(f->control),p,1.25f);
        f->pos=p;
    }
    void up(int64_t device, int64_t id) { if(auto* f=find(device,id)) f->used=false; }
    GameInput read(bool cameraActive) {
        GameInput result;
        if(blocked || editing || !visible) { pending=0; return result; }
        result.buttons=pending; pending=0;
        for(const auto& f:fingers) if(f.used && f.control>=0) {
            if(f.control==static_cast<int>(Control::Stick)) {
                result.stickOwned=true;
                const auto center=layout.positions[0], r=halfSize(Control::Stick);
                float x=(f.pos.x-center.x)/r.x, y=(center.y-f.pos.y)/r.y;
                const float length=std::hypot(x,y);
                if(length>.12f) {
                    const float amount=std::clamp((length-.12f)/.78f,0.0f,1.0f);
                    result.x=x/length*amount; result.y=y/length*amount;
                }
            }
            else if(f.inside) result.buttons |= mask(static_cast<Control>(f.control));
        }
        if(modern && cameraActive) result.buttons &= ~uint16_t(0x0010); // Same R policy as physical input.
        return result;
    }
    CameraInput take_camera() {
        auto result=camera; camera={};
        if(blocked || editing || !visible || dialogue) return {};
        result.zoom=(held(Control::ZoomOut)?1.0f:0.0f)-(held(Control::ZoomIn)?1.0f:0.0f);
        return result;
    }
    Point stick_offset() const {
        for(const auto& f:fingers) if(f.used && f.control==0) {
            auto p=layout.positions[0], r=halfSize(Control::Stick);
            float x=(f.pos.x-p.x)/r.x, y=(f.pos.y-p.y)/r.y, n=std::max(1.0f,std::hypot(x,y));
            return {x/n,y/n};
        }
        return {};
    }
private:
    static constexpr int Camera=-1, Ignored=-2;
    struct Finger {
        bool used=false; int64_t device=0,id=0; int control=Ignored;
        Point pos,origin,offset; bool inside=false;
    };
    std::array<Finger,16> fingers{};
    Layout backup;
    uint16_t pending=0;
    CameraInput camera;
    Finger* find(int64_t device,int64_t id) {
        for(auto& f:fingers) if(f.used && f.device==device && f.id==id) return &f;
        return nullptr;
    }
};
}
