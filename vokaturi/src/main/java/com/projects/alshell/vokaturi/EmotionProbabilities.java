package com.projects.alshell.vokaturi;

import java.math.BigDecimal;
import java.math.RoundingMode;

/**
 * Container class for high accuracy metrics of emotions analyzed
 */
public class EmotionProbabilities
{
    public double Neutrality;
    public double Happiness;
    public double Sadness;
    public double Anger;
    public double Fear;

    public EmotionProbabilities()
    {
        Neutrality = 0.0;
        Happiness = 0.0;
        Sadness = 0.0;
        Anger = 0.0;
        Fear = 0.0;
    }

    /**
     * To remove trailing digits from the metric values, that even reach to E.
     * @param scale new scale, to be greater than 1
     */
    public void scaledValues(int scale)
    {
        Neutrality  = scale(Neutrality, scale);
        Happiness  = scale(Happiness, scale);
        Sadness  = scale(Sadness, scale);
        Anger  = scale(Anger, scale);
        Fear  = scale(Fear, scale);
    }

    private double scale(double val, int scale)
    {
        if(val == 0)
            return val;
        return new BigDecimal(val).setScale(scale, RoundingMode.HALF_EVEN).doubleValue();
    }

    @Override
    public String toString()
    {
        return "Neutrality: " + Neutrality +
                ", Happiness: " + Happiness +
                ", Sadness: " + Sadness +
                ", Anger: " + Anger +
                ", Fear: " + Fear;
    }
}
