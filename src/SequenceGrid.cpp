#include "SequenceGrid.h"
#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLinearGradient>

namespace {
    const QColor BG_EVEN   { 42,  42,  42 };
    const QColor BG_ODD    { 50,  50,  50 };
    const QColor HDR_BG    { 60,  60,  60 };
    const QColor HDR_TXT   {200, 200, 200 };
    const QColor GRID_LINE { 70,  70,  70 };
    const QColor ROW_HDR   { 45,  45,  50 };
    const QColor PLAYHEAD  {255,  80,  80 };
}

SequenceGrid::SequenceGrid(QWidget* parent)
    : QAbstractScrollArea(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
}

void SequenceGrid::setModels(const QVector<QString>& models)
{
    m_models = models;
    updateScrollBars();
    viewport()->update();
}

void SequenceGrid::setTotalFrames(int frames)
{
    m_totalFrames = frames;
    updateScrollBars();
    viewport()->update();
}

void SequenceGrid::loadSampleEffects()
{
    struct S { int row, s, e; QColor c; const char* n; };
    static const S data[] = {
        {0,   0,  60, {  0,160,255}, "Twinkle"},
        {0,  80, 150, {255,100,  0}, "Bars"},
        {0, 200, 280, {  0,200,100}, "Wave"},
        {1,  10,  80, {200, 50,200}, "Spiral"},
        {1, 100, 180, {255,200,  0}, "Fire"},
        {2,   0,  40, {  0,200,200}, "On"},
        {2,  50, 120, {255, 80, 80}, "Wipe"},
        {2, 150, 240, {100,200,  0}, "Morph"},
        {3,  20, 100, {255,160, 20}, "Brightness"},
        {3, 120, 200, { 80,120,255}, "Color Wash"},
        {4,   0,  80, {200,200, 50}, "Curtain"},
        {4, 100, 180, {255, 50,150}, "Pinwheel"},
        {5,  30, 110, {  0,180,255}, "Marquee"},
        {5, 130, 230, {200, 80, 30}, "Snowflakes"},
        {6,   0,  60, {150,255,100}, "Butterfly"},
        {6,  80, 160, {255,200,100}, "Kaleidoscope"},
        {7,  20,  90, {100,100,255}, "Piano"},
        {7, 110, 200, {255, 80, 80}, "Shockwave"},
        {8,   0, 130, { 80,230,200}, "Galaxy"},
        {9,  40, 180, {230,100,230}, "Liquid"},
        {10,  0,  50, {255,160, 40}, "Glitter"},
        {10, 70, 170, { 60,200,255}, "Lightning"},
        {11, 10, 100, {200,255, 80}, "Ripple"},
        {11,120, 300, {255,100,150}, "Fire"},
    };
    m_effects.clear();
    for (const auto& d : data)
        m_effects.append({d.row, d.s, d.e, d.c, d.n});
    viewport()->update();
}

void SequenceGrid::zoom(double factor)
{
    m_cellWidth = qBound(4.0, m_cellWidth * factor, 80.0);
    updateScrollBars();
    viewport()->update();
}

void SequenceGrid::updateScrollBars()
{
    const int gridW = static_cast<int>(m_totalFrames * m_cellWidth);
    const int gridH = m_models.size() * ROW_H;
    const int vpW   = viewport()->width()  - HDR_W;
    const int vpH   = viewport()->height() - HDR_H;

    horizontalScrollBar()->setRange(0, qMax(0, gridW - vpW));
    horizontalScrollBar()->setPageStep(vpW);
    horizontalScrollBar()->setSingleStep(static_cast<int>(m_cellWidth));

    verticalScrollBar()->setRange(0, qMax(0, gridH - vpH));
    verticalScrollBar()->setPageStep(vpH);
    verticalScrollBar()->setSingleStep(ROW_H);
}

