#include "XsqFile.h"
#include <QFile>
#include <QXmlStreamReader>

// ---------------------------------------------------------------------------
// Forward declarations for internal helpers
// ---------------------------------------------------------------------------

static void                    parseHead(QXmlStreamReader& xml, XsqSequence& seq);
static QVector<TimingMark>     parseTimingEffectLayer(QXmlStreamReader& xml);
static TimingTrack             parseTimingElement(QXmlStreamReader& xml);
static QVector<ModelEffect>    parseModelEffectLayer(QXmlStreamReader& xml);
static ModelElement            parseModelElement(QXmlStreamReader& xml);
static void                    parseElementEffects(QXmlStreamReader& xml, XsqSequence& seq);

// ---------------------------------------------------------------------------
// parseHead
//
// Reads the <head> element and populates sequence metadata.
// Exits when the </head> end element is reached.
// ---------------------------------------------------------------------------
static void parseHead(QXmlStreamReader& xml, XsqSequence& seq)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == u"head")
            break;

        if (!xml.isStartElement())
            continue;

        const auto tag = xml.name();

        if (tag == u"song") {
            seq.song = xml.readElementText();
        }
        else if (tag == u"artist") {
            seq.artist = xml.readElementText();
        }
        else if (tag == u"mediaFile") {
            seq.mediaFile = xml.readElementText();
        }
        else if (tag == u"sequenceTiming") {
            // Format: "20 ms" — extract the leading integer.
            const QString text    = xml.readElementText().trimmed();
            const int     spaceAt = text.indexOf(u' ');
            const QString numPart = (spaceAt > 0) ? text.left(spaceAt) : text;
            bool ok = false;
            const int ms = numPart.toInt(&ok);
            if (ok && ms > 0)
                seq.frameDurationMs = ms;
        }
        else if (tag == u"sequenceDuration") {
            bool ok = false;
            const double secs = xml.readElementText().toDouble(&ok);
            if (ok)
                seq.durationSeconds = secs;
        }
        else {
            // Any other <head> child we don't currently use.
            xml.skipCurrentElement();
        }
    }
}

// ---------------------------------------------------------------------------
// parseTimingEffectLayer
//
// Reads one <EffectLayer> inside a timing element and returns the marks.
// Exits when the </EffectLayer> end element is reached.
// ---------------------------------------------------------------------------
static QVector<TimingMark> parseTimingEffectLayer(QXmlStreamReader& xml)
{
    QVector<TimingMark> marks;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == u"EffectLayer")
            break;

        if (!xml.isStartElement() || xml.name() != u"Effect")
            continue;

        const auto attrs = xml.attributes();
        bool startOk = false;
        bool endOk   = false;

        TimingMark mark;
        mark.startMs = attrs.value(u"startTime").toInt(&startOk);
        mark.endMs   = attrs.value(u"endTime").toInt(&endOk);
        mark.label   = attrs.value(u"label").toString();

        // Only add the mark if both time values parsed successfully.
        if (startOk && endOk)
            marks.append(mark);

        // <Effect> is usually self-closing; skip any unexpected child content.
        xml.skipCurrentElement();
    }

    return marks;
}

// ---------------------------------------------------------------------------
// parseTimingElement
//
// Reads an <Element type="timing"> node and returns a populated TimingTrack.
// Each <EffectLayer> child becomes one entry in TimingTrack::layers.
// Exits when the </Element> end element is reached.
// ---------------------------------------------------------------------------
static TimingTrack parseTimingElement(QXmlStreamReader& xml)
{
    TimingTrack track;
    track.name = xml.attributes().value(u"name").toString();

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == u"Element")
            break;

        if (!xml.isStartElement())
            continue;

        if (xml.name() == u"EffectLayer") {
            track.layers.append(parseTimingEffectLayer(xml));
        }
        else {
            xml.skipCurrentElement();
        }
    }

    return track;
}

// ---------------------------------------------------------------------------
// parseModelEffectLayer
//
// Reads one <EffectLayer> inside a model element and returns the effects.
// Each <Effect> uses the "ref" attribute for the effect-type name.
// Exits when the </EffectLayer> end element is reached.
// ---------------------------------------------------------------------------
static QVector<ModelEffect> parseModelEffectLayer(QXmlStreamReader& xml)
{
    QVector<ModelEffect> effects;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == u"EffectLayer")
            break;

        if (!xml.isStartElement() || xml.name() != u"Effect")
            continue;

        const auto attrs = xml.attributes();
        bool startOk = false;
        bool endOk   = false;

        ModelEffect effect;
        effect.startMs    = attrs.value(u"startTime").toInt(&startOk);
        effect.endMs      = attrs.value(u"endTime").toInt(&endOk);
        effect.effectType = attrs.value(u"ref").toString();

        if (startOk && endOk)
            effects.append(effect);

        xml.skipCurrentElement();
    }

    return effects;
}

// ---------------------------------------------------------------------------
// parseModelElement
//
// Reads an <Element type="model"> node and returns a populated ModelElement.
// Each <EffectLayer> child becomes one entry in ModelElement::layers.
// Exits when the </Element> end element is reached.
// ---------------------------------------------------------------------------
static ModelElement parseModelElement(QXmlStreamReader& xml)
{
    ModelElement element;
    element.name = xml.attributes().value(u"name").toString();

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == u"Element")
            break;

        if (!xml.isStartElement())
            continue;

        if (xml.name() == u"EffectLayer") {
            element.layers.append(parseModelEffectLayer(xml));
        }
        else {
            xml.skipCurrentElement();
        }
    }

    return element;
}

// ---------------------------------------------------------------------------
// parseElementEffects
//
// Reads the <ElementEffects> block, collecting timing and model elements.
// Any other element types (submodel, group, etc.) are skipped.
// Exits when the </ElementEffects> end element is reached.
// ---------------------------------------------------------------------------
static void parseElementEffects(QXmlStreamReader& xml, XsqSequence& seq)
{
    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == u"ElementEffects")
            break;

        if (!xml.isStartElement() || xml.name() != u"Element")
            continue;

        const auto type = xml.attributes().value(u"type");

        if (type == u"timing") {
            seq.timingTracks.append(parseTimingElement(xml));
        }
        else if (type == u"model") {
            seq.modelElements.append(parseModelElement(xml));
        }
        else {
            xml.skipCurrentElement();
        }
    }
}

// ---------------------------------------------------------------------------
// parseXsqFile (public)
// ---------------------------------------------------------------------------
XsqSequence parseXsqFile(const QString& filePath)
{
    XsqSequence seq;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        seq.parseError = QStringLiteral("Cannot open file: ") + file.errorString();
        return seq;
    }

    QXmlStreamReader xml(&file);

    // The first real token must be the <xsequence> root element.
    if (!xml.readNextStartElement() || xml.name() != u"xsequence") {
        seq.parseError = QStringLiteral(
            "Not a valid xLights sequence file (expected <xsequence> root element).");
        return seq;
    }

    // Walk the top-level children of <xsequence>.
    while (!xml.atEnd()) {
        xml.readNext();

        if (!xml.isStartElement())
            continue;

        const auto tag = xml.name();

        if (tag == u"head") {
            parseHead(xml, seq);
        }
        else if (tag == u"ElementEffects") {
            parseElementEffects(xml, seq);
        }
        else {
            // DisplayElements, ColorPalettes, DataLayers, etc. — skip for now.
            xml.skipCurrentElement();
        }
    }

    if (xml.hasError()) {
        seq.parseError = QStringLiteral("XML error at line %1: %2")
                             .arg(xml.lineNumber())
                             .arg(xml.errorString());
    }

    return seq;
}
