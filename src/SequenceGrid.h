#pragma once
#include <QAbstractScrollArea>
#include <QVector>
#include <QString>
#include <QColor>

struct EffectBlock {
    int     row;
    int     startCol;
    int     endCol;
    QColor  color;
    QString name;
};

class SequenceGrid : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit SequenceGrid(QWidget* parent = nullptr);

    void setModels(const QVector<QString>& models);
    void setTotalFrames(int frames);
    void loadSampleEffects();
    void zoom(double factor);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void updateScrollBars();

    QVector<QString>     m_models;
    QVector<EffectBlock> m_effects;
    int    m_totalFrames = 0;
    int    m_selectedRow = -1;
    int    m_selectedCol = -1;
    double m_cellWidth   = 20.0;

    static constexpr int HDR_W  = 150;  // row header width
    static constexpr int HDR_H  = 30;   // col header height
    static constexpr int ROW_H  = 25;
};
