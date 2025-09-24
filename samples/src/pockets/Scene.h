/*
 * Copyright (c) 2014 David Wicks, sansumbrella.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include "Pockets.h"

#include "choreograph/Choreograph.h"

#include <imgui.h>
#include <string>

struct Color : public ImColor {
    using ImColor::ImColor;
};

struct vec2 {
    vec2(double x = 0.0f, double y = 0.0f) : x(x), y(y) {}

    vec2 &operator+=(const vec2 &o) {
        *this = *this + o;
        return *this;
    }
    vec2 operator+(const vec2 &o) const { return {x + o.x, y + o.y}; }
    vec2 operator-(const vec2 &o) const { return {x - o.x, y - o.y}; }
    vec2 operator*(double s) const { return {x * s, y * s}; }
    vec2 operator*(const vec2 &o) const { return {x * o.x, y * o.y}; }

    operator ImVec2() const { return {float(x), float(y)}; }

    double x, y;
};

struct vec3 {
    vec3(double x = 0.0f, double y = 0.0f, double z = 0.0f)
        : x(x), y(y), z(z) {}

    vec3 operator+(const vec3 &o) const { return {x + o.x, y + o.y, z + o.z}; }
    vec3 operator-(const vec3 &o) const { return {x - o.x, y - o.y, z - o.z}; }
    vec3 operator*(double s) const { return {x * s, y * s, z * s}; }

private:
    double x, y, z;
};

struct quat {
    quat(double x = 0.0f, double y = 0.0f, double z = 0.0f, double w = 0.0f)
        : x(x), y(y), z(z), w(w) {}

private:
    double x, y, z, w;
};

struct mat4 {
    double a[4][4];
};

namespace pockets {
/**
 A basic application layer encapsulating input and view controls

 Renderable
 Updatable, pausable (always calls its own update on app signal unless paused)
 Listens for UI events

 Eventually, would like to make a simple scene-graph-like setup of:
 Connectable -> can connect itself to window stuff
 Renderable -> can be drawn to screen
 Maybe Runnable -> can/expects to be updated at 60Hz

 Scene graph would be useful primarily for laying out a simple GUI.
 Simple scene graph is implemented as Nodes in scene2d.
 */
typedef std::shared_ptr<class Scene> SceneRef;
class Scene {
public:
    typedef std::function<void()> Callback;
    Scene();
    virtual ~Scene();
    /// Set up scene when OpenGL context is guaranteed to exist (in or after
    /// app::setup())
    virtual void setup() {}

    /// temporarily freeze updates
    void pause();

    /// continue receiving updates
    void resume();

    /// update content
    virtual void update(ch::Time dt) {}

    void baseDraw(ch::Time dt);

    /// render content
    virtual void draw() {}

    /// Returns a pointer to the Scene's offset for animation.
    ch::Output<vec2> *getOffsetOutput() { return &_offset; }

    void setOffset(const vec2 &offset) { _offset = offset; }

    ch::Output<ch::Time> *getAnimationSpeedOutput() {
        return &_animation_speed;
    }

    /// returns the bounds of the controller in points
    ImVec4 getBounds() const { return _bounds; }

    /// set the region of screen into which we should draw this view
    void setBounds(const ImVec4 &points) { _bounds = points; }

    void setSize(const vec2 &size) { _size = size; }
    vec2 getSize() const { return _size; }

    void show(bool useWindowBounds = true);
    /// Returns a reference to our timeline.
    choreograph::Timeline &timeline() { return _timeline; }

private:
    ch::Output<vec2> _offset = vec2(0);
    ch::Output<ch::Time> _animation_speed = 1;
    ImVec4 _bounds;

    choreograph::Timeline _timeline;
    bool _paused = false;
    vec2 _size;
};
} // namespace pockets
