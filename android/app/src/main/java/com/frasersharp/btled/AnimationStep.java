package com.frasersharp.btled;

import android.graphics.Color;

public class AnimationStep {
    public Color color;
    public int duration;
    public int flickerAmplitude;
    public int flickerFrequency;

    public AnimationStep(Color color_, int duration_) {
        this.color = color_;
        this.duration = duration_;
        this.flickerAmplitude = 0;
        this.flickerFrequency = 0;
    }

    public AnimationStep(Color color_, int duration_, int flickerAmplitude_, int flickerFrequency_) {
        this.color = color_;
        this.duration = duration_;
        this.flickerAmplitude = flickerAmplitude_;
        this.flickerFrequency = flickerFrequency_;
    }
}
