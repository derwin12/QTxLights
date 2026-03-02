#pragma once
#include <QString>
#include <QVector>

// A single timing mark within one layer of a timing track.
// startMs and endMs are in milliseconds, matching the .xsq file's
// <Effect startTime="..." endTime="..."/> attributes.
struct TimingMark
{
    int     startMs = 0;
    int     endMs   = 0;
    QString label;
};

// One timing track from the sequence.
// xLights allows a track to have multiple EffectLayer children (e.g. Papagayo
// lip-sync tracks have one layer per phoneme set).  Each is stored separately
// so callers can choose how many layers to display.
struct TimingTrack
{
    QString                      name;
    QVector<QVector<TimingMark>> layers;  // one entry per <EffectLayer>
};

// A single effect placed on one layer of a model.
// effectType is the value of the "ref" attribute (e.g. "Twinkle", "Fire").
// Palette and settings strings are intentionally omitted for now.
struct ModelEffect
{
    int     startMs    = 0;
    int     endMs      = 0;
    QString effectType;  // from the "ref" attribute
};

// One model element from the sequence.
// Most models have a single EffectLayer; sub-model elements may have more.
struct ModelElement
{
    QString                         name;
    QVector<QVector<ModelEffect>>   layers;  // one entry per <EffectLayer>
};

// Everything we extract from parsing an .xsq file.
// This struct intentionally contains only the data we currently use.
struct XsqSequence
{
    // Metadata from <head>
    QString song;
    QString artist;
    QString mediaFile;
    int     frameDurationMs = 20;    // parsed from <sequenceTiming>, e.g. "20 ms"
    double  durationSeconds = 0.0;   // parsed from <sequenceDuration>

    QVector<TimingTrack>  timingTracks;
    QVector<ModelElement> modelElements;

    // Non-empty when parsing failed; describes the problem.
    QString parseError;

    bool isValid() const { return parseError.isEmpty(); }

    // Total number of frames derived from duration and frame rate.
    int totalFrames() const
    {
        if (frameDurationMs <= 0)
            return 0;
        return static_cast<int>(durationSeconds * 1000.0 / frameDurationMs);
    }
};

// Parse an .xsq file and return the result.
// On failure, returns an XsqSequence with parseError set and isValid() == false.
XsqSequence parseXsqFile(const QString& filePath);
