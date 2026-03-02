#include "SequenceGrid.h"
#include "XsqFile.h"
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLinearGradient>

// ---------------------------------------------------------------------------
// Color palette used throughout the grid paint code.
// Keeping them together makes visual tweaks straightforward.
// ---------------------------------------------------------------------------
namespace {
    const QColor BG_EVEN        { 42,  42,  42 };
    const QColor BG_ODD         { 50,  50,  50 };
    const QColor HDR_BG         { 60,  60,  60 };
    const QColor HDR_TXT        {200, 200, 200 };
    const QColor GRID_LINE      { 70,  70,  70 };
    const QColor MODEL_ROW_HDR  { 45,  45,  50 };   // dark grey — model rows
    const QColor TIMING_ROW_HDR { 30,  50,  70 };   // dark blue  — timing rows
    const QColor PLAYHEAD       {255,  80,  80 };
}

// Round-robin palette assigned to timing tracks so each track has a distinct
// colour without requiring the .xsq file to carry colour information.
static const QColor TIMING_PALETTE[] = {
    {  0, 160, 255},
    {255, 130,   0},
    {  0, 200, 100},
    {200,  60, 200},
    {255, 200,   0},
    { 80, 200, 255},
    {255,  80, 120},
    {100, 220,  80},
};
static constexpr int TIMING_PALETTE_SIZE =
    static_cast<int>(std::size(TIMING_PALETTE));

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
SequenceGrid::SequenceGrid(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
}

// ---------------------------------------------------------------------------
// loadFromXsq
//
// Replaces all current content with the timing tracks from the parsed file.
// Each EffectLayer within a track becomes its own row so that multi-layer
// tracks (e.g. Papagayo) are fully visible.
// ---------------------------------------------------------------------------
void SequenceGrid::loadFromXsq(const XsqSequence& xsq)
{
    m_rows.clear();
    m_blocks.clear();
    m_selectedRow = -1;
    m_selectedCol = -1;
    m_totalFrames = xsq.totalFrames();

    int colorIdx = 0;
    for (const TimingTrack& track : xsq.timingTracks) {
        const int     layerCount = track.layers.size();
        const QColor& color      = TIMING_PALETTE[colorIdx % TIMING_PALETTE_SIZE];

        for (int layerIdx = 0; layerIdx < layerCount; ++layerIdx) {
            const int rowIndex = m_rows.size();

            // Second and subsequent layers get a bracketed suffix so the user
            // can tell them apart, e.g. "Phrases [2]".
            const QString rowName = (layerCount > 1)
                ? QStringLiteral("%1 [%2]").arg(track.name).arg(layerIdx + 1)
                : track.name;

            m_rows.append({RowType::TimingTrack, rowName});

            for (const TimingMark& mark : track.layers[layerIdx]) {
                const int startFrame = mark.startMs / xsq.frameDurationMs;
                const int endFrame   = mark.endMs   / xsq.frameDurationMs;
                m_blocks.append({rowIndex, startFrame, endFrame, color, mark.label});
            }
        }

        ++colorIdx;
    }

    updateScrollBars();
    viewport()->update();
}

// ---------------------------------------------------------------------------
// loadSampleData
//
// Fills the grid with hard-coded data for development and UI testing.
// ---------------------------------------------------------------------------
void SequenceGrid::loadSampleData()
{
    static const QStringList MODEL_NAMES = {
        "Mega Tree", "Matrix 1", "Arches 1", "Arches 2",
        "Arches 3",  "Arches 4", "Roofline", "Candy Canes",
        "Stars",     "Spinner",  "Icicles",  "Window Frames",
    };

    m_rows.clear();
    m_blocks.clear();
    m_selectedRow = -1;
    m_selectedCol = -1;
    m_totalFrames = 400;

    for (const QString& name : MODEL_NAMES)
        m_rows.append({RowType::Model, name});

    struct Sample { int row, s, e; QColor c; const char* label; };
    static const Sample DATA[] = {
        { 0,   0,  60, {  0,160,255}, "Twinkle"      },
        { 0,  80, 150, {255,100,  0}, "Bars"          },
        { 0, 200, 280, {  0,200,100}, "Wave"          },
        { 1,  10,  80, {200, 50,200}, "Spiral"        },
        { 1, 100, 180, {255,200,  0}, "Fire"          },
        { 2,   0,  40, {  0,200,200}, "On"            },
        { 2,  50, 120, {255, 80, 80}, "Wipe"          },
        { 2, 150, 240, {100,200,  0}, "Morph"         },
        { 3,  20, 100, {255,160, 20}, "Brightness"    },
        { 3, 120, 200, { 80,120,255}, "Color Wash"    },
        { 4,   0,  80, {200,200, 50}, "Curtain"       },
        { 4, 100, 180, {255, 50,150}, "Pinwheel"      },
        { 5,  30, 110, {  0,180,255}, "Marquee"       },
        { 5, 130, 230, {200, 80, 30}, "Snowflakes"    },
        { 6,   0,  60, {150,255,100}, "Butterfly"     },
        { 6,  80, 160, {255,200,100}, "Kaleidoscope"  },
        { 7,  20,  90, {100,100,255}, "Piano"         },
        { 7, 110, 200, {255, 80, 80}, "Shockwave"     },
        { 8,   0, 130, { 80,230,200}, "Galaxy"        },
        { 9,  40, 180, {230,100,230}, "Liquid"        },
        {10,   0,  50, {255,160, 40}, "Glitter"       },
        {10,  70, 170, { 60,200,255}, "Lightning"     },
        {11,  10, 100, {200,255, 80}, "Ripple"        },
        {11, 120, 300, {255,100,150}, "Fire"          },
    };

    for (const auto& d : DATA)
        m_blocks.append({d.row, d.s, d.e, d.c, d.label});

    updateScrollBars();
    viewport()->update();
}

