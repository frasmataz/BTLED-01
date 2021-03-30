package com.frasersharp.btled;

import android.graphics.Color;

public class AnimationStep {
    public Color color;
    public int duration;

    public AnimationStep(Color color_, int duration_) {
        this.color = color_;
        this.duration = duration_;
    }
}