void SequenceGrid::paintEvent(QPaintEvent*)
{
    QPainter p(viewport());
    p.setRenderHint(QPainter::Antialiasing, false);

    const int sx   = horizontalScrollBar()->value();
    const int sy   = verticalScrollBar()->value();
    const int vpW  = viewport()->width();
    const int vpH  = viewport()->height();
    const int rows = m_models.size();
    const int cw   = static_cast<int>(m_cellWidth);

    // Fill entire background
    p.fillRect(viewport()->rect(), BG_EVEN);

    // Row stripes
    for (int r = 0; r < rows; ++r) {
        int y = HDR_H + r * ROW_H - sy;
        if (y + ROW_H < HDR_H || y > vpH) continue;
        p.fillRect(HDR_W, y, vpW - HDR_W, ROW_H, (r % 2 == 0) ? BG_EVEN : BG_ODD);
    }

    // Vertical grid lines
    {
        int step = (cw >= 20) ? 1 : (cw >= 10) ? 5 : 10;
        p.setPen(QPen(GRID_LINE, 1));
        for (int f = 0; f <= m_totalFrames; f += step) {
            int x = HDR_W + static_cast<int>(f * m_cellWidth) - sx;
            if (x < HDR_W || x > vpW) continue;
            p.drawLine(x, HDR_H, x, vpH);
        }
    }

    // Horizontal grid lines
    p.setPen(QPen(GRID_LINE, 1));
    for (int r = 0; r <= rows; ++r) {
        int y = HDR_H + r * ROW_H - sy;
        if (y < HDR_H || y > vpH) continue;
        p.drawLine(HDR_W, y, vpW, y);
    }

    // Effect blocks
    QFont ef = p.font();
    ef.setPointSize(8);
    p.setFont(ef);
    for (const auto& e : m_effects) {
        int y  = HDR_H + e.row * ROW_H - sy + 1;
        if (y + ROW_H < HDR_H || y > vpH) continue;
        int x1 = HDR_W + static_cast<int>(e.startCol * m_cellWidth) - sx;
        int x2 = HDR_W + static_cast<int>(e.endCol   * m_cellWidth) - sx;
        if (x2 < HDR_W || x1 > vpW || x2 - x1 < 2) continue;

        QRect r(x1, y, x2 - x1, ROW_H - 2);
        QLinearGradient grad(r.topLeft(), r.bottomLeft());
        grad.setColorAt(0.0, e.color.lighter(140));
        grad.setColorAt(1.0, e.color.darker(130));
        p.fillRect(r, grad);
        p.setPen(QPen(e.color.darker(160), 1));
        p.drawRect(r);
        if (r.width() > 30) {
            p.setPen(Qt::white);
            p.drawText(r.adjusted(3, 0, -2, 0), Qt::AlignVCenter | Qt::AlignLeft, e.name);
        }
    }

    // Playhead at frame 50
    {
        int x = HDR_W + static_cast<int>(50 * m_cellWidth) - sx;
        if (x >= HDR_W && x <= vpW) {
            p.setPen(QPen(PLAYHEAD, 2));
            p.drawLine(x, HDR_H, x, vpH);
            p.setBrush(PLAYHEAD);
            p.setPen(Qt::NoPen);
            QPolygon arrow;
            arrow << QPoint(x - 6, HDR_H)
                << QPoint(x + 6, HDR_H)
                << QPoint(x,     HDR_H + 10);
            p.drawPolygon(arrow);
        }
    }

    // Column header (time) - drawn over grid
    p.fillRect(HDR_W, 0, vpW - HDR_W, HDR_H, HDR_BG);
    p.setPen(QPen(GRID_LINE, 1));
    p.drawLine(HDR_W, HDR_H, vpW, HDR_H);

    {
        int step = (cw >= 20) ? 5 : (cw >= 10) ? 10 : 20;
        QFont hf = p.font();
        hf.setPointSize(8);
        p.setFont(hf);
        for (int f = 0; f <= m_totalFrames; f += step) {
            int x = HDR_W + static_cast<int>(f * m_cellWidth) - sx;
            if (x < HDR_W || x > vpW) continue;
            p.setPen(QPen(GRID_LINE, 1));
            p.drawLine(x, 0, x, HDR_H);
            p.setPen(HDR_TXT);
            p.drawText(x + 3, 0, 60, HDR_H - 2,
                Qt::AlignVCenter | Qt::AlignLeft, QString::number(f));
        }
    }

    // Row headers - drawn over grid
    p.fillRect(0, HDR_H, HDR_W, vpH - HDR_H, ROW_HDR);
    p.setPen(QPen(GRID_LINE, 1));
    p.drawLine(HDR_W, HDR_H, HDR_W, vpH);

    QFont rf = p.font();
    rf.setPointSize(9);
    p.setFont(rf);
    for (int r = 0; r < rows; ++r) {
        int y = HDR_H + r * ROW_H - sy;
        if (y + ROW_H < HDR_H || y > vpH) continue;
        p.fillRect(0, y, HDR_W - 1, ROW_H, ROW_HDR);
        p.setPen(QPen(GRID_LINE, 1));
        p.drawLine(0, y + ROW_H, HDR_W, y + ROW_H);
        p.setPen(HDR_TXT);
        p.drawText(QRect(8, y, HDR_W - 12, ROW_H),
            Qt::AlignVCenter | Qt::AlignLeft, m_models[r]);
    }

    // Corner
    p.fillRect(0, 0, HDR_W, HDR_H, HDR_BG);
    p.setPen(QPen(GRID_LINE, 1));
    p.drawLine(HDR_W, 0, HDR_W, HDR_H);
    p.drawLine(0, HDR_H, HDR_W, HDR_H);
    QFont cf = p.font();
    cf.setPointSize(8);
    cf.setBold(true);
    p.setFont(cf);
    p.setPen(HDR_TXT);
    p.drawText(QRect(0, 0, HDR_W, HDR_H), Qt::AlignCenter, "Models / Frames");
}

void SequenceGrid::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBars();
}

void SequenceGrid::mousePressEvent(QMouseEvent* event)
{
    const int x = event->pos().x();
    const int y = event->pos().y();
    if (x < HDR_W || y < HDR_H) return;

    m_selectedCol = static_cast<int>((x - HDR_W + horizontalScrollBar()->value()) / m_cellWidth);
    m_selectedRow = (y - HDR_H + verticalScrollBar()->value()) / ROW_H;
    viewport()->update();
}

void SequenceGrid::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        zoom((event->angleDelta().y() > 0) ? 1.15 : (1.0 / 1.15));
        event->accept();
    } else {
        QAbstractScrollArea::wheelEvent(event);
    }
}