// ---------------------------------------------------------------------------
// zoom
// ---------------------------------------------------------------------------
void SequenceGrid::zoom(double factor)
{
    m_cellWidth = qBound(4.0, m_cellWidth * factor, 80.0);
    updateScrollBars();
    viewport()->update();
}

// ---------------------------------------------------------------------------
// updateScrollBars
// ---------------------------------------------------------------------------
void SequenceGrid::updateScrollBars()
{
    const int gridW = static_cast<int>(m_totalFrames * m_cellWidth);
    const int gridH = m_rows.size() * ROW_H;
    const int vpW   = viewport()->width()  - HDR_W;
    const int vpH   = viewport()->height() - HDR_H;

    horizontalScrollBar()->setRange(0, qMax(0, gridW - vpW));
    horizontalScrollBar()->setPageStep(vpW);
    horizontalScrollBar()->setSingleStep(static_cast<int>(m_cellWidth));

    verticalScrollBar()->setRange(0, qMax(0, gridH - vpH));
    verticalScrollBar()->setPageStep(vpH);
    verticalScrollBar()->setSingleStep(ROW_H);
}

// ---------------------------------------------------------------------------
// paintEvent
// ---------------------------------------------------------------------------
void SequenceGrid::paintEvent(QPaintEvent*)
{
    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing, false);

    const int sx   = horizontalScrollBar()->value();
    const int sy   = verticalScrollBar()->value();
    const int vpW  = viewport()->width();
    const int vpH  = viewport()->height();
    const int rows = m_rows.size();
    const int cw   = static_cast<int>(m_cellWidth);

    // --- Background fill ---
    p.fillRect(viewport()->rect(), BG_EVEN);

    // --- Row stripes (alternating dark/slightly-lighter) ---
    for (int r = 0; r < rows; ++r) {
        const int y = HDR_H + r * ROW_H - sy;
        if (y + ROW_H < HDR_H || y > vpH)
            continue;
        p.fillRect(HDR_W, y, vpW - HDR_W, ROW_H, (r % 2 == 0) ? BG_EVEN : BG_ODD);
    }

    // --- Vertical grid lines ---
    {
        // Draw fewer lines when zoomed out to keep the grid readable.
        const int step = (cw >= 20) ? 1 : (cw >= 10) ? 5 : 10;
        p.setPen(QPen(GRID_LINE, 1));
        for (int f = 0; f <= m_totalFrames; f += step) {
            const int x = HDR_W + static_cast<int>(f * m_cellWidth) - sx;
            if (x < HDR_W || x > vpW)
                continue;
            p.drawLine(x, HDR_H, x, vpH);
        }
    }

    // --- Horizontal grid lines ---
    p.setPen(QPen(GRID_LINE, 1));
    for (int r = 0; r <= rows; ++r) {
        const int y = HDR_H + r * ROW_H - sy;
        if (y < HDR_H || y > vpH)
            continue;
        p.drawLine(HDR_W, y, vpW, y);
    }

    // --- Effect / timing blocks ---
    QFont blockFont = p.font();
    blockFont.setPointSize(8);
    p.setFont(blockFont);

    for (const EffectBlock& block : m_blocks) {
        const int y = HDR_H + block.row * ROW_H - sy + 1;
        if (y + ROW_H < HDR_H || y > vpH)
            continue;

        const int x1 = HDR_W + static_cast<int>(block.startFrame * m_cellWidth) - sx;
        const int x2 = HDR_W + static_cast<int>(block.endFrame   * m_cellWidth) - sx;
        if (x2 < HDR_W || x1 > vpW || x2 - x1 < 2)
            continue;

        const QRect blockRect(x1, y, x2 - x1, ROW_H - 2);

        QLinearGradient grad(blockRect.topLeft(), blockRect.bottomLeft());
        grad.setColorAt(0.0, block.color.lighter(140));
        grad.setColorAt(1.0, block.color.darker(130));
        p.fillRect(blockRect, grad);

        p.setPen(QPen(block.color.darker(160), 1));
        p.drawRect(blockRect);

        if (blockRect.width() > 30) {
            p.setPen(Qt::white);
            p.drawText(blockRect.adjusted(3, 0, -2, 0),
                       Qt::AlignVCenter | Qt::AlignLeft,
                       block.label);
        }
    }

    // --- Playhead (fixed at frame 50 until playback is implemented) ---
    {
        const int x = HDR_W + static_cast<int>(50 * m_cellWidth) - sx;
        if (x >= HDR_W && x <= vpW) {
            p.setPen(QPen(PLAYHEAD, 2));
            p.drawLine(x, HDR_H, x, vpH);

            QPolygon arrow;
            arrow << QPoint(x - 6, HDR_H)
                  << QPoint(x + 6, HDR_H)
                  << QPoint(x,     HDR_H + 10);
            p.setBrush(PLAYHEAD);
            p.setPen(Qt::NoPen);
            p.drawPolygon(arrow);
        }
    }

    // --- Column (time) header — drawn on top so it is never obscured ---
    p.fillRect(HDR_W, 0, vpW - HDR_W, HDR_H, HDR_BG);
    p.setPen(QPen(GRID_LINE, 1));
    p.drawLine(HDR_W, HDR_H, vpW, HDR_H);

    {
        const int step = (cw >= 20) ? 5 : (cw >= 10) ? 10 : 20;
        QFont hf = p.font();
        hf.setPointSize(8);
        p.setFont(hf);
        for (int f = 0; f <= m_totalFrames; f += step) {
            const int x = HDR_W + static_cast<int>(f * m_cellWidth) - sx;
            if (x < HDR_W || x > vpW)
                continue;
            p.setPen(QPen(GRID_LINE, 1));
            p.drawLine(x, 0, x, HDR_H);
            p.setPen(HDR_TXT);
            p.drawText(x + 3, 0, 60, HDR_H - 2,
                       Qt::AlignVCenter | Qt::AlignLeft,
                       QString::number(f));
        }
    }

    // --- Row headers — drawn on top so they are never obscured ---
    p.setPen(QPen(GRID_LINE, 1));
    p.drawLine(HDR_W, HDR_H, HDR_W, vpH);

    QFont rowFont = p.font();
    rowFont.setPointSize(9);
    p.setFont(rowFont);

    for (int r = 0; r < rows; ++r) {
        const int y = HDR_H + r * ROW_H - sy;
        if (y + ROW_H < HDR_H || y > vpH)
            continue;

        // Timing-track rows use a slightly blue header to distinguish them
        // from model rows at a glance.
        const QColor& hdrColor = (m_rows[r].type == RowType::TimingTrack)
                                 ? TIMING_ROW_HDR
                                 : MODEL_ROW_HDR;

        p.fillRect(0, y, HDR_W - 1, ROW_H, hdrColor);
        p.setPen(QPen(GRID_LINE, 1));
        p.drawLine(0, y + ROW_H, HDR_W, y + ROW_H);
        p.setPen(HDR_TXT);
        p.drawText(QRect(8, y, HDR_W - 12, ROW_H),
                   Qt::AlignVCenter | Qt::AlignLeft,
                   m_rows[r].name);
    }

    // --- Corner cell ---
    p.fillRect(0, 0, HDR_W, HDR_H, HDR_BG);
    p.setPen(QPen(GRID_LINE, 1));
    p.drawLine(HDR_W, 0, HDR_W, HDR_H);
    p.drawLine(0, HDR_H, HDR_W, HDR_H);

    QFont cornerFont = p.font();
    cornerFont.setPointSize(8);
    cornerFont.setBold(true);
    p.setFont(cornerFont);
    p.setPen(HDR_TXT);
    p.drawText(QRect(0, 0, HDR_W, HDR_H), Qt::AlignCenter, "Row / Frame");
}

// ---------------------------------------------------------------------------
// resizeEvent
// ---------------------------------------------------------------------------
void SequenceGrid::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBars();
}

// ---------------------------------------------------------------------------
// mousePressEvent
// ---------------------------------------------------------------------------
void SequenceGrid::mousePressEvent(QMouseEvent* event)
{
    const int x = event->pos().x();
    const int y = event->pos().y();

    // Clicks inside the header areas don't select a cell.
    if (x < HDR_W || y < HDR_H)
        return;

    m_selectedCol = static_cast<int>((x - HDR_W + horizontalScrollBar()->value()) / m_cellWidth);
    m_selectedRow = (y - HDR_H + verticalScrollBar()->value()) / ROW_H;
    viewport()->update();
}

// ---------------------------------------------------------------------------
// wheelEvent
// ---------------------------------------------------------------------------
void SequenceGrid::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        zoom((event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15));
        event->accept();
    }
    else {
        QAbstractScrollArea::wheelEvent(event);
    }
}
