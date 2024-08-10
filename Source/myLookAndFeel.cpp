/*
  ==============================================================================

    myLookAndFeel.cpp
    Created: 22 Mar 2023 12:55:15pm
    Author:  noone

  ==============================================================================
*/

#include "myLookAndFeel.h"

myLookAndFeel::myLookAndFeel() 
{
    setColour(backgroundColourId, juce::Colours::black);
    setColour(outlineColourId, juce::Colours::whitesmoke);
    setColour(knobBGColourId, juce::Colour(16, 16, 16));
    setColour(buttonBGColourId, juce::Colour(16, 16, 16));

    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(4, 4, 4));

    //setDefaultSansSerifTypefaceName("Wingdings");

    blueGlow.isRadial = true;
    blueGlow.addColour(0, juce::Colours::blue);
    blueGlow.addColour(1, juce::Colours::blue.withAlpha(0.f));
}

void myLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, Slider::SliderStyle ss , Slider& slider)
{
    const int lineWidth = 5;
    const int thumbWidth = 25;
    int centreX = (float)x + (float)width * 0.5f;
    int centreY = (float)y + (float)height * 0.5f;
    int sliderY = sliderPos;
    if (ss == Slider::SliderStyle::LinearVertical)
    {
        g.setColour(Colours::darkgrey);
        g.fillRect(centreX - lineWidth / 2, y, lineWidth, height);
        g.fillRect(centreX - thumbWidth / 2, sliderY, thumbWidth, 5);
        g.setColour(Colours::white);
        g.drawRect(centreX - thumbWidth / 2, sliderY, thumbWidth, 5, 2);
    }
}


void myLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto p = slider.getProperties();
    auto what = p["bipolar"];
    if (p["bipolar"])
    {
        drawBipolarSlider(g, x, y, width, height, sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
    }
    else
    {
        drawRegularSlider(g, x, y, width, height, sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
    }
}

void myLookAndFeel::drawRegularSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(findColour(knobBGColourId));
    g.fillEllipse(rx, ry, rw, rw);

    // replace ugly white circle with arc thing
    g.setColour(findColour(outlineColourId));
    juce::Path arcLine;
    arcLine.addArc(rx, ry, rw, rw, rotaryStartAngle, angle, true);
    g.strokePath(arcLine, juce::PathStrokeType(2));

    //markers for start / end angle
    const float markerLineLength = 1.25;
    float sx = rw * sin(rotaryStartAngle) / 2.0;
    float sy = -rw * cos(rotaryStartAngle) / 2.0;
    juce::Line startLine(sx + centreX, sy + centreY, sx * markerLineLength + centreX, sy * markerLineLength + centreY);
    sx = rw * sin(rotaryEndAngle) / 2.0;
    sy = -rw * cos(rotaryEndAngle) / 2.0;
    juce::Line endLine(sx + centreX, sy + centreY, sx * markerLineLength + centreX, sy * markerLineLength + centreY);
    g.drawLine(startLine, 2);
    g.drawLine(endLine, 2);

    //pointer
    juce::Path p;
    auto pointerLength = radius * 0.53f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius + 5.0, pointerThickness, pointerLength);
    //auto glowp = p;
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colours::white);
    g.fillPath(p);
}

void myLookAndFeel::drawBipolarSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto centerAngle = rotaryStartAngle + 0.5 * (rotaryEndAngle - rotaryStartAngle);
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(findColour(knobBGColourId));
    g.fillEllipse(rx, ry, rw, rw);

    // replace ugly white circle with arc thing
    g.setColour(findColour(outlineColourId));
    juce::Path arcLine;
    arcLine.addArc(rx, ry, rw, rw, centerAngle, angle, true);
    g.strokePath(arcLine, juce::PathStrokeType(2));

    //markers for start / end angle
    const float markerLineLength = 1.25;
    float sx = rw * sin(rotaryStartAngle) / 2.0;
    float sy = -rw * cos(rotaryStartAngle) / 2.0;
    juce::Line startLine(sx + centreX, sy + centreY, sx * markerLineLength + centreX, sy * markerLineLength + centreY);
    sx = rw * sin(rotaryEndAngle) / 2.0;
    sy = -rw * cos(rotaryEndAngle) / 2.0;
    juce::Line endLine(sx + centreX, sy + centreY, sx * markerLineLength + centreX, sy * markerLineLength + centreY);
    g.drawLine(startLine, 2);
    g.drawLine(endLine, 2);

    //pointer
    juce::Path p;
    auto pointerLength = radius * 0.53f;
    auto pointerThickness = 2.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius + 5.0, pointerThickness, pointerLength);
    //auto glowp = p;
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colours::white);
    g.fillPath(p);
}
