#pragma once
#include <QAbstractScrollArea>
#include <QVector>
#include <QString>
#include <QColor>

struct XsqSequence;  // defined in XsqFile.h

// Distinguishes how a row should be drawn and labelled.
enum class RowType
{
    TimingTrack,
    Model
};

// Metadata for one row in the grid.
struct GridRow
{
    RowType type;
    QString name;
};

// A colored block spanning a range of frames on a single row.
struct EffectBlock
{
    int     row;
    int     startFrame;
    int     endFrame;
    QColor  color;
    QString label;
};

class SequenceGrid : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit SequenceGrid(QWidget* parent = nullptr);

    // Replace all content with the timing tracks from a parsed .xsq file.
    void loadFromXsq(const XsqSequence& xsq);

    // Populate with built-in sample data for development and testing.
    void loadSampleData();

    // Scale the frame cells by factor (clamped to a sensible range).
    void zoom(double factor);

protected:
    void paintEvent(QPaintEvent* event)   override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event)   override;

private:
    void updateScrollBars();

    QVector<GridRow>     m_rows;
    QVector<EffectBlock> m_blocks;
    int    m_totalFrames = 0;
    int    m_selectedRow = -1;
    int    m_selectedCol = -1;
    double m_cellWidth   = 20.0;

    static constexpr int HDR_W = 150;  // row-header column width  (px)
    static constexpr int HDR_H =  30;  // column-header row height (px)
    static constexpr int ROW_H =  25;  // height of each data row  (px)
};
