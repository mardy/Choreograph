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

#include "Repetition.h"

using namespace choreograph;

void Repetition::setup() {
    float w = getSize().x;
    float left = w * 0.08f;
    float right = w - left;
    PhraseRef<vec2> leftToRight =
        makeRamp(vec2(left, 0.0f), vec2(right, 0.0f), 1.0f, EaseInOutQuad());

    //=====================================================
    // Looping Motions. Use for things that loop "forever."
    // Reset time when the Motion finishes.
    //=====================================================

    Output<vec2> loopTarget;
    timeline()
        .apply(&loopTarget, leftToRight)
        .finishFn([&m = *loopTarget.inputPtr()] {
            m.resetTime();
        }); // Motion should play forever.

    Output<vec2> pingPongTarget;
    timeline()
        .apply(&pingPongTarget, leftToRight)
        .finishFn([&m = *pingPongTarget.inputPtr()] {
            // reverse Motion direction on finish.
            m.setPlaybackSpeed(m.getPlaybackSpeed() * -1);
            // Start each cycle from "zero" to keep in sync with loopTarget
            // timing.
            m.resetTime();
        });

    Output<vec2> pingPongSlowerTarget;
    timeline()
        .apply(&pingPongSlowerTarget, leftToRight)
        .finishFn([&m = *pingPongSlowerTarget.inputPtr()] {
            // Reverse and slow Motion on finish.
            m.setPlaybackSpeed(m.getPlaybackSpeed() * -0.9f);
            // If we're unbearably slow, stop looping.
            if (std::abs(m.getPlaybackSpeed()) < 0.2f) {
                m.setFinishFn([] {});
            } else {
                m.resetTime();
            }
        });

    //==========================================================
    // Looping Motion Group. Use for separate motions that loop
    // with each other, but may have different durations.
    // Reset time when the MotionGroup finishes.
    //==========================================================

    Sequence<vec2> positionSequence(vec2(getSize()) * vec2(0.66, 1) +
                                    vec2(0, 66));
    Sequence<vec3> rotationSequence(vec3(M_PI / 2, 0, 0));

    rotationSequence.then<RampTo>(vec3(4 * M_PI, 2 * M_PI, 0), 1.0f,
                                  EaseOutQuint());
    positionSequence.then<RampTo>(vec2(getSize()) * vec2(0.66, 0.5), 0.5f,
                                  EaseOutAtan());

    auto group = std::make_shared<ch::Timeline>();
    group->setDefaultRemoveOnFinish(false);
    group->apply(&_position, positionSequence);
    group->apply(&_rotation, rotationSequence);
    // start grouped motions after a 0.5 second hold on their start values.
    group->setStartTime(0.5f);
    group->setFinishFn([group] {
        group->setPlaybackSpeed(group->getPlaybackSpeed() * -1);
        group->resetTime();
    });

    timeline().addShared(group);

    //=====================================================
    // Looping Phrases. Use for a finite number of Repetition.
    // Compose Phrases in Loop/PingPongPhrases.
    //=====================================================

    Output<vec2> loopPhraseTarget;
    timeline().apply(&loopPhraseTarget).then(makeRepeat(leftToRight, 7.5f));

    Output<vec2> pingPongPhraseTarget;
    timeline()
        .apply(&pingPongPhraseTarget)
        .then(makePingPong(leftToRight, 7.5f));

    // Keep track of our animating things.
    // Note:
    // We `move` our Outputs into our Points to preserve the Motion connection
    // to them. This invalidates the locally-scoped names for the Outputs, which
    // cannot be used after the move.
    mTargets.emplace_back(Point{std::move(loopTarget), Color(1.0f, 0.0f, 1.0f),
                                "Reset Time on Motion Finish"});
    mTargets.emplace_back(
        Point{std::move(pingPongTarget), Color(1.0f, 0.0f, 1.0f),
              "Reverse Speed and Reset Time on Motion Finish"});
    mTargets.emplace_back(
        Point{std::move(pingPongSlowerTarget), Color(0.0f, 1.0f, 1.0f),
              "Reverse and Reduce Speed and Reset Time on Motion Finish"});
    mTargets.emplace_back(
        Point{std::move(loopPhraseTarget), Color(1.0f, 1.0f, 0.0f),
              "LoopPhrase 7.5 times composed with makeRepeat"});
    mTargets.emplace_back(
        Point{std::move(pingPongPhraseTarget), Color(1.0f, 1.0f, 0.0f),
              "PingPongPhrase 7.5 times composed with makePingPong"});

    // place things at initial timelined values.
    timeline().jumpTo(0);
}

void Repetition::update(Time dt) { timeline().step(dt); }

void Repetition::draw() {
    ImDrawList *list = ImGui::GetWindowDrawList();
    vec2 translation;
    {
        const float y_step = 100.0f;
        translation += vec2(0, (getSize().y - y_step * mTargets.size()) / 2);

        for (auto &target : mTargets) {
            list->AddCircleFilled(vec2(target._position) + translation, 24.0f,
                                  Color(target._color));

            list->AddText(vec2(10, 0) + translation, Color(1.0f, 1.0f, 1.0f),
                          target._description.c_str());

            translation += (vec2(0, y_step));
        }
    }

#if 0
  {
    auto color =  Color::HSV( 0.575f, 1.0f, 1.0f );
    gl::translate( _position );
    gl::drawString( "Reverse and Reset Time on MotionGroup Finish.", vec2( 0, -50 ) );
    gl::multModelMatrix( glm::eulerAngleYXZ( _rotation().y, _rotation().x, _rotation().z ) );
    gl::drawSolidCircle( vec2( 0 ), 36.0f );
  }
#endif
}
